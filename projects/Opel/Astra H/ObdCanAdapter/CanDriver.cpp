#include "CanDriver.h"
#include <Arduino.h>

// ============================================================================
// Constructor
// ============================================================================

CanDriver::CanDriver() 
    : initialized_(false) {
}

// ============================================================================
// Initialize TWAI (CAN) with specified bitrate
// ============================================================================

bool CanDriver::init(uint32_t bitrate, int tx_pin, int rx_pin) {
    // General configuration
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(
        (gpio_num_t)tx_pin, 
        (gpio_num_t)rx_pin, 
        TWAI_MODE_NORMAL
    );
    g_config.rx_queue_len = 10;
    g_config.tx_queue_len = 10;
    
    // Timing configuration for specified bitrate
    twai_timing_config_t t_config;
    if (!calculateTiming(bitrate, t_config)) {
        Serial.println("[ERROR] Failed to calculate TWAI timing");
        return false;
    }
    
    // Filter configuration - accept all messages
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    
    // Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        Serial.println("[ERROR] Failed to install TWAI driver");
        return false;
    }
    
    // Start TWAI driver
    if (twai_start() != ESP_OK) {
        Serial.println("[ERROR] Failed to start TWAI");
        twai_driver_uninstall();
        return false;
    }
    
    initialized_ = true;
    Serial.println("[CAN] TWAI initialized successfully");
    Serial.print("[CAN] Bitrate: ");
    Serial.print(bitrate);
    Serial.println(" bps");
    
    return true;
}

// ============================================================================
// Check if packet is available
// ============================================================================

bool CanDriver::hasPacket() {
    if (!initialized_) {
        return false;
    }
    
    twai_status_info_t status;
    if (twai_get_status_info(&status) == ESP_OK) {
        return (status.msgs_to_rx > 0);
    }
    
    return false;
}

// ============================================================================
// Receive packet (blocking with timeout)
// ============================================================================

bool CanDriver::receive(uint32_t& id, uint8_t* data, uint8_t& len) {
    if (!initialized_) {
        return false;
    }
    
    twai_message_t message;
    
    // Try to receive with 10ms timeout
    if (twai_receive(&message, pdMS_TO_TICKS(10)) == ESP_OK) {
        // Check if it's a standard frame (not extended)
        if (!(message.extd)) {
            id = message.identifier;
            len = message.data_length_code;
            
            // Copy data
            for (uint8_t i = 0; i < len && i < 8; i++) {
                data[i] = message.data[i];
            }
            
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// Send packet
// ============================================================================

bool CanDriver::send(uint32_t id, const uint8_t* data, uint8_t len) {
    if (!initialized_) {
        return false;
    }
    
    twai_message_t message;
    message.identifier = id;
    message.extd = 0;  // Standard frame
    message.rtr = 0;   // Data frame
    message.data_length_code = len;
    
    // Copy data
    for (uint8_t i = 0; i < len && i < 8; i++) {
        message.data[i] = data[i];
    }
    
    // Try to transmit with 100ms timeout
    if (twai_transmit(&message, pdMS_TO_TICKS(100)) == ESP_OK) {
        return true;
    }
    
    return false;
}

// ============================================================================
// Calculate TWAI timing configuration for given bitrate
// For 33.333 kbps GMLAN
// ============================================================================

bool CanDriver::calculateTiming(uint32_t bitrate, twai_timing_config_t& timing) {
    // For 33.333 kbps (GMLAN/SW-CAN)
    if (bitrate == 33333) {
        // Custom timing for 33.333 kbps
        // Based on 80 MHz APB clock
        // BRP = 120, TSEG1 = 15, TSEG2 = 4, SJW = 3
        // Bitrate = 80000000 / (BRP * (1 + TSEG1 + TSEG2))
        // Bitrate = 80000000 / (120 * 20) = 33333 bps
        
        timing.brp = 120;
        timing.tseg_1 = 15;
        timing.tseg_2 = 4;
        timing.sjw = 3;
        timing.triple_sampling = false;
        
        return true;
    }
    
    // Fallback to standard configurations
    switch (bitrate) {
        case 25000:
            timing = TWAI_TIMING_CONFIG_25KBITS();
            return true;
        case 50000:
            timing = TWAI_TIMING_CONFIG_50KBITS();
            return true;
        case 100000:
            timing = TWAI_TIMING_CONFIG_100KBITS();
            return true;
        case 125000:
            timing = TWAI_TIMING_CONFIG_125KBITS();
            return true;
        case 250000:
            timing = TWAI_TIMING_CONFIG_250KBITS();
            return true;
        case 500000:
            timing = TWAI_TIMING_CONFIG_500KBITS();
            return true;
        case 800000:
            timing = TWAI_TIMING_CONFIG_800KBITS();
            return true;
        case 1000000:
            timing = TWAI_TIMING_CONFIG_1MBITS();
            return true;
        default:
            return false;
    }
}
