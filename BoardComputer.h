#pragma once

#include "LVGL_Driver.h"

// Основная функция инициализации дисплея бортового компьютера
// Настраивает GPIO9 (кнопка BOOT) для переключения режимов отображения
void BoardComputer_Init(void);

// Функции для управления режимом отображения
// mode: 0 - часы, 1 - обороты, 2 - скорость, 3 - расход, 4 - остаток хода
void change_display_mode(int mode);

// Функции для обновления данных на дисплее
void update_engine_temp(float temp);
void update_battery_voltage(float voltage);
void update_engine_rpm(int rpm);
void update_speed(float speed);
void update_fuel_consumption(float consumption);
void update_remaining_km(int km);
void update_time(int hour, int minute);
