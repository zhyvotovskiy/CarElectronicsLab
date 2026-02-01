#pragma once

// ============================================================================
// CAN Configuration
// ============================================================================

// TWAI (CAN) pins for ESP32
#define CAN_TX_PIN 5
#define CAN_RX_PIN 4

// CAN bitrate (GMLAN LS-CAN)
#define CAN_BITRATE 33333  // 33.3 kbps

// ============================================================================
// CAN Message IDs (LS-CAN)
// ============================================================================

#define LS_ID_ENGINE  0x145  // Engine data (coolant temp)
#define LS_ID_VOLTAGE 0x500  // Battery voltage
#define LS_ID_ECN     0x5E8  // ECN display output

// ============================================================================
// Timing Configuration
// ============================================================================

#define ECN_UPDATE_INTERVAL_MS 300   // ECN display update interval
#define STATUS_LOG_INTERVAL_MS 2000  // Serial status logging interval

// ============================================================================
// Data Validation Ranges
// ============================================================================

#define TEMP_MIN_C    0    // Minimum valid temperature (°C)
#define TEMP_MAX_C    150  // Maximum valid temperature (°C)
#define VOLTAGE_MIN_V 8.0  // Minimum valid voltage (V)
#define VOLTAGE_MAX_V 16.0 // Maximum valid voltage (V)
