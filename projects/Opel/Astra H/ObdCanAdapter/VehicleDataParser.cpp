#include "VehicleDataParser.h"
#include "config.h"

// ============================================================================
// Constructor
// ============================================================================

VehicleDataParser::VehicleDataParser() {
}

// ============================================================================
// Update DTO from received CAN message
// ============================================================================

void VehicleDataParser::updateFromMessage(uint32_t id, const uint8_t* data, uint8_t len, VehicleDto& dto) {
    switch (id) {
        case LS_ID_ENGINE:
            parseEngineTemp(data, len, dto);
            break;
            
        case LS_ID_VOLTAGE:
            parseVoltage(data, len, dto);
            break;
            
        default:
            // Unknown ID - ignore
            break;
    }
}

// ============================================================================
// Parse engine temperature from 0x145
// Formula: temp_celsius = data[3] - 40
// ============================================================================

void VehicleDataParser::parseEngineTemp(const uint8_t* data, uint8_t len, VehicleDto& dto) {
    if (len < 4) {
        return; // Not enough data
    }
    
    // Data[3] contains raw temperature value
    // Formula: °C = raw - 40
    int raw_temp = data[3];
    int temp = raw_temp - 40;
    
    if (isValidTemp(temp)) {
        dto.engine_temp = temp;
        dto.has_engine_data = true;
    }
}

// ============================================================================
// Parse battery voltage from 0x500
// Formula: voltage = data[1] * 0.125
// ============================================================================

void VehicleDataParser::parseVoltage(const uint8_t* data, uint8_t len, VehicleDto& dto) {
    if (len < 2) {
        return; // Not enough data
    }
    
    // Data[1] contains voltage in eighths of a volt
    // Formula: V = raw * 0.125
    uint8_t raw_voltage = data[1];
    float voltage = raw_voltage * 0.125f;
    
    if (isValidVoltage(voltage)) {
        dto.battery_voltage = voltage;
        dto.has_voltage_data = true;
    }
}

// ============================================================================
// Validate temperature value
// ============================================================================

bool VehicleDataParser::isValidTemp(int temp) const {
    return (temp >= TEMP_MIN_C && temp <= TEMP_MAX_C);
}

// ============================================================================
// Validate voltage value
// ============================================================================

bool VehicleDataParser::isValidVoltage(float voltage) const {
    return (voltage >= VOLTAGE_MIN_V && voltage <= VOLTAGE_MAX_V);
}
