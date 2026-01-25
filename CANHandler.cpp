#include "CANHandler.h"
#include <Arduino.h>

// ============================================================================
// ВАЖНО: Используем библиотеку autowp-mcp2515
// ============================================================================
// Библиотека esp32-mcp2515 от dedalqq использует ESP-IDF SPI (spi_device_handle_t),
// что не совместимо с Arduino SPIClass. Поэтому используем autowp-mcp2515,
// которая поддерживает SPIClass через конструктор: MCP2515(CS_PIN, CLOCK, &SPI)
// ============================================================================

// Второй SPI интерфейс для MCP2515
// Дисплей использует основной SPI на пинах 6, 7, 5 (внутренние, не выведены)
// MCP2515 использует второй SPI на пинах 0, 1, 2, 4 (выведены на разъемы)
// Для ESP32-C6 создаем новый SPI интерфейс
SPIClass mcpSPI;

// Глобальный экземпляр MCP2515 с указанием SPI интерфейса
// Библиотека autowp-mcp2515: MCP2515(CS_PIN, SPI_CLOCK, SPI_INTERFACE)
// CS_PIN = MCP2515_CS_PIN (GP4)
// SPI_CLOCK = 10000000 (10 MHz - стандартная скорость для MCP2515)
// SPI_INTERFACE = &mcpSPI (указатель на второй SPI интерфейс)
MCP2515 mcp2515(MCP2515_CS_PIN, 10000000, &mcpSPI);

// Флаг успешной инициализации CAN
static bool canInitialized = false;

// Инициализация CAN-шины
void CANHandler_Init(void)
{
    Serial.println("Инициализация MCP2515 с библиотекой autowp-mcp2515...");
    canInitialized = false;
    
    // Инициализация второго SPI интерфейса для MCP2515
    // Дисплей использует основной SPI на пинах 6, 7, 5 (внутренние, не выведены)
    // MCP2515 использует второй SPI на пинах 0, 1, 2, 4 (выведены на разъемы)
    Serial.println("Инициализация второго SPI интерфейса...");
    Serial.print("  SCK: GP"); Serial.println(MCP2515_SCK_PIN);
    Serial.print("  MOSI: GP"); Serial.println(MCP2515_MOSI_PIN);
    Serial.print("  MISO: GP"); Serial.println(MCP2515_MISO_PIN);
    Serial.print("  CS: GP"); Serial.println(MCP2515_CS_PIN);
    
    // Настройка пинов перед инициализацией SPI
    pinMode(MCP2515_SCK_PIN, OUTPUT);
    pinMode(MCP2515_MOSI_PIN, OUTPUT);
    pinMode(MCP2515_MISO_PIN, INPUT);
    pinMode(MCP2515_CS_PIN, OUTPUT);
    
    digitalWrite(MCP2515_CS_PIN, HIGH); // CS активен низким уровнем
    digitalWrite(MCP2515_SCK_PIN, LOW);
    digitalWrite(MCP2515_MOSI_PIN, LOW);
    
    // Инициализация SPI интерфейса
    mcpSPI.begin(MCP2515_SCK_PIN, MCP2515_MISO_PIN, MCP2515_MOSI_PIN, MCP2515_CS_PIN);
    delay(100); // Даем больше времени на инициализацию SPI
    
    Serial.println("SPI интерфейс инициализирован");
    
    // Инициализация MCP2515
    Serial.println("Сброс MCP2515...");
    int resetResult = mcp2515.reset();
    if (resetResult != MCP2515::ERROR_OK) {
        Serial.print("✗ Ошибка сброса MCP2515: ");
        Serial.println(resetResult);
        Serial.println("Возможные причины:");
        Serial.println("  1. Неправильное подключение SPI (MOSI, MISO, SCK, CS)");
        Serial.println("  2. MCP2515 не получает питание (проверьте 3.3V и GND)");
        Serial.println("  3. Библиотека autowp-mcp2515 может не поддерживать второй SPI на ESP32-C6");
        Serial.println("  4. Конфликт пинов (убедитесь, что пины свободны)");
        Serial.println("Продолжаем работу без CAN (режим демо)");
        return;
    }
    Serial.println("MCP2515 сброшен успешно");
    
    // Настройка скорости: 33.3 kbps для GMLAN (LS CAN), кварц 8 МГц
    // Библиотека autowp-mcp2515 поддерживает CAN_33KBPS
    Serial.println("Попытка установить скорость 33.3 kbps (GMLAN)...");
    
    int result = mcp2515.setBitrate(CAN_33KBPS, MCP_8MHZ);
    if (result == MCP2515::ERROR_OK) {
        Serial.println("✓ Скорость установлена: CAN_33KBPS (33.3 kbps)");
    } else {
        Serial.print("✗ Ошибка установки скорости CAN_33KBPS: ");
        Serial.println(result);
        
        // Пробуем альтернативную скорость CAN_31K25BPS (ближайшая)
        Serial.println("Пробуем альтернативную скорость CAN_31K25BPS...");
        result = mcp2515.setBitrate(CAN_31K25BPS, MCP_8MHZ);
        if (result == MCP2515::ERROR_OK) {
            Serial.println("✓ Скорость установлена: CAN_31K25BPS (31.25 kbps) - ближайшая к 33.3");
        } else {
            Serial.print("✗ Ошибка установки скорости CAN_31K25BPS: ");
            Serial.println(result);
        }
    }
    
    if (result != MCP2515::ERROR_OK) {
        Serial.print("✗ Ошибка настройки скорости CAN: ");
        Serial.println(result);
        Serial.println("Проверьте подключение MCP2515 и правильность библиотеки autowp-mcp2515");
        Serial.println("Продолжаем работу без CAN (режим демо)");
        return;
    }
    
    // Установка нормального режима работы
    Serial.println("Установка нормального режима...");
    result = mcp2515.setNormalMode();
    if (result != MCP2515::ERROR_OK) {
        Serial.print("✗ Ошибка установки режима: ");
        Serial.println(result);
        Serial.println("Продолжаем работу без CAN (режим демо)");
        return;
    }
    Serial.println("✓ Нормальный режим установлен");
    
    // Настройка прерывания на INT пине (опционально, для асинхронной обработки)
    pinMode(MCP2515_INT_PIN, INPUT_PULLUP);
    
    canInitialized = true;
    Serial.println("✓ MCP2515 инициализирован успешно (GMLAN)");
    
    // Пробуждение шины после инициализации
    delay(100);
    CANHandler_WakeUpBus();
}

// Обработка входящих CAN-сообщений
void CANHandler_Process(void)
{
    // Проверяем, инициализирован ли CAN
    if (!canInitialized) {
        // CAN не инициализирован, используем только имитацию данных
        return;
    }
    
    // Проверка наличия новых сообщений
    struct can_frame canMsg;
    
    // Библиотека esp32-mcp2515 использует метод readMessage
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
        // Сообщение получено успешно
        CANHandler_ParseMessage(canMsg.can_id, canMsg.data, canMsg.can_dlc);
    }
    
    // Временная имитация данных для демонстрации (работает даже без CAN)
    // Удалите этот блок после успешного подключения к реальной CAN-шине
    static unsigned long lastSimUpdate = 0;
    if (millis() - lastSimUpdate > 1000) {
        lastSimUpdate = millis();
        // Имитация данных для демонстрации экрана
        g_vehicleData.engine_temp = 95 + (millis() / 10000) % 20; // 95-115°C
        g_vehicleData.battery_voltage = 13.5 + (millis() / 5000) % 15 * 0.1; // 13.5-14.9V
        g_vehicleData.engine_rpm = 1500 + (millis() / 100) % 3000; // 1500-4500 RPM
        g_vehicleData.speed_kmh = 60 + (millis() / 200) % 80; // 60-140 km/h
        g_vehicleData.fuel_consumption = 5.0 + (millis() / 1000) % 50 * 0.1; // 5.0-10.0 L/100
        g_vehicleData.remaining_km = 300 + (millis() / 500) % 400; // 300-700 km
        g_vehicleData.current_minute = (millis() / 60000) % 60;
        g_vehicleData.current_hour = 12 + (millis() / 3600000) % 12;
    }
}

// Парсинг конкретного CAN-сообщения
void CANHandler_ParseMessage(uint32_t id, uint8_t* data, uint8_t len)
{
    // Парсинг сообщений в зависимости от CAN ID (LS CAN / GMLAN)
    switch(id)
    {
        case CAN_ID_SPEED_RPM:  // 0x108 - Speed + RPM (LS CAN)
            // Формат: data[2]data[3] = speed (деление на 128)
            //         data[5]data[6] = RPM
            if (len >= 7) {
                uint16_t speed_raw = (data[2] << 8) | data[3];
                g_vehicleData.speed_kmh = speed_raw / 128.0;
                
                uint16_t rpm_raw = (data[5] << 8) | data[6];
                g_vehicleData.engine_rpm = rpm_raw;
            }
            break;

        case CAN_ID_ENGINE_TEMP:  // 0x145 - Engine temp (LS CAN)
            // Формат: temp = data[3] - 40
            if (len >= 4) {
                g_vehicleData.engine_temp = data[3] - 40;
            }
            break;

        case CAN_ID_BATTERY_VOLTAGE:  // 0x500 - Voltage (LS CAN)
            // Формат: V = data[1] / 8.0
            if (len >= 2) {
                g_vehicleData.battery_voltage = data[1] / 8.0;
            }
            break;

        // Дополнительные ID можно добавить по мере необходимости
        case CAN_ID_DOORS_LOCKS:  // 0x230 - Doors/locks
            // TODO: Парсинг состояния дверей
            break;

        case CAN_ID_HANDBRAKE_FOG:  // 0x370 - Handbrake/fog/etc
            // TODO: Парсинг состояния ручника, противотуманок
            break;

        case CAN_ID_STEERING_WHEEL:  // 0x175 - Steering wheel buttons
            // TODO: Парсинг кнопок на руле
            break;

        default:
            // Неизвестный CAN ID - можно логировать для отладки
            // Serial.print("Unknown CAN ID: 0x");
            // Serial.println(id, HEX);
            break;
    }
}

// Отправка CAN-сообщения
bool CANHandler_SendMessage(uint32_t id, uint8_t* data, uint8_t len)
{
    if (!canInitialized) {
        return false;
    }
    
    struct can_frame canMsg;
    canMsg.can_id = id;
    canMsg.can_dlc = (len > 8) ? 8 : len;
    
    for (uint8_t i = 0; i < canMsg.can_dlc; i++) {
        canMsg.data[i] = data[i];
    }
    
    // Библиотека esp32-mcp2515 использует метод sendMessage
    int result = mcp2515.sendMessage(&canMsg);
    return (result == MCP2515::ERROR_OK);
}

// Пробуждение CAN-шины (отправка wake up сообщения)
void CANHandler_WakeUpBus(void)
{
    // Wake up сообщение: ID 0x100, DLC 0
    uint8_t emptyData[1] = {0};
    CANHandler_SendMessage(CAN_ID_WAKE_UP, emptyData, 0);
    Serial.println("Wake up сообщение отправлено");
}
