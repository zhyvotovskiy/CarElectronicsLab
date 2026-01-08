#pragma once
#include <Arduino.h>
#include <SPI.h>

#define LCD_WIDTH   320 // LCD width (horizontal)
#define LCD_HEIGHT  172 // LCD height (horizontal)

#define SPIFreq                        80000000
#define EXAMPLE_PIN_NUM_MISO           5
#define EXAMPLE_PIN_NUM_MOSI           6
#define EXAMPLE_PIN_NUM_SCLK           7
#define EXAMPLE_PIN_NUM_LCD_CS         14
#define EXAMPLE_PIN_NUM_LCD_DC         15
#define EXAMPLE_PIN_NUM_LCD_RST        21
#define EXAMPLE_PIN_NUM_BK_LIGHT       22
#define Frequency       1000
#define Resolution      10

#define VERTICAL   0
#define HORIZONTAL 1

#define Offset_X 0
#define Offset_Y 34

void LCD_Init(void);
void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t* color);
