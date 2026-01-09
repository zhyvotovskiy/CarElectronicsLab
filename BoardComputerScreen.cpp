#include "BoardComputerScreen.h"
#include "VehicleData.h"
#include "ButtonHandler.h"

// ============================================================================
// Константы
// ============================================================================
namespace {
    // Цвета
    constexpr uint32_t COLOR_BLACK = 0x71310B;
    constexpr uint32_t COLOR_ORANGE = 0xFCFA10;

    // Размеры и позиции
    constexpr int16_t HEADER_HEIGHT = 60;
    constexpr int16_t SEPARATOR_HEIGHT = 2;
    constexpr int16_t SEPARATOR_MARGIN = 20;
    constexpr int16_t SEPARATOR_Y_POS = 62;
    constexpr int16_t BOTTOM_CONTAINER_HEIGHT = LCD_HEIGHT - 70;

    // Таймеры
    constexpr uint32_t UPDATE_INTERVAL_MS = 500;

    // Размеры буферов
    constexpr size_t TEXT_BUFFER_SIZE = 100;
}

// ============================================================================
// Внутренние структуры и переменные
// ============================================================================
namespace {
    // UI элементы
    struct UIElements {
        lv_obj_t* label_engine_temp = nullptr;
        lv_obj_t* label_battery = nullptr;
        lv_obj_t* label_bottom_value = nullptr;
        lv_obj_t* label_bottom_unit = nullptr;
        lv_timer_t* update_timer = nullptr;
    };

    UIElements ui;
    ButtonManager button_manager;
    DisplayMode current_mode = MODE_CLOCK;
}

// ============================================================================
// Вспомогательные функции для управления режимами
// ============================================================================
namespace {
    void cycleDisplayMode() {
        current_mode = static_cast<DisplayMode>((current_mode + 1) % MODE_COUNT);
    }
}

// ============================================================================
// Вспомогательные функции для форматирования
// ============================================================================
namespace {
    void formatBottomDisplay(char* value_buffer, size_t value_size,
                            char* unit_buffer, size_t unit_size,
                            DisplayMode mode, const VehicleData& data) {
        switch(mode) {
            case MODE_CLOCK:
                data.formatClock(value_buffer, value_size);
                unit_buffer[0] = '\0';  // Нет единиц измерения для часов
                break;
            case MODE_RPM:
                data.formatRPM(value_buffer, value_size, unit_buffer, unit_size);
                break;
            case MODE_SPEED:
                data.formatSpeed(value_buffer, value_size, unit_buffer, unit_size);
                break;
            case MODE_FUEL_CONSUMPTION:
                data.formatFuelConsumption(value_buffer, value_size, unit_buffer, unit_size);
                break;
            case MODE_REMAINING_KM:
                data.formatRemainingKm(value_buffer, value_size, unit_buffer, unit_size);
                break;
            default:
                snprintf(value_buffer, value_size, "---");
                unit_buffer[0] = '\0';
                break;
        }
    }
}

// ============================================================================
// Функции обновления UI
// ============================================================================
namespace {
    void updateHeaderLabels(const VehicleData& data) {
        char buffer[TEXT_BUFFER_SIZE];

        data.formatEngineTemp(buffer, sizeof(buffer));
        lv_label_set_text(ui.label_engine_temp, buffer);

        data.formatBatteryVoltage(buffer, sizeof(buffer));
        lv_label_set_text(ui.label_battery, buffer);
    }

    void updateBottomLabel(const VehicleData& data) {
        char value_buffer[TEXT_BUFFER_SIZE];
        char unit_buffer[TEXT_BUFFER_SIZE];
        formatBottomDisplay(value_buffer, sizeof(value_buffer),
                          unit_buffer, sizeof(unit_buffer),
                          current_mode, data);
        lv_label_set_text(ui.label_bottom_value, value_buffer);
        lv_label_set_text(ui.label_bottom_unit, unit_buffer);
    }

    void handleButtonInput() {
        if (button_manager.getBootButton().isPressed()) {
            cycleDisplayMode();
        }
    }
}

// ============================================================================
// Callback таймера
// ============================================================================
void IRAM_ATTR update_display(lv_timer_t* timer) {
    handleButtonInput();
    updateHeaderLabels(g_vehicleData);
    updateBottomLabel(g_vehicleData);
}

// ============================================================================
// Вспомогательные функции для создания UI элементов
// ============================================================================
namespace {
    void applyCommonLabelStyle(lv_obj_t* label, uint32_t color, const lv_font_t* font) {
        lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
        lv_obj_set_style_text_font(label, font, 0);
        lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    }

    void applyContainerStyle(lv_obj_t* container) {
        lv_obj_set_style_bg_color(container, lv_color_hex(COLOR_BLACK), 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 0, 0);
    }

    lv_obj_t* createHeaderLabel(lv_obj_t* parent, const char* initial_text) {
        lv_obj_t* label = lv_label_create(parent);
        lv_label_set_text(label, initial_text);
        applyCommonLabelStyle(label, COLOR_ORANGE, &lv_font_montserrat_48);
        return label;
    }

    lv_obj_t* createHeaderContainer() {
        lv_obj_t* container = lv_obj_create(lv_scr_act());
        lv_obj_set_size(container, LCD_WIDTH, HEADER_HEIGHT);
        lv_obj_align(container, LV_ALIGN_TOP_MID, 0, 0);
        applyContainerStyle(container);
        lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_EVENLY,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        return container;
    }

    void createHeaderLabels(lv_obj_t* header_container) {
        ui.label_engine_temp = createHeaderLabel(header_container, "--c");
        ui.label_battery = createHeaderLabel(header_container, "--v");
    }

    void createSeparator() {
        lv_obj_t* separator = lv_obj_create(lv_scr_act());
        lv_obj_set_size(separator, LCD_WIDTH - SEPARATOR_MARGIN, SEPARATOR_HEIGHT);
        lv_obj_align(separator, LV_ALIGN_TOP_MID, 0, SEPARATOR_Y_POS);
        lv_obj_set_style_bg_color(separator, lv_color_hex(COLOR_ORANGE), 0);
        lv_obj_set_style_border_width(separator, 0, 0);
    }

    void createBottomContainer() {
        lv_obj_t* container = lv_obj_create(lv_scr_act());
        lv_obj_set_size(container, LCD_WIDTH, BOTTOM_CONTAINER_HEIGHT);
        lv_obj_align(container, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_set_style_bg_color(container, lv_color_hex(COLOR_BLACK), 0);
        lv_obj_set_style_border_width(container, 0, 0);
        lv_obj_set_style_pad_all(container, 5, 0);
        lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Создаём label для значения (большой шрифт)
        ui.label_bottom_value = lv_label_create(container);
        lv_label_set_text(ui.label_bottom_value, "--:--");
        applyCommonLabelStyle(ui.label_bottom_value, COLOR_ORANGE, &lv_font_montserrat_48);

        // Создаём label для единиц измерения (маленький шрифт)
        ui.label_bottom_unit = lv_label_create(container);
        lv_label_set_text(ui.label_bottom_unit, "");
        applyCommonLabelStyle(ui.label_bottom_unit, COLOR_ORANGE, &lv_font_montserrat_24);
    }

    void createUIElements() {
        lv_obj_t* header_container = createHeaderContainer();
        createHeaderLabels(header_container);
        createSeparator();
        createBottomContainer();
    }

    void startUpdateTimer() {
        ui.update_timer = lv_timer_create(update_display, UPDATE_INTERVAL_MS, NULL);
    }
}

// ============================================================================
// Публичные функции
// ============================================================================
void BoardComputerScreen_Init() {
    // ButtonManager инициализируется автоматически в конструкторе

    // Устанавливаем черный фон для всего экрана
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(COLOR_BLACK), 0);

    createUIElements();
    startUpdateTimer();
}

void BoardComputerScreen_Stop() {
    if (ui.update_timer != nullptr) {
        lv_timer_del(ui.update_timer);
        ui.update_timer = nullptr;
    }
}

void BoardComputerScreen_ChangeMode(DisplayMode mode) {
    if (mode < MODE_COUNT) {
        current_mode = mode;
        update_display(nullptr);
    }
}
