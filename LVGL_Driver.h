#pragma once

#include <lvgl.h>
#include <lv_conf.h>
#include <esp_heap_caps.h>
#include "Display_ST7789.h"

#define LVGL_WIDTH    (LCD_WIDTH)
#define LVGL_HEIGHT   LCD_HEIGHT
#define LVGL_BUF_LEN  (LVGL_WIDTH * LVGL_HEIGHT / 20)

#define EXAMPLE_LVGL_TICK_PERIOD_MS  5

void Lvgl_Init(void);
void Timer_Loop(void);
