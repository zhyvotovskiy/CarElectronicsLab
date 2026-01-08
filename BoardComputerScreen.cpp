#include "BoardComputerScreen.h"
#include "VehicleData.h"
#include "ButtonHandler.h"

// ============================================================================
// Константы
// ============================================================================
namespace {
    // Цвета
    constexpr uint32_t COLOR_BLACK = 0x000000;
    constexpr uint32_t COLOR_ORANGE = 0xFFA500;

    // Размеры и позиции
    constexpr int16_t HEADER_HEIGHT = 45;
    constexpr int16_t SEPARATOR_HEIGHT = 2;
    constexpr int16_t SEPARATOR_MARGIN = 20;
    constexpr int16_t SEPARATOR_Y_POS = 47;
    constexpr int16_t BOTTOM_CONTAINER_HEIGHT = LCD_HEIGHT - 55;

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
        lv_obj_t* label_bottom = nullptr;
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
// Вспомогательные функции для форматирования с иконками LVGL
// ============================================================================
namespace {
    void formatBottomDisplay(char* buffer, size_t size, DisplayMode mode, const VehicleData& data) {
        switch(mode) {
            case MODE_CLOCK:
                data.formatClock(buffer, size);
                break;
            case MODE_RPM:
                data.formatRPM(buffer, size, LV_SYMBOL_REFRESH);
                break;
            case MODE_SPEED:
                data.formatSpeed(buffer, size, LV_SYMBOL_GPS);
                break;
            case MODE_FUEL_CONSUMPTION:
                data.formatFuelConsumption(buffer, size, LV_SYMBOL_CHARGE);
                break;
            case MODE_REMAINING_KM:
                data.formatRemainingKm(buffer, size, LV_SYMBOL_DRIVE);
                break;
            default:
                snprintf(buffer, size, "---");
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

        data.formatEngineTemp(buffer, sizeof(buffer), LV_SYMBOL_TINT);
        lv_label_set_text(ui.label_engine_temp, buffer);

        data.formatBatteryVoltage(buffer, sizeof(buffer), LV_SYMBOL_BATTERY_FULL);
        lv_label_set_text(ui.label_battery, buffer);
    }

    void updateBottomLabel(const VehicleData& data) {
        char buffer[TEXT_BUFFER_SIZE];
        formatBottomDisplay(buffer, sizeof(buffer), current_mode, data);
        lv_label_set_text(ui.label_bottom, buffer);
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
        applyCommonLabelStyle(label, COLOR_ORANGE, &lv_font_montserrat_28);
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
        ui.label_engine_temp = createHeaderLabel(header_container,
                                                  LV_SYMBOL_TINT " --C");
        ui.label_battery = createHeaderLabel(header_container,
                                              LV_SYMBOL_BATTERY_FULL " --V");
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

        ui.label_bottom = lv_label_create(container);
        lv_label_set_text(ui.label_bottom, "--:--");
        applyCommonLabelStyle(ui.label_bottom, COLOR_ORANGE, &lv_font_montserrat_36);
        lv_obj_align(ui.label_bottom, LV_ALIGN_CENTER, 0, 0);
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
