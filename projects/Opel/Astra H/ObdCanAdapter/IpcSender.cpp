#include "IpcSender.h"
#include "CanDriver.h"
#include "config.h"

// ============================================================================
// Static constants
// ============================================================================

constexpr uint8_t IpcSender::ERROR_DIGIT_LOW;
constexpr uint8_t IpcSender::ERROR_DIGIT_HIGH;

// ============================================================================
// Constructor
// ============================================================================

IpcSender::IpcSender(CanDriver& can_driver) 
    : can_driver_(can_driver) {
}

// ============================================================================
// Send current vehicle data to IPC ECN display
// Format: XXXYYY where XXX=temp, YYY=voltage*10
// ============================================================================

bool IpcSender::sendToDisplay(const VehicleDto& dto) {
    // Prepare values
    int temp_value = 0;
    int voltage_value = 0;
    
    // Get temperature (0-399 range)
    if (dto.has_engine_data) {
        temp_value = dto.engine_temp;
        if (temp_value < 0) temp_value = 0;
        if (temp_value > 399) temp_value = 399;
    } else {
        // No data - show as "---" (encoded as 0xEE pattern)
        temp_value = 0;
    }
    
    // Get voltage (0-999 range, multiplied by 10)
    if (dto.has_voltage_data) {
        voltage_value = (int)(dto.battery_voltage * 10.0f);
        if (voltage_value < 0) voltage_value = 0;
        if (voltage_value > 999) voltage_value = 999;
    } else {
        // No data - show as "---"
        voltage_value = 0;
    }
    
    // Encode values into ECN format
    uint8_t d0, d1, d2;
    encodeEcnValues(temp_value, voltage_value, d0, d1, d2);
    
    // Send to IPC
    return sendEcnPacket(d0, d1, d2);
}

// ============================================================================
// Encode two 3-digit values into ECN format
// Based on lsShowEcnDecimal from megadrifter LS-module
// value1: 0-399 (displayed as XXX), value2: 0-999 (displayed as YYY)
// ============================================================================

void IpcSender::encodeEcnValues(int value1, int value2, uint8_t& d0, uint8_t& d1, uint8_t& d2) {
    d0 = 0x00;
    d1 = 0x00;
    d2 = 0x00;
    
    // Encode voltage (YYY) - affects d1 and d2
    encodeVoltage(value2, d1, d2);
    
    // Encode temperature (XXX) - affects d0 and d1
    encodeTemperature(value1, d0, d1);
}

// ============================================================================
// Encode temperature value (XXX format) into d0 and d1 bytes
// Temperature range: 0-399
// d0: [hundreds][tens], d1: [ones][...]
// ============================================================================

void IpcSender::encodeTemperature(int temperature, uint8_t& d0, uint8_t& d1) {
    if (temperature > 399) {
        // Value too large - show error pattern "EE-"
        d0 = ERROR_DIGIT_LOW;      // E.. (hundreds position)
        d1 += ERROR_DIGIT_HIGH;    // .E. (tens position)
        return;
    }
    
    // Extract all digits
    uint8_t hundreds, tens, ones;
    extractDigits(temperature, hundreds, tens, ones);
    
    // Encode into bytes
    d0 += hundreds * 0x10;  // Place hundreds in high nibble of d0
    d0 += tens;             // Place tens in low nibble of d0
    d1 += ones * 0x10;      // Place ones in high nibble of d1
}

// ============================================================================
// Encode voltage value (YYY format) into d1 and d2 bytes
// Voltage range: 0-999
// d1: [..][hundreds], d2: [tens][ones]
// ============================================================================

void IpcSender::encodeVoltage(int voltage, uint8_t& d1, uint8_t& d2) {
    if (voltage > 999) {
        // Value too large - show error and clamp to last two digits
        d1 += ERROR_DIGIT_LOW;  // Show 'E' in hundreds position
        voltage = voltage % 100;  // Keep only last two digits
    }
    
    // Extract all digits
    uint8_t hundreds, tens, ones;
    extractDigits(voltage, hundreds, tens, ones);
    
    // Encode into bytes
    d1 += hundreds;                // Place hundreds in low nibble of d1
    d2 = tens * 0x10 + ones;       // Pack tens and ones into d2
}

// ============================================================================
// Extract decimal digits from value (helper method)
// ============================================================================

void IpcSender::extractDigits(int value, uint8_t& hundreds, uint8_t& tens, uint8_t& ones) {
    hundreds = (value / 100) % 10;  // Extract hundreds digit
    tens = (value / 10) % 10;       // Extract tens digit
    ones = value % 10;              // Extract ones digit
}

// ============================================================================
// Send raw ECN packet to IPC
// Format: 0x5E8, 8 bytes: 0x81, d0, d1, d2, 0x00, 0x00, 0x00, 0x00
// ============================================================================

bool IpcSender::sendEcnPacket(uint8_t d0, uint8_t d1, uint8_t d2) {
    uint8_t data[8] = {
        0x81,  // ECN command byte
        d0,    // First encoded byte
        d1,    // Second encoded byte
        d2,    // Third encoded byte
        0x00,  // Padding
        0x00,
        0x00,
        0x00
    };
    
    return can_driver_.send(LS_ID_ECN, data, 8);
}
