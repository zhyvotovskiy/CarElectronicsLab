# CANHacker - работа с CAN-шиной

CANHacker - программа для Windows для мониторинга и отправки сообщений по CAN-шине через Arduino/ESP32 с контроллером MCP2515.

## Содержимое папки

В репозитории находятся архивы с библиотеками и прошивками:
- `arduino-canhacker-master.zip` - прошивка Arduino для работы с CANHacker
- `arduino-mcp2515-master.zip` - библиотека драйвера MCP2515
- `esp32-mcp2515-master.zip` - библиотека MCP2515 для ESP32
- `can-usb-master.zip` - USB-CAN адаптер
- `CAN+arduino.zip` - дополнительные примеры

В одном из архивов есть приложение CANHacker 2.0 для Windows.

## Установка CANHacker (Windows)

1. Распаковать архив с приложением CANHacker
2. Запустить программу (проверено на Windows 10)
3. При необходимости доустановить библиотеки Visual Basic Runtime

## Модификация для работы с Opel (GMLAN)

Для работы с Opel Astra H требуется модифицировать библиотеку MCP2515:

### Добавить скорости для 8 МГц кварца:
- **33.3 kbit/s** - для GMLAN/SW-CAN (заменил профиль 10 kbit/s)
- **95.2 kbit/s** - для диагностики (заменил профиль 100 kbit/s)

### Маппинг профилей:
| Стандартный профиль | Фактическая скорость |
|---------------------|----------------------|
| 10 kbit/s           | 33.3 kbit/s (GMLAN) |
| 100 kbit/s          | 95.2 kbit/s         |

**Важно:** При выборе скорости в CANHacker выбирайте 10 kbit/s для подключения к однопроводному CAN (GMLAN 33.3 kbit/s) Opel.

1. [can-hacker-connection-example](https://canhacker.ru/%D0%BE%D0%B4%D0%BD%D0%BE%D0%BF%D1%80%D0%BE%D0%B2%D0%BE%D0%B4%D0%BD%D1%8B%D0%B9-can-gmlan-33-3kbit/)

## Подключение оборудования

### Arduino + MCP2515
- **CS** (Chip Select) - D5
- **INT** (Interrupt) - D2
- **SCK** (SPI Clock) - D18
- **MOSI** (Master Out) - D23
- **MISO** (Master In) - D19
- **VCC** (Pover) - 5v
- **GND** (GND) - GND
- **CAN-H** - к линии CAN-High (или GND для SW-CAN)
- **CAN-L** - к линии CAN-Low (или к SW-CAN для Opel)

### ESP32 + MCP2515
Распиновка зависит от используемого SPI. См. примеры в архиве `esp32-mcp2515-master.zip`.

## Полезные видео

1. [Arduino CAN Hacker, CAN Monitor, CAN Sender - разбор вопросов](https://www.youtube.com/watch?v=s1vA49HFYCY&list=PLLUTlEXxNhgahfNqFVH9yg7EInX9Bsei-)
2. [Arduino CanHacker Lowicel - дружим Arduino с программой CanHacker](https://www.youtube.com/watch?v=8pwFN5bfxc4&list=PLLUTlEXxNhgahfNqFVH9yg7EInX9Bsei-&index=2)
3. [ESP32 + MCP2515 use CANHacker on CAN Bus system](https://www.youtube.com/watch?v=X1TrU1CR5nc)
4. [CANSAN-V10 CANHACKER](https://www.youtube.com/watch?v=YYj9HbvQ7tg&list=PLLUTlEXxNhgahfNqFVH9yg7EInX9Bsei-&index=5)


## Примечания
- Для других автомобилей используйте стандартные скорости CAN (125, 250, 500 kbit/s)
- SW-CAN требует специального трансивера (см. `/docs/CAN/SW-CAN/`)
