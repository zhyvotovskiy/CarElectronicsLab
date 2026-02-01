#include "VehicleData.h"
#include <cstdio>

// Определение глобального экземпляра данных автомобиля
VehicleData g_vehicleData;

// ============================================================================
// Методы форматирования данных
// ============================================================================

void VehicleData::formatEngineTemp(char* buffer, size_t size) const {
    snprintf(buffer, size, "%.0fc", engine_temp);
}

void VehicleData::formatBatteryVoltage(char* buffer, size_t size) const {
    snprintf(buffer, size, "%.1fv", battery_voltage);
}

void VehicleData::formatClock(char* buffer, size_t size) const {
    snprintf(buffer, size, "%02d:%02d", current_hour, current_minute);
}

void VehicleData::formatRPM(char* value_buffer, size_t value_size, char* unit_buffer, size_t unit_size) const {
    snprintf(value_buffer, value_size, "%d", engine_rpm);
    snprintf(unit_buffer, unit_size, "rpm");
}

void VehicleData::formatSpeed(char* value_buffer, size_t value_size, char* unit_buffer, size_t unit_size) const {
    snprintf(value_buffer, value_size, "%.0f", speed_kmh);
    snprintf(unit_buffer, unit_size, "km/h");
}

void VehicleData::formatFuelConsumption(char* value_buffer, size_t value_size, char* unit_buffer, size_t unit_size) const {
    snprintf(value_buffer, value_size, "%.1f", fuel_consumption);
    snprintf(unit_buffer, unit_size, "l/100");
}

void VehicleData::formatRemainingKm(char* value_buffer, size_t value_size, char* unit_buffer, size_t unit_size) const {
    snprintf(value_buffer, value_size, "%d", remaining_km);
    snprintf(unit_buffer, unit_size, "range/km");
}
