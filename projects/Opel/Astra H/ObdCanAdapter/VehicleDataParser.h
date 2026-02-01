#pragma once

#include <stdint.h>
#include "VehicleDto.h"

// ============================================================================
// Vehicle Data Parser
// ============================================================================

class VehicleDataParser {
public:
    VehicleDataParser();
    
    // Update DTO from received CAN message
    void updateFromMessage(uint32_t id, const uint8_t* data, uint8_t len, VehicleDto& dto);
    
private:
    // Parse engine temperature from 0x145
    void parseEngineTemp(const uint8_t* data, uint8_t len, VehicleDto& dto);
    
    // Parse battery voltage from 0x500
    void parseVoltage(const uint8_t* data, uint8_t len, VehicleDto& dto);
    
    // Validate temperature value
    bool isValidTemp(int temp) const;
    
    // Validate voltage value
    bool isValidVoltage(float voltage) const;
};
