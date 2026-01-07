#include "BoardComputer.h"

// Глобальные переменные для экранных элементов
static lv_obj_t * label_engine_temp;
static lv_obj_t * label_battery;
static lv_obj_t * label_bottom;
static lv_timer_t * update_timer;

// Переменные для хранения данных (заполните их данными из CAN шины)
float engine_temp = 105.0;
float battery_voltage = 14.2;
int engine_rpm = 2500;
float speed_kmh = 80.0;
float fuel_consumption = 7.5;
int remaining_km = 450;

// Текущее время для часов (заполните данными из RTC)
int current_hour = 12;
int current_minute = 30;

// Режим отображения нижней строки: 0 - часы, 1 - обороты, 2 - скорость, 3 - расход, 4 - остаток хода
int display_mode = 0;

// Кнопка BOOT для переключения режимов
#define BOOT_BUTTON_PIN 9  // GPIO9 - кнопка BOOT на ESP32-C6
bool last_button_state = HIGH;
unsigned long last_button_press = 0;
const unsigned long BUTTON_DEBOUNCE = 100; // 100 мс защита от дребезга

// Функция обновления дисплея
void IRAM_ATTR update_display(lv_timer_t * timer)
{
  char buf[100];
  unsigned long current_time = millis();

  // Проверка кнопки BOOT для ручного переключения режима
  bool button_state = digitalRead(BOOT_BUTTON_PIN);
  if (button_state == LOW && last_button_state == HIGH) {
    // Кнопка нажата (с защитой от дребезга)
    if (current_time - last_button_press > BUTTON_DEBOUNCE) {
      display_mode++;
      if (display_mode > 4) {
        display_mode = 0;
      }
      last_button_press = current_time;
    }
  }
  last_button_state = button_state;

  // Имитация изменения данных (для демонстрации)
  engine_temp = 95 + (millis() / 10000) % 20; // 95-115°C
  battery_voltage = 13.5 + (millis() / 5000) % 15 * 0.1; // 13.5-14.9V
  engine_rpm = 1500 + (millis() / 100) % 3000; // 1500-4500 RPM
  speed_kmh = 60 + (millis() / 200) % 80; // 60-140 km/h
  fuel_consumption = 5.0 + (millis() / 1000) % 50 * 0.1; // 5.0-10.0 L/100
  remaining_km = 300 + (millis() / 500) % 400; // 300-700 km

  // Обновление времени (имитация)
  current_minute = (millis() / 60000) % 60;
  current_hour = 12 + (millis() / 3600000) % 12;

  // Обновляем температуру двигателя с иконкой
  snprintf(buf, sizeof(buf), LV_SYMBOL_TINT " %.0fC", engine_temp);
  lv_label_set_text(label_engine_temp, buf);

  // Обновляем напряжение аккумулятора с иконкой
  snprintf(buf, sizeof(buf), LV_SYMBOL_BATTERY_FULL " %.1fV", battery_voltage);
  lv_label_set_text(label_battery, buf);

  // Обновляем нижнюю строку в зависимости от режима
  switch(display_mode)
  {
    case 0: // Часы
      snprintf(buf, sizeof(buf), "%02d:%02d", current_hour, current_minute);
      break;
    case 1: // Обороты двигателя
      snprintf(buf, sizeof(buf), LV_SYMBOL_REFRESH " %d RPM", engine_rpm);
      break;
    case 2: // Скорость
      snprintf(buf, sizeof(buf), LV_SYMBOL_GPS " %.0f km/h", speed_kmh);
      break;
    case 3: // Расход топлива
      snprintf(buf, sizeof(buf), LV_SYMBOL_CHARGE " %.1f l/100", fuel_consumption);
      break;
    case 4: // Остаток хода
      snprintf(buf, sizeof(buf), LV_SYMBOL_DRIVE " %d km", remaining_km);
      break;
  }
  lv_label_set_text(label_bottom, buf);
}

void BoardComputer_Init(void)
{
  // Инициализация кнопки BOOT
  pinMode(BOOT_BUTTON_PIN, INPUT_PULLUP);

  // Устанавливаем черный фон для всего экрана
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);

  // Создаем контейнер для хедера (верхняя строка)
  lv_obj_t * header_container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(header_container, LCD_WIDTH, 45);
  lv_obj_align(header_container, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_color(header_container, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(header_container, 0, 0);
  lv_obj_set_style_pad_all(header_container, 0, 0);
  lv_obj_set_flex_flow(header_container, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(header_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // Температура двигателя с иконкой (слева)
  label_engine_temp = lv_label_create(header_container);
  lv_label_set_text(label_engine_temp, LV_SYMBOL_TINT " --C");  // Иконка капли для охлаждающей жидкости
  lv_obj_set_style_text_color(label_engine_temp, lv_color_hex(0xFFA500), 0);
  #if LV_FONT_MONTSERRAT_24
    lv_obj_set_style_text_font(label_engine_temp, &lv_font_montserrat_24, 0);
  #elif LV_FONT_MONTSERRAT_22
    lv_obj_set_style_text_font(label_engine_temp, &lv_font_montserrat_22, 0);
  #elif LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(label_engine_temp, &lv_font_montserrat_20, 0);
  #elif LV_FONT_MONTSERRAT_18
    lv_obj_set_style_text_font(label_engine_temp, &lv_font_montserrat_18, 0);
  #else
    lv_obj_set_style_text_font(label_engine_temp, &lv_font_montserrat_16, 0);
  #endif
  lv_label_set_long_mode(label_engine_temp, LV_LABEL_LONG_CLIP);

  // Заряд аккумулятора с иконкой (справа)
  label_battery = lv_label_create(header_container);
  lv_label_set_text(label_battery, LV_SYMBOL_BATTERY_FULL " --V");  // Иконка батареи
  lv_obj_set_style_text_color(label_battery, lv_color_hex(0xFFA500), 0);
  #if LV_FONT_MONTSERRAT_24
    lv_obj_set_style_text_font(label_battery, &lv_font_montserrat_24, 0);
  #elif LV_FONT_MONTSERRAT_22
    lv_obj_set_style_text_font(label_battery, &lv_font_montserrat_22, 0);
  #elif LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(label_battery, &lv_font_montserrat_20, 0);
  #elif LV_FONT_MONTSERRAT_18
    lv_obj_set_style_text_font(label_battery, &lv_font_montserrat_18, 0);
  #else
    lv_obj_set_style_text_font(label_battery, &lv_font_montserrat_16, 0);
  #endif
  lv_label_set_long_mode(label_battery, LV_LABEL_LONG_CLIP);

  // Создаем горизонтальный разделитель
  lv_obj_t * separator = lv_obj_create(lv_scr_act());
  lv_obj_set_size(separator, LCD_WIDTH - 20, 2);
  lv_obj_align(separator, LV_ALIGN_TOP_MID, 0, 47);
  lv_obj_set_style_bg_color(separator, lv_color_hex(0xFFA500), 0);
  lv_obj_set_style_border_width(separator, 0, 0);

  // Создаем контейнер для нижней строки (данные)
  lv_obj_t * bottom_container = lv_obj_create(lv_scr_act());
  lv_obj_set_size(bottom_container, LCD_WIDTH, LCD_HEIGHT - 55);
  lv_obj_align(bottom_container, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_color(bottom_container, lv_color_hex(0x000000), 0);
  lv_obj_set_style_border_width(bottom_container, 0, 0);
  lv_obj_set_style_pad_all(bottom_container, 5, 0);

  // Нижняя строка - часы / данные CAN
  label_bottom = lv_label_create(bottom_container);
  lv_label_set_text(label_bottom, "--:--");
  lv_obj_set_style_text_color(label_bottom, lv_color_hex(0xFFA500), 0);
  #if LV_FONT_MONTSERRAT_36
    lv_obj_set_style_text_font(label_bottom, &lv_font_montserrat_36, 0);
  #elif LV_FONT_MONTSERRAT_32
    lv_obj_set_style_text_font(label_bottom, &lv_font_montserrat_32, 0);
  #elif LV_FONT_MONTSERRAT_28
    lv_obj_set_style_text_font(label_bottom, &lv_font_montserrat_28, 0);
  #elif LV_FONT_MONTSERRAT_24
    lv_obj_set_style_text_font(label_bottom, &lv_font_montserrat_24, 0);
  #elif LV_FONT_MONTSERRAT_22
    lv_obj_set_style_text_font(label_bottom, &lv_font_montserrat_22, 0);
  #elif LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(label_bottom, &lv_font_montserrat_20, 0);
  #else
    lv_obj_set_style_text_font(label_bottom, &lv_font_montserrat_18, 0);
  #endif
  lv_obj_align(label_bottom, LV_ALIGN_CENTER, 0, 0);

  // Создаем таймер для обновления дисплея каждые 500 мс
  update_timer = lv_timer_create(update_display, 500, NULL);
}

// Функция для изменения режима отображения (вызывайте при нажатии кнопок)
void change_display_mode(int mode)
{
  display_mode = mode;
  update_display(NULL);
}

// Функции для обновления данных (вызывайте из CAN обработчика)
void update_engine_temp(float temp)
{
  engine_temp = temp;
}

void update_battery_voltage(float voltage)
{
  battery_voltage = voltage;
}

void update_engine_rpm(int rpm)
{
  engine_rpm = rpm;
}

void update_speed(float speed)
{
  speed_kmh = speed;
}

void update_fuel_consumption(float consumption)
{
  fuel_consumption = consumption;
}

void update_remaining_km(int km)
{
  remaining_km = km;
}

void update_time(int hour, int minute)
{
  current_hour = hour;
  current_minute = minute;
}
