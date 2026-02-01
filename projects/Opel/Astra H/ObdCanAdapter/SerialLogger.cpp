#include "SerialLogger.h"
#include "config.h"
#include <Arduino.h>

// ============================================================================
// Constructor
// ============================================================================

SerialLogger::SerialLogger() {
}

// ============================================================================
// Initialize serial communication
// ============================================================================

void SerialLogger::init(unsigned long baud) {
    Serial.begin(baud);
    delay(100);
    Serial.println("\n===========================================");
    Serial.println("ObdCanAdapter - GMLAN Reader for Opel Astra H");
    Serial.println("===========================================\n");
}

// ============================================================================
// Log received CAN packet
// ============================================================================

void SerialLogger::logReceivedPacket(uint32_t id, const uint8_t* data, uint8_t len) {
    Serial.print("[CAN] RX 0x");
    Serial.print(id, HEX);
    Serial.print(" [");
    Serial.print(len);
    Serial.print("] ");
    
    for (uint8_t i = 0; i < len; i++) {
        if (data[i] < 0x10) Serial.print("0");
        Serial.print(data[i], HEX);
        if (i < len - 1) Serial.print(" ");
    }
    Serial.println();
}

// ============================================================================
// Log sent display packet
// ============================================================================

void SerialLogger::logDisplaySent() {
    Serial.println("[CAN] TX 0x5E8 -> IPC ECN display");
}

// ============================================================================
// Log current vehicle status from DTO
// ============================================================================

void SerialLogger::logStatus(const VehicleDto& dto) {
    Serial.print("[STATUS] ");
    logTemperature(dto);
    Serial.print(" | ");
    logVoltage(dto);
    Serial.println();
}

// ============================================================================
// Log temperature value from DTO
// ============================================================================

void SerialLogger::logTemperature(const VehicleDto& dto) {
    if (dto.has_engine_data) {
        Serial.print("Temp=");
        Serial.print(dto.engine_temp);
        Serial.print("°C");
    } else {
        Serial.print("Temp=---");
    }
}

// ============================================================================
// Log voltage value from DTO
// ============================================================================

void SerialLogger::logVoltage(const VehicleDto& dto) {
    if (dto.has_voltage_data) {
        Serial.print("Volt=");
        Serial.print(dto.battery_voltage, 1);
        Serial.print("V");
    } else {
        Serial.print("Volt=---");
    }
}

// ============================================================================
// Log general message
// ============================================================================

void SerialLogger::logMessage(const char* message) {
    Serial.print("[INFO] ");
    Serial.println(message);
}
