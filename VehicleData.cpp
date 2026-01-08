#include "VehicleData.h"
#include <cstdio>

// Определение глобального экземпляра данных автомобиля
VehicleData g_vehicleData;

// ============================================================================
// Методы форматирования данных
// ============================================================================

void VehicleData::formatEngineTemp(char* buffer, size_t size, const char* icon) const {
    if (icon) {
        snprintf(buffer, size, "%s %.0fC", icon, engine_temp);
    } else {
        snprintf(buffer, size, "%.0fC", engine_temp);
    }
}

void VehicleData::formatBatteryVoltage(char* buffer, size_t size, const char* icon) const {
    if (icon) {
        snprintf(buffer, size, "%s %.1fV", icon, battery_voltage);
    } else {
        snprintf(buffer, size, "%.1fV", battery_voltage);
    }
}

void VehicleData::formatClock(char* buffer, size_t size) const {
    snprintf(buffer, size, "%02d:%02d", current_hour, current_minute);
}

void VehicleData::formatRPM(char* buffer, size_t size, const char* icon) const {
    if (icon) {
        snprintf(buffer, size, "%s %d RPM", icon, engine_rpm);
    } else {
        snprintf(buffer, size, "%d RPM", engine_rpm);
    }
}

void VehicleData::formatSpeed(char* buffer, size_t size, const char* icon) const {
    if (icon) {
        snprintf(buffer, size, "%s %.0f km/h", icon, speed_kmh);
    } else {
        snprintf(buffer, size, "%.0f km/h", speed_kmh);
    }
}

void VehicleData::formatFuelConsumption(char* buffer, size_t size, const char* icon) const {
    if (icon) {
        snprintf(buffer, size, "%s %.1f l/100", icon, fuel_consumption);
    } else {
        snprintf(buffer, size, "%.1f l/100", fuel_consumption);
    }
}

void VehicleData::formatRemainingKm(char* buffer, size_t size, const char* icon) const {
    if (icon) {
        snprintf(buffer, size, "%s %d km", icon, remaining_km);
    } else {
        snprintf(buffer, size, "%d km", remaining_km);
    }
}
