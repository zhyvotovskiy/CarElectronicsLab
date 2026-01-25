#pragma once

#include <Arduino.h>
#include "VehicleData.h"
#include <SPI.h>

// ВАЖНО: Используем библиотеку autowp-mcp2515
// Библиотека esp32-mcp2515 от dedalqq использует ESP-IDF SPI (spi_device_handle_t),
// что не совместимо с Arduino SPIClass напрямую
//
// Библиотека autowp-mcp2515 поддерживает SPIClass через конструктор:
// MCP2515(CS_PIN, SPI_CLOCK, SPI_INTERFACE)
// Это позволяет использовать второй SPI интерфейс для MCP2515
#include <mcp2515.h>

// Пины для MCP2515 (Вариант A - как подключено)
// ВАЖНО: Дисплей использует SPI на пинах 7(SCK), 5(MISO), 6(MOSI)
// Для MCP2515 используем программный SPI на других пинах
#define MCP2515_SCK_PIN   2   // GP2
#define MCP2515_MOSI_PIN  1   // GP1
#define MCP2515_MISO_PIN  0   // GP0
#define MCP2515_CS_PIN    4   // GP4
#define MCP2515_INT_PIN   5   // GP5

// CAN ID для Opel Astra H (LS CAN / GMLAN - 33.3 kbps)
// Источник: example/LS-module и EXAMPLE_CAN_COMMANDS.txt
#define CAN_ID_WAKE_UP           0x100  // Wake up bus
#define CAN_ID_SPEED_RPM         0x108  // Speed + RPM (LS CAN)
#define CAN_ID_ENGINE_TEMP       0x145  // Engine temp (LS CAN), temp = Data[3] - 40
#define CAN_ID_REMOTE            0x160  // Remote open/close (LS CAN)
#define CAN_ID_STEERING_WHEEL    0x175  // Steering wheel buttons (LS CAN)
#define CAN_ID_DOORS_LOCKS       0x230  // Doors/locks (LS CAN)
#define CAN_ID_BACKWARDS         0x350  // Backwards bit (LS CAN)
#define CAN_ID_HANDBRAKE_FOG     0x370  // Handbrake/fog/etc (LS CAN)
#define CAN_ID_BATTERY_VOLTAGE  0x500  // Voltage (LS CAN), V = value/8
#define CAN_ID_IPC_CENTRAL       0x305  // IPC central buttons/light state

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

// Отправка CAN-сообщения
// id - идентификатор сообщения
// data - массив данных
// len - длина данных (0-8)
// Возвращает true при успехе
bool CANHandler_SendMessage(uint32_t id, uint8_t* data, uint8_t len);

// Пробуждение CAN-шины (отправка wake up сообщения)
void CANHandler_WakeUpBus(void);
