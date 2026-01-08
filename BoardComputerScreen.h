#pragma once

#include "LVGL_Driver.h"

// Основная функция инициализации дисплея бортового компьютера
void BoardComputerScreen_Init(void);

// Остановка таймера обновления (для переключения экранов)
void BoardComputerScreen_Stop(void);

// Функции для управления режимом отображения
// mode: 0 - часы, 1 - обороты, 2 - скорость, 3 - расход, 4 - остаток хода
void BoardComputerScreen_ChangeMode(int mode);
