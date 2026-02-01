#pragma once

#include <stdint.h>
#include "VehicleDto.h"

// Forward declaration
class CanDriver;

// ============================================================================
// IPC ECN Display Sender
// ============================================================================

class IpcSender {
public:
    IpcSender(CanDriver& can_driver);
    
    // Send current vehicle data to IPC ECN display
    bool sendToDisplay(const VehicleDto& dto);
    
private:
    CanDriver& can_driver_;
    
    // ECN Display error pattern constants
    static constexpr uint8_t ERROR_DIGIT_LOW = 0x0E;   // 'E' in low nibble
    static constexpr uint8_t ERROR_DIGIT_HIGH = 0xE0;  // 'E' in high nibble
    
    // Encode two 3-digit values into ECN format
    // value1: 0-399 (temperature), value2: 0-999 (voltage*10)
    void encodeEcnValues(int value1, int value2, uint8_t& d0, uint8_t& d1, uint8_t& d2);
    
    // Encode temperature (XXX) into d0 and d1
    void encodeTemperature(int temperature, uint8_t& d0, uint8_t& d1);
    
    // Encode voltage (YYY) into d1 and d2
    void encodeVoltage(int voltage, uint8_t& d1, uint8_t& d2);
    
    // Extract decimal digits from value (helper)
    void extractDigits(int value, uint8_t& hundreds, uint8_t& tens, uint8_t& ones);
    
    // Send raw ECN packet
    bool sendEcnPacket(uint8_t d0, uint8_t d1, uint8_t d2);
};
