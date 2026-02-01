# ObdCanAdapter - GMLAN Reader для Opel Astra H

Устройство для чтения данных из LS-CAN (GMLAN) Opel Astra H через OBD2 разъём и отображения температуры двигателя и напряжения аккумулятора на штатном экране приборной панели (IPC) в режиме ESN.

## Описание

ObdCanAdapter подключается в OBD2 разъём под ручником и:
- Читает данные из шины LS-CAN (GMLAN 33.3 kbit/s)
- Парсит температуру двигателя (из пакета 0x145)
- Парсит напряжение аккумулятора (из пакета 0x500)
- Отправляет отформатированные данные на экран IPC в формате XXXYYY (XXX = температура, YYY = напряжение×10)

## Железо

### Компоненты
- **ESP32** - микроконтроллер со встроенным TWAI (CAN 2.0)
- **VP230** (или совместимый) - CAN-трансивер для однопроводного GMLAN
- **Преобразователь напряжения** (12V → 5V/3.3V) - для питания ESP32

### Распиновка OBD2 (Opel Astra H)
```
Pin 1  - SWCAN (GMLAN) → подключить к CANH трансивера
Pin 4  - GND (масса)
Pin 16 - +12V (питание через преобразователь)
```

### Подключение VP230
```
VP230 CANH  → OBD2 Pin 1 (SWCAN)
VP230 CANL  → GND (масса)
VP230 TX    → ESP32 GPIO 5 (CAN_TX_PIN)
VP230 RX    → ESP32 GPIO 4 (CAN_RX_PIN)
VP230 VCC   → 3.3V
VP230 GND   → GND
```

## Конфигурация

### Пины (config.h)
```cpp
#define CAN_TX_PIN 5  // ESP32 TX для TWAI
#define CAN_RX_PIN 4  // ESP32 RX для TWAI
```

### Интервалы обновления
```cpp
#define ECN_UPDATE_INTERVAL_MS 300   // Обновление дисплея IPC
#define STATUS_LOG_INTERVAL_MS 2000  // Логирование в Serial
```

## Сборка и загрузка

### Arduino IDE
1. Установите поддержку ESP32 в Arduino IDE
2. Выберите плату ESP32 (например, ESP32 Dev Module)
3. Откройте `ObdCanAdapter.ino`
4. Компилируйте и загрузите

### PlatformIO (рекомендуется)
Создайте `platformio.ini`:
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
```

## Использование

1. Подключите устройство в OBD2 разъём
2. Включите зажигание
3. На экране IPC в режиме ESN появятся данные в формате `XXXYYY`:
   - `XXX` - температура двигателя (°C)
   - `YYY` - напряжение аккумулятора (V×10), например `138` = 13.8V

## Serial Monitor

При подключении к Serial Monitor (115200 baud) вы увидите:
```
[CAN] TWAI initialized successfully
[CAN] Bitrate: 33333 bps
[CAN] RX 0x145 [8] 20 00 01 5A 00 04 00 00
[CAN] RX 0x500 [2] 00 5C
[CAN] TX 0x5E8 -> IPC ECN display
[STATUS] Temp=50°C | Volt=11.5V
```

## Модульная структура

```
ObdCanAdapter/
├── ObdCanAdapter.ino      # Главный файл, setup/loop
├── config.h               # Конфигурация: пины, ID, константы
├── VehicleDto.h           # DTO: структура данных автомобиля
├── CanDriver.cpp/h        # TWAI драйвер для CAN
├── CanFilter.cpp/h        # Фильтрация пакетов по ID
├── VehicleDataParser.cpp/h# Парсинг 0x145, 0x500
├── IpcSender.cpp/h        # Отправка 0x5E8 на IPC
└── SerialLogger.cpp/h     # Логирование в Serial Monitor
```

## Обработка пакетов

### 0x145 - Engine Data (температура)
- **Байт 3**: Coolant temperature
- **Формула**: `temp_celsius = data[3] - 40`
- **Пример**: `data[3] = 0x5A` (90) → `90 - 40 = 50°C`

### 0x500 - Voltage (напряжение АКБ)
- **Байт 1**: Voltage в восьмых вольта
- **Формула**: `voltage = data[1] * 0.125`
- **Пример**: `data[1] = 0x5C` (92) → `92 * 0.125 = 11.5V`

### 0x5E8 - ECN Display (вывод на IPC)
- **Формат**: `0x81, d0, d1, d2, 0x00, 0x00, 0x00, 0x00`
- **Кодирование**: Два значения по 3 цифры (XXX YYY)
- **Алгоритм**: На основе `lsShowEcnDecimal` из megadrifter LS-module

## Валидация данных

- **Температура**: 0–150°C
- **Напряжение**: 8.0–16.0V
- При выходе за пределы данные игнорируются

## Отладка

### Проблема: Нет связи с CAN
1. Проверьте подключение VP230 к OBD2 Pin 1
2. Убедитесь, что CANL подключен к массе
3. Проверьте правильность пинов TX/RX на ESP32

### Проблема: Не приходят данные
1. Убедитесь, что зажигание включено
2. Проверьте Serial Monitor на наличие логов
3. Убедитесь, что автомобиль поддерживает GMLAN на OBD2 Pin 1

### Проблема: Не отображается на IPC
1. Убедитесь, что IPC поддерживает ECN режим
2. Проверьте формат данных в Serial Monitor
3. Попробуйте увеличить интервал обновления

## Источники

- [Opel Astra H CAN Database](../../docs/Opel/AstraH/can_examples/Car-CAN-Message-DB/)
- [Megadrifter LS-module](../../docs/Opel/AstraH/can_examples/megadrifter/LS-module/)
- [IPC Documentation](../../docs/Opel/AstraH/IPC/readme.md)

## Лицензия

Проект создан исключительно для образовательных целей и законного использования на собственных автомобилях.
