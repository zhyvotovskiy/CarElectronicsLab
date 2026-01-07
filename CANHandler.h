#pragma once

#include <Arduino.h>
#include "VehicleData.h"

// CAN ID для Opel Astra H (заполните реальными значениями)
// TODO: Добавить реальные CAN ID из документации автомобиля
#define CAN_ID_ENGINE_TEMP      0x100  // Пример
#define CAN_ID_ENGINE_RPM       0x101  // Пример
#define CAN_ID_BATTERY_VOLTAGE  0x102  // Пример
#define CAN_ID_SPEED            0x103  // Пример
#define CAN_ID_FUEL             0x104  // Пример

// Инициализация CAN-шины
// Настраивает пины, скорость и фильтры CAN
void CANHandler_Init(void);

// Обработка входящих CAN-сообщений
// Вызывайте эту функцию в главном цикле или по прерыванию
void CANHandler_Process(void);

// Парсинг конкретного CAN-сообщения
// id - идентификатор сообщения
// data - массив данных (обычно 8 байт)
// len - длина данных
void CANHandler_ParseMessage(uint32_t id, uint8_t* data, uint8_t len);
