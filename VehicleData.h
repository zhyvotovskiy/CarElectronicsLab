#pragma once

// Структура для хранения всех данных автомобиля
struct VehicleData {
    // Данные двигателя
    float engine_temp;          // Температура двигателя (°C)
    int engine_rpm;             // Обороты двигателя (RPM)

    // Электропитание
    float battery_voltage;      // Напряжение АКБ (V)

    // Движение
    float speed_kmh;            // Скорость (км/ч)

    // Топливо
    float fuel_consumption;     // Расход топлива (л/100км)
    int remaining_km;           // Остаток хода (км)

    // Время
    int current_hour;
    int current_minute;

    // Конструктор с значениями по умолчанию
    VehicleData() :
        engine_temp(0),
        engine_rpm(0),
        battery_voltage(0),
        speed_kmh(0),
        fuel_consumption(0),
        remaining_km(0),
        current_hour(0),
        current_minute(0)
    {}
};

// Глобальный экземпляр данных автомобиля
extern VehicleData g_vehicleData;
