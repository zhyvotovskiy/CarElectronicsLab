#pragma once

#include <stdint.h>
#include <driver/twai.h>

// ============================================================================
// CAN Driver (ESP32 TWAI)
// ============================================================================

class CanDriver {
public:
    CanDriver();
    
    // Initialize TWAI (CAN) with specified bitrate
    bool init(uint32_t bitrate, int tx_pin, int rx_pin);
    
    // Check if packet is available
    bool hasPacket();
    
    // Receive packet (blocking with timeout)
    bool receive(uint32_t& id, uint8_t* data, uint8_t& len);
    
    // Send packet
    bool send(uint32_t id, const uint8_t* data, uint8_t len);
    
private:
    bool initialized_;
    
    // Calculate TWAI timing configuration for given bitrate
    bool calculateTiming(uint32_t bitrate, twai_timing_config_t& timing);
};
