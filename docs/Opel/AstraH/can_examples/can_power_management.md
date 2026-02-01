# CAN Power Management для Opel Astra H

Документация по управлению питанием и детектированию зажигания в CAN-устройствах для Opel Astra H.

---

## 📋 Содержание

1. [Введение](#введение)
2. [Методы детектирования зажигания](#методы-детектирования-зажигания)
3. [Wake-up шины (0x100)](#wake-up-шины-0x100)
4. [Режимы сна ESP32](#режимы-сна-esp32)
5. [Поэтапный план реализации](#поэтапный-план-реализации)
6. [Примеры кода](#примеры-кода)
7. [Production-ready решения](#production-ready-решения)

---

## Введение

### Зачем нужно управление питанием?

В автомобильной электронике критично минимизировать потребление энергии, когда зажигание выключено:

- **Без управления питанием**: 80-120 mA → разряд АКБ за 10-20 дней
- **С Deep Sleep**: 10-50 μA → ~1% заряда АКБ в месяц

### Основные задачи

1. **Детектирование состояния зажигания** (включено/выключено)
2. **Переход в режим сна** при выключенном зажигании
3. **Пробуждение** при включении зажигания
4. **Корректное завершение работы** перед сном

---

## Методы детектирования зажигания

### Метод 1: CAN ID 0x170 (Key Position) ⭐ Рекомендуется для начала

#### Описание

CAN сообщение `0x170` содержит информацию о положении ключа зажигания.

**Формат пакета:**
```
ID: 0x170
DLC: 3 байта
Data[0]: 0x55 (константа)
Data[1]: 0x55 (константа)
Data[2]: Key State (состояние ключа)
```

#### Константы состояния ключа

```cpp
// Из примеров megadrifter/LS-module
#define KEY_LOCKED      0x60  // Ключ вынут, двери заперты
#define KEY_UNLOCKED    0x72  // Разблокировано, зажигание OFF
#define KEY_IGN_OFF     0x72  // Зажигание выключено
#define KEY_IGN_ON      0x74  // Зажигание включено
#define KEY_STARTER_ON  0x54  // Стартер крутится (активно)
#define KEY_STARTER_OFF 0x54  // Стартер выключен
#define KEY_IGN         0x04  // Бит зажигания (маска)
```

#### Пример реализации

```cpp
// ============== config.h ==============
#define LS_ID_KEY 0x170

#define KEY_LOCKED      0x60
#define KEY_UNLOCKED    0x72
#define KEY_IGN_ON      0x74
#define KEY_STARTER_ON  0x54

// ============== VehicleDto.h ==============
struct VehicleDto {
    int engine_temp;
    bool has_engine_data;
    float battery_voltage;
    bool has_voltage_data;
    
    // Добавить:
    uint8_t key_state;
    bool is_ignition_on;
};

// ============== VehicleDataParser.cpp ==============
void VehicleDataParser::updateFromMessage(uint32_t id, const uint8_t* data, uint8_t len, VehicleDto& dto) {
    switch (id) {
        case LS_ID_ENGINE:
            parseEngineTemp(data, len, dto);
            break;
            
        case LS_ID_VOLTAGE:
            parseVoltage(data, len, dto);
            break;
            
        case LS_ID_KEY:
            parseKeyState(data, len, dto);
            break;
            
        default:
            break;
    }
}

void VehicleDataParser::parseKeyState(const uint8_t* data, uint8_t len, VehicleDto& dto) {
    if (len < 3) return;
    
    dto.key_state = data[2];
    
    // Определяем состояние зажигания
    dto.is_ignition_on = (data[2] == KEY_IGN_ON || data[2] == KEY_STARTER_ON);
}

// ============== ObdCanAdapter.ino ==============
bool displayEnabled = true;

void loop() {
    // Обработка CAN пакетов
    while (canDriver.hasPacket()) {
        uint32_t id;
        uint8_t data[8];
        uint8_t len;
        
        if (canDriver.receive(id, data, len)) {
            if (canFilter.shouldProcess(id)) {
                logger.logReceivedPacket(id, data, len);
                dataParser.updateFromMessage(id, data, len, vehicleData);
                
                // Проверка зажигания
                if (id == LS_ID_KEY) {
                    if (vehicleData.key_state == KEY_LOCKED) {
                        displayEnabled = false;
                        logger.logMessage("Ignition OFF - display disabled");
                    } else if (vehicleData.key_state == KEY_IGN_ON) {
                        displayEnabled = true;
                        logger.logMessage("Ignition ON - display enabled");
                    }
                }
            }
        }
    }
    
    // Отправка на дисплей только если зажигание включено
    if (displayEnabled && isTimeToUpdate(lastEcnUpdate, ECN_UPDATE_INTERVAL_MS)) {
        if (ipcSender.sendToDisplay(vehicleData)) {
            logger.logDisplaySent();
        }
    }
    
    // ... остальной код
}
```

**Плюсы:**
- ✅ Не требует дополнительных проводов
- ✅ Точное определение состояния
- ✅ Простая реализация

**Минусы:**
- ❌ Устройство продолжает потреблять ~100 mA (без сна)
- ❌ CAN должна быть активна

---

### Метод 2: GPIO детектирование ACC ⭐ Рекомендуется для production

#### Описание

Подключение GPIO к линии ACC (+12V при включенном зажигании).

#### Схема подключения

```
OBD2 Pin 16 (+12V ACC) ──┬─── 10kΩ ───┬─── GPIO34 (ESP32)
                         │             │
                        LED          4.7kΩ
                      (опц)           │
                                     GND

Расчет напряжения: 12V × (4.7kΩ / (10kΩ + 4.7kΩ)) = ~3.84V (безопасно для ESP32)
```

**Важно:**
- GPIO34-39 на ESP32 — **только INPUT** (идеально для этой задачи)
- Делитель напряжения обязателен (ESP32 max 3.3V)

#### Пример реализации

```cpp
// ============== config.h ==============
#define IGNITION_GPIO 34           // GPIO для детектирования зажигания
#define IGNITION_CHECK_INTERVAL 10000  // Проверка раз в 10 секунд

// ============== ObdCanAdapter.ino ==============
#include <esp_sleep.h>

unsigned long lastIgnitionCheck = 0;

void setup() {
    // ... существующий код ...
    
    // Настроить GPIO для зажигания
    pinMode(IGNITION_GPIO, INPUT);
    
    // Проверить причину пробуждения
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0) {
        logger.logMessage("Woke up by ignition ON");
    } else if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) {
        logger.logMessage("Woke up by timer");
    }
    
    // Проверить зажигание при старте
    if (!isIgnitionOn()) {
        logger.logMessage("Ignition OFF at startup - going to sleep");
        enterDeepSleep();
    }
    
    logger.logMessage("Ignition is ON - starting normal operation");
}

void loop() {
    // ... существующий код обработки CAN ...
    
    // Периодическая проверка зажигания
    if (isTimeToUpdate(lastIgnitionCheck, IGNITION_CHECK_INTERVAL)) {
        if (!isIgnitionOn()) {
            logger.logMessage("Ignition turned OFF - entering deep sleep");
            shutdownAndSleep();
        }
    }
}

// Проверка состояния зажигания
bool isIgnitionOn() {
    return digitalRead(IGNITION_GPIO) == HIGH;
}

// Корректное выключение и переход в deep sleep
void shutdownAndSleep() {
    // Отправить финальное сообщение на дисплей (очистить экран)
    uint8_t clearData[8] = {0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    canDriver.send(LS_ID_ECN, clearData, 8);
    delay(100);  // Дать время отправиться
    
    // Остановить CAN
    twai_stop();
    twai_driver_uninstall();
    
    logger.logMessage("Entering deep sleep...");
    delay(100);  // Дать время отправиться в Serial
    
    // Настроить пробуждение
    enterDeepSleep();
}

void enterDeepSleep() {
    // Вариант 1: Пробуждение по GPIO (зажигание включено)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);  // HIGH = проснуться
    
    // Вариант 2: Дополнительно просыпаться раз в минуту для проверки
    esp_sleep_enable_timer_wakeup(60 * 1000000ULL);  // 60 секунд
    
    // Уйти в глубокий сон
    esp_deep_sleep_start();
    
    // Сюда код не дойдет - при пробуждении ESP32 перезагрузится
}
```

**Плюсы:**
- ✅ Минимальное потребление (~10-50 μA)
- ✅ Надежное детектирование
- ✅ Независимо от CAN шины

**Минусы:**
- ❌ Нужен дополнительный провод
- ❌ Требует пайки делителя напряжения

---

### Метод 3: CAN активность (косвенный метод)

#### Описание

Если CAN пакеты не приходят более N секунд → зажигание выключено.

#### Пример реализации

```cpp
// ============== config.h ==============
#define CAN_ACTIVITY_TIMEOUT_MS 5000  // 5 секунд без CAN = зажигание OFF

// ============== ObdCanAdapter.ino ==============
unsigned long lastCanActivity = 0;

void loop() {
    if (canDriver.hasPacket()) {
        lastCanActivity = millis();  // Обновляем время последней активности
        
        // ... обработка пакетов
    }
    
    // Проверка неактивности
    if (millis() - lastCanActivity > CAN_ACTIVITY_TIMEOUT_MS) {
        logger.logMessage("CAN inactive - entering light sleep");
        enterLightSleep();
    }
}

void enterLightSleep() {
    // Настроить пробуждение от CAN активности (на RX пине)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);  // CAN RX pin
    
    logger.logMessage("Entering light sleep...");
    delay(100);
    
    // Уйти в легкий сон (CAN остается активным!)
    esp_light_sleep_start();
    
    logger.logMessage("Woke up from light sleep");
    lastCanActivity = millis();  // Обновить время
}
```

**Плюсы:**
- ✅ Не требует дополнительных проводов
- ✅ Простая реализация

**Минусы:**
- ❌ Неточное определение (CAN может быть активна без зажигания)
- ❌ Потребление в Light Sleep ~1 mA (не минимальное)

---

## Wake-up шины (0x100)

### Описание

В Opel Astra H CAN шина может переходить в "спящий" режим после выключения зажигания. Пакет `0x100` с пустыми данными "будит" все устройства на шине.

### Когда отправлять

1. **При старте устройства** (в `setup()`)
2. **После выхода из Deep Sleep**
3. **Перед важными операциями** (опционально)

### Пример кода

```cpp
void wakeUpBus() {
    uint8_t data[8] = {0};  // Пустой пакет
    canDriver.send(0x100, data, 0);  // DLC = 0 (нет данных)
    delay(50);  // Небольшая задержка для стабилизации
}

// В setup():
void setup() {
    // ... инициализация CAN ...
    
    wakeUpBus();  // Разбудить шину
    
    // ... остальная инициализация ...
}
```

### Примечания

- Пакет имеет **нулевую длину** (DLC = 0)
- Не обязателен, но рекомендуется для надежности
- Используется во всех примерах megadrifter

---

## Режимы сна ESP32

### Light Sleep (легкий сон)

#### Характеристики

- **Потребление**: ~0.8-1 mA
- **RAM**: сохраняется
- **Периферия**: WiFi/BT выключены, CAN может работать
- **Время пробуждения**: ~3 ms
- **Пробуждение**: GPIO, таймер, UART, CAN RX

#### Когда использовать

- Кратковременные остановки (светофор, пробка)
- Когда нужно быстро проснуться
- Когда CAN активность может разбудить устройство

#### Пример

```cpp
void enterLightSleep() {
    // Настроить источник пробуждения
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 1);  // CAN RX pin HIGH
    
    // Уйти в легкий сон
    esp_light_sleep_start();
    
    // После пробуждения продолжается выполнение здесь
    Serial.println("Woke up!");
}
```

---

### Deep Sleep (глубокий сон) ⭐ Рекомендуется

#### Характеристики

- **Потребление**: ~10-50 μA (в 100-1000 раз меньше Light Sleep!)
- **RAM**: **теряется** (перезагрузка при пробуждении)
- **Периферия**: все выключено (кроме RTC)
- **Время пробуждения**: ~200-300 ms
- **Пробуждение**: GPIO, таймер, touchpad

#### Когда использовать

- Зажигание выключено
- Длительная парковка
- Production использование

#### Особенности

1. **При пробуждении ESP32 перезагружается** → выполнение начинается с `setup()`
2. **RTC память сохраняется** → можно хранить статистику
3. **CAN нужно переинициализировать** после пробуждения

#### Пример с RTC памятью

```cpp
// RTC память сохраняется между пробуждениями
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR unsigned long totalUptime = 0;

void setup() {
    bootCount++;
    
    // Проверить причину пробуждения
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    
    switch(wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            Serial.println("Woke up by GPIO (ignition ON)");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            Serial.println("Woke up by timer");
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            Serial.println("First boot or reset");
            bootCount = 1;
            totalUptime = 0;
            break;
    }
    
    Serial.printf("Boot count: %d\n", bootCount);
    
    // ... остальная инициализация ...
}

void enterDeepSleep() {
    // Сохранить uptime перед сном
    totalUptime += millis();
    
    // Настроить пробуждение
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);  // ACC pin
    esp_sleep_enable_timer_wakeup(60 * 1000000ULL);  // Резервный таймер 60 сек
    
    // Уйти в сон
    esp_deep_sleep_start();
}
```

---

### Сравнение режимов

| Параметр | Normal | Light Sleep | Deep Sleep |
|----------|--------|-------------|------------|
| **Потребление** | 80-120 mA | ~1 mA | ~10-50 μA |
| **RAM** | Сохраняется | Сохраняется | **Теряется** |
| **Время пробуждения** | - | 3 ms | 200-300 ms |
| **CAN работает** | ✅ | ❌ (выключен) | ❌ (выключен) |
| **Пробуждение GPIO** | - | ✅ | ✅ |
| **Пробуждение таймер** | - | ✅ | ✅ |
| **RTC память** | - | - | ✅ |
| **Разряд АКБ за месяц** | ~60 Ah | ~0.7 Ah | **~0.036 Ah** |

**Рекомендация:** Используй **Deep Sleep** для production, **Light Sleep** для отладки.

---

## Поэтапный план реализации

### Этап 0: Базовая функциональность ✅ Готово

- [x] Чтение CAN данных (0x145, 0x500)
- [x] Отправка на ECN дисплей (0x5E8)
- [x] Модульная архитектура
- [x] Serial логирование

---

### Этап 1: Отслеживание зажигания (без сна)

**Цель:** Перестать отправлять данные на дисплей при выключенном зажигании.

**Изменения:**

1. Добавить `0x170` в `CanFilter`
2. Добавить `key_state` и `is_ignition_on` в `VehicleDto`
3. Реализовать `parseKeyState()` в `VehicleDataParser`
4. Добавить флаг `displayEnabled` в `ObdCanAdapter.ino`
5. Условная отправка на дисплей

**Потребление:** ~100 mA постоянно

**Время реализации:** 30-60 минут

---

### Этап 2: Light Sleep по CAN активности

**Цель:** Уменьшить потребление до ~1 mA при неактивной шине.

**Изменения:**

1. Добавить отслеживание `lastCanActivity`
2. Реализовать `enterLightSleep()` с пробуждением от CAN RX
3. Добавить timeout проверку

**Потребление:** ~1 mA при выключенном зажигании

**Время реализации:** 1-2 часа

---

### Этап 3: Deep Sleep + GPIO ACC ⭐ Рекомендуется

**Цель:** Минимальное потребление ~10-50 μA.

**Аппаратные изменения:**

1. Припаять делитель напряжения (10kΩ + 4.7kΩ)
2. Подключить к GPIO34 и OBD2 Pin 16

**Программные изменения:**

1. Добавить `IGNITION_GPIO` в config
2. Реализовать `isIgnitionOn()`, `shutdownAndSleep()`, `enterDeepSleep()`
3. Добавить проверку зажигания в `setup()` и `loop()`
4. Корректное завершение CAN перед сном

**Потребление:** ~10-50 μA при выключенном зажигании

**Время реализации:** 2-3 часа (включая пайку)

---

### Этап 4: Watchdog Timer

**Цель:** Автоматическая перезагрузка при зависании.

**Изменения:**

```cpp
#include <esp_task_wdt.h>

void setup() {
    // Настроить watchdog на 30 секунд
    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);
    
    // ... инициализация ...
}

void loop() {
    // Сбросить watchdog каждую итерацию
    esp_task_wdt_reset();
    
    // ... основной код ...
}
```

**Время реализации:** 15-30 минут

---

### Этап 5: CAN Bus Recovery

**Цель:** Автоматическое восстановление при Bus-Off.

**Изменения:**

```cpp
// В loop():
unsigned long lastHealthCheck = 0;

if (isTimeToUpdate(lastHealthCheck, 5000)) {  // Раз в 5 секунд
    twai_status_info_t status;
    if (twai_get_status_info(&status) == ESP_OK) {
        if (status.state == TWAI_STATE_BUS_OFF) {
            logger.logMessage("[CAN] Bus-Off detected! Recovering...");
            twai_initiate_recovery();
            delay(1000);
            twai_start();
        }
        
        if (status.tx_error_counter > 10 || status.rx_error_counter > 10) {
            logger.logMessage("[WARN] CAN errors detected");
        }
    }
}
```

**Время реализации:** 30-60 минут

---

### Этап 6: Brown-out Protection

**Цель:** Защита от просадок напряжения (запуск двигателя).

**Изменения в platformio.ini:**

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

build_flags = 
    -D CONFIG_ESP32_BROWNOUT_DET=1
    -D CONFIG_ESP32_BROWNOUT_DET_LVL=2

board_build.f_cpu = 240000000L
```

**Время реализации:** 5 минут

---

## Примеры кода

### Полный пример: Deep Sleep с GPIO

```cpp
// ============================================================================
// ObdCanAdapter с Deep Sleep управлением
// ============================================================================

#include <esp_sleep.h>
#include <esp_task_wdt.h>
#include "config.h"
#include "VehicleDto.h"
#include "CanDriver.h"
#include "CanFilter.h"
#include "VehicleDataParser.h"
#include "IpcSender.h"
#include "SerialLogger.h"

// ============================================================================
// Global Objects
// ============================================================================

VehicleDto vehicleData;
CanDriver canDriver;
CanFilter canFilter;
VehicleDataParser dataParser;
IpcSender ipcSender(canDriver);
SerialLogger logger;

// ============================================================================
// Power Management
// ============================================================================

#define IGNITION_GPIO 34
#define IGNITION_CHECK_INTERVAL 10000

unsigned long lastIgnitionCheck = 0;

// RTC память для статистики
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR unsigned long totalUptime = 0;

// ============================================================================
// Timing
// ============================================================================

unsigned long lastEcnUpdate = 0;
unsigned long lastStatusLog = 0;
unsigned long lastHealthCheck = 0;

// ============================================================================
// Helper Functions
// ============================================================================

bool isTimeToUpdate(unsigned long& lastUpdate, unsigned long interval) {
    unsigned long now = millis();
    if (now - lastUpdate >= interval) {
        lastUpdate = now;
        return true;
    }
    return false;
}

bool isIgnitionOn() {
    return digitalRead(IGNITION_GPIO) == HIGH;
}

void wakeUpBus() {
    uint8_t data[8] = {0};
    canDriver.send(0x100, data, 0);
    delay(50);
}

void shutdownAndSleep() {
    // Сохранить uptime
    totalUptime += millis();
    
    // Очистить дисплей
    uint8_t clearData[8] = {0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    canDriver.send(LS_ID_ECN, clearData, 8);
    delay(100);
    
    // Остановить CAN
    twai_stop();
    twai_driver_uninstall();
    
    logger.logMessage("Entering deep sleep...");
    Serial.printf("Total uptime: %lu ms, Boot count: %d\n", totalUptime, bootCount);
    delay(100);
    
    // Настроить пробуждение
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);  // ACC HIGH
    esp_sleep_enable_timer_wakeup(60 * 1000000ULL);  // Резервный таймер
    
    // Уйти в сон
    esp_deep_sleep_start();
}

void checkCanHealth() {
    twai_status_info_t status;
    if (twai_get_status_info(&status) == ESP_OK) {
        if (status.state == TWAI_STATE_BUS_OFF) {
            logger.logMessage("[CAN] Bus-Off detected! Recovering...");
            twai_initiate_recovery();
            delay(1000);
            twai_start();
        }
    }
}

// ============================================================================
// Setup
// ============================================================================

void setup() {
    bootCount++;
    
    // Initialize serial logger
    logger.init(115200);
    logger.logMessage("ObdCanAdapter starting...");
    
    // Проверить причину пробуждения
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0:
            logger.logMessage("Woke up by ignition ON");
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            logger.logMessage("Woke up by timer");
            break;
        default:
            logger.logMessage("First boot or reset");
            bootCount = 1;
            totalUptime = 0;
            break;
    }
    
    Serial.printf("Boot count: %d, Total uptime: %lu ms\n", bootCount, totalUptime);
    
    // Настроить GPIO зажигания
    pinMode(IGNITION_GPIO, INPUT);
    
    // Проверить зажигание
    if (!isIgnitionOn()) {
        logger.logMessage("Ignition OFF at startup - going to sleep");
        delay(100);
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);
        esp_deep_sleep_start();
    }
    
    logger.logMessage("Ignition is ON - starting normal operation");
    
    // Initialize CAN driver (TWAI)
    logger.logMessage("Initializing TWAI (CAN)...");
    if (!canDriver.init(CAN_BITRATE, CAN_TX_PIN, CAN_RX_PIN)) {
        logger.logMessage("FATAL: Failed to initialize CAN");
        while (1) {
            delay(1000);
        }
    }
    
    // Wake up bus
    wakeUpBus();
    
    // Initialize watchdog
    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);
    
    logger.logMessage("Initialization complete");
    logger.logMessage("Listening on LS-CAN (GMLAN)...");
    logger.logMessage("-------------------------------------------\n");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    // Сбросить watchdog
    esp_task_wdt_reset();
    
    // ========================================================================
    // Process incoming CAN messages
    // ========================================================================
    
    while (canDriver.hasPacket()) {
        uint32_t id;
        uint8_t data[8];
        uint8_t len;
        
        if (canDriver.receive(id, data, len)) {
            if (canFilter.shouldProcess(id)) {
                logger.logReceivedPacket(id, data, len);
                dataParser.updateFromMessage(id, data, len, vehicleData);
            }
        }
    }
    
    // ========================================================================
    // Periodic: Send data to IPC ECN display
    // ========================================================================
    
    if (isTimeToUpdate(lastEcnUpdate, ECN_UPDATE_INTERVAL_MS)) {
        if (ipcSender.sendToDisplay(vehicleData)) {
            logger.logDisplaySent();
        }
    }
    
    // ========================================================================
    // Periodic: Log status to Serial
    // ========================================================================
    
    if (isTimeToUpdate(lastStatusLog, STATUS_LOG_INTERVAL_MS)) {
        logger.logStatus(vehicleData);
    }
    
    // ========================================================================
    // Periodic: Check CAN health
    // ========================================================================
    
    if (isTimeToUpdate(lastHealthCheck, 5000)) {
        checkCanHealth();
    }
    
    // ========================================================================
    // Periodic: Check ignition state
    // ========================================================================
    
    if (isTimeToUpdate(lastIgnitionCheck, IGNITION_CHECK_INTERVAL)) {
        if (!isIgnitionOn()) {
            logger.logMessage("Ignition turned OFF - entering deep sleep");
            shutdownAndSleep();
        }
    }
    
    // Small delay to prevent CPU hogging
    delay(5);
}
```

---

## Production-ready решения

### Checklist перед установкой в автомобиль

- [ ] **Watchdog Timer** настроен и протестирован
- [ ] **CAN Bus Recovery** реализован
- [ ] **Brown-out Protection** включен в platformio.ini
- [ ] **Deep Sleep** работает корректно
- [ ] **GPIO делитель напряжения** припаян и проверен мультиметром
- [ ] **Корректное завершение** CAN перед сном
- [ ] **RTC статистика** для мониторинга работы
- [ ] **Защита от зависания** в `delay()` (заменены на неблокирующие проверки)
- [ ] **Timeout** во всех CAN операциях
- [ ] **Тестирование** на стенде минимум 24 часа

### Мониторинг работы

Используй RTC память для сбора статистики:

```cpp
RTC_DATA_ATTR struct {
    int boot_count;
    unsigned long total_uptime_ms;
    int can_errors;
    int sleep_cycles;
    unsigned long last_wakeup_time;
} stats;

// При каждом boot выводи статистику
void setup() {
    Serial.printf("Boot #%d, Uptime: %lu ms, Sleeps: %d, CAN errors: %d\n",
        stats.boot_count, stats.total_uptime_ms, stats.sleep_cycles, stats.can_errors);
}
```

### Рекомендуемые настройки для production

```cpp
// config.h
#define ECN_UPDATE_INTERVAL_MS 300        // 300ms - оптимально
#define STATUS_LOG_INTERVAL_MS 2000       // Логи раз в 2 секунды
#define IGNITION_CHECK_INTERVAL 10000     // Проверка зажигания раз в 10 сек
#define CAN_HEALTH_CHECK_INTERVAL 5000    // Проверка здоровья CAN раз в 5 сек
#define WATCHDOG_TIMEOUT_SEC 30           // Watchdog на 30 секунд

// Таймауты
#define CAN_TX_TIMEOUT_MS 100
#define CAN_RX_TIMEOUT_MS 10

// Deep Sleep
#define DEEP_SLEEP_TIMER_WAKEUP_SEC 60    // Резервный таймер пробуждения
```

---

## Заключение

### Итоговая оценка потребления

| Режим | Потребление | Разряд АКБ (60 Ah) |
|-------|-------------|-------------------|
| **Без управления питанием** | 100 mA | ~25 дней |
| **Light Sleep** | 1 mA | ~2.5 года |
| **Deep Sleep (рекомендуется)** | 0.03 mA | **~228 лет** |

### Рекомендуемый путь развития

1. **Начни с Этапа 1** (отслеживание 0x170 без сна) — протестируй базовую работу
2. **Перейди сразу на Этап 3** (Deep Sleep + GPIO) — пропусти Light Sleep
3. **Добавь Этап 4-6** (Watchdog, Recovery, Brown-out) для production

### Дополнительные ресурсы

- Примеры megadrifter: `/docs/Opel/AstraH/can_examples/megadrifter/`
- ESP32 Deep Sleep API: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/sleep_modes.html
- TWAI Driver: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html

---

**Автор:** AI Assistant  
**Дата:** 2026-02-01  
**Версия:** 1.0  
**Проект:** ObdCanAdapter для Opel Astra H
