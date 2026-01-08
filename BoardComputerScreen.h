#pragma once

#include "LVGL_Driver.h"

// Режимы отображения нижней строки
enum DisplayMode : uint8_t {
    MODE_CLOCK = 0,
    MODE_RPM,
    MODE_SPEED,
    MODE_FUEL_CONSUMPTION,
    MODE_REMAINING_KM,
    MODE_COUNT  // Количество режимов
};

// Основная функция инициализации дисплея бортового компьютера
void BoardComputerScreen_Init(void);

// Остановка таймера обновления (для переключения экранов)
void BoardComputerScreen_Stop(void);

// Функции для управления режимом отображения
void BoardComputerScreen_ChangeMode(DisplayMode mode);
