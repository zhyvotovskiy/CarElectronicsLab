#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "SplashScreen.h"
#include "BoardComputerScreen.h"
#include "CANHandler.h"

void setup()
{
  Serial.begin(115200);
  LCD_Init();
  Lvgl_Init();

  SplashScreen_Show();

  CANHandler_Init();
  BoardComputerScreen_Init();
}

void loop()
{
  CANHandler_Process();  // Обновление данных автомобиля
  Timer_Loop();
  delay(5);
}
