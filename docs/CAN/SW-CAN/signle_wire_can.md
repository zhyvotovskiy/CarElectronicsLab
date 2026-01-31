Однопроводный CAN (Single Wire CAN, SW-CAN) применяется преимущественно в нишевых автомобильных системах и ориентирован на снижение стоимости. Он определен спецификацией SAE 2411 и использует один однополярный провод данных вместо дифференциальной пары, что уменьшает помехоустойчивость и, как следствие, ограничивает скорость по сравнению с другими физическими уровнями CAN.  
Источник: [NI XNET документация](https://www.ni.com/docs/en-US/bundle/ni-xnet/page/single-wire-can.html)

Ключевые особенности:
- 4 режима связи:
  - Normal Mode: 33.333 кбит/с — штатный режим для внутрисистемного обмена.
  - High Speed Mode: 83.333 кбит/с — режим для загрузки данных при подключении внешнего тестера (offboard tester ECU).
  - Sleep Mode: контроллер игнорирует трафик с номинальными уровнями 0 В и 4 В.
  - High Voltage Wakeup Mode: обмен на нормальной скорости, но с уровнями 0 В и 12 В (обычно близко к Vbat); при приеме таких уровней спящий контроллер пробуждается.
- Номинальные уровни в Normal/High Speed Mode: 0 В и 4 В.
- Уровни в High Voltage Wakeup Mode: 0 В и 12 В.

Примечание: рекомендации по кабелированию и дополнительная информация приведены в разделе «Single Wire Physical Layer» в документации NI.
Также однопроводный CAN (GMLAN 33.3 кбит/с) применяется в автомобилях GM, в том числе у Opel Astra H.  
Источник: https://canhacker.ru/%D0%BE%D0%B4%D0%BD%D0%BE%D0%BF%D1%80%D0%BE%D0%B2%D0%BE%D0%B4%D0%BD%D1%8B%D0%B9-can-gmlan-33-3kbit/

## SWCAN Transceiver Breakout Board (Beyondlogic)
Короткая справка по статье о плате-трансивере SWCAN (SO8‑вариант):
- SWCAN часто используется в автомобильных системах с ограничениями по стоимости и длине линии; типичные скорости 33.3 кбит/с (Normal) и 83.3 кбит/с (High Speed).
- Пример применения: однопроводная цифровая связь через линию Control Pilot (CP) в EV‑зарядке (упоминается использование Tesla).
- Указаны распространенные трансиверы: NCV7356, TH8056, MC33897 и др.
- В статье есть ссылки на файлы проекта (Gerbers, Schematics, BOM).

Ссылки:
- Статья: https://www.beyondlogic.org/swcan-single-wire-can-transceiver-breakout-board/
- Схемы (PDF): https://www.beyondlogic.org/design_files/SWCAN_1.A.1_Schematics.pdf
- Gerbers (ZIP): https://www.beyondlogic.org/design_files/SWCAN_1.A.1_Gerbers.zip
- BOM (XLSX): https://www.beyondlogic.org/design_files/SWCAN_1.A.1_BOM.xlsx
- Выкаченые файлы находятся в папке [./swcan_breakout]