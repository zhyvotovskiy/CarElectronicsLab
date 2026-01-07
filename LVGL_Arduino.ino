#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "BoardComputer.h"

void setup()
{
  Serial.begin(115200);
  LCD_Init();
  Lvgl_Init();

  BoardComputer_Init();
}

void loop()
{
  Timer_Loop();
  delay(5);
}
