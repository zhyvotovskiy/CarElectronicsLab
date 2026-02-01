#pragma once

#include <stdint.h>
#include "VehicleDto.h"

// ============================================================================
// Serial Logger
// ============================================================================

class SerialLogger {
public:
    SerialLogger();
    
    // Initialize serial communication
    void init(unsigned long baud = 115200);
    
    // Log received CAN packet
    void logReceivedPacket(uint32_t id, const uint8_t* data, uint8_t len);
    
    // Log sent display packet
    void logDisplaySent();
    
    // Log current vehicle status from DTO
    void logStatus(const VehicleDto& dto);
    
    // Log general message
    void logMessage(const char* message);

private:
    // Log temperature value
    void logTemperature(const VehicleDto& dto);
    
    // Log voltage value
    void logVoltage(const VehicleDto& dto);
};
