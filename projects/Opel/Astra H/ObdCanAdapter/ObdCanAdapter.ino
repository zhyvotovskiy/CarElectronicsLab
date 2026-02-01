// ============================================================================
// ObdCanAdapter - GMLAN Reader for Opel Astra H
// ============================================================================
// Reads LS-CAN (GMLAN 33.3 kbit/s) from OBD2 port
// Parses engine temperature (0x145) and battery voltage (0x500)
// Sends formatted data to IPC ECN display (0x5E8) in XXXYYY format
// ============================================================================

#include "config.h"
#include "VehicleDto.h"
#include "CanDriver.h"
#include "CanFilter.h"
#include "VehicleDataParser.h"
#include "IpcSender.h"
#include "SerialLogger.h"

// ============================================================================
// Global Objects
// ============================================================================

VehicleDto vehicleData;
CanDriver canDriver;
CanFilter canFilter;
VehicleDataParser dataParser;
IpcSender ipcSender(canDriver);
SerialLogger logger;

// ============================================================================
// Timing
// ============================================================================

unsigned long lastEcnUpdate = 0;
unsigned long lastStatusLog = 0;

// ============================================================================
// Helper Functions
// ============================================================================

// Check if enough time has passed since last update
// Updates the lastUpdate timestamp if interval has elapsed
bool isTimeToUpdate(unsigned long& lastUpdate, unsigned long interval) {
    unsigned long now = millis();
    if (now - lastUpdate >= interval) {
        lastUpdate = now;
        return true;
    }
    return false;
}

// ============================================================================
// Setup
// ============================================================================

void setup() {
    // Initialize serial logger
    logger.init(115200);
    logger.logMessage("ObdCanAdapter starting...");
    
    // Initialize CAN driver (TWAI)
    logger.logMessage("Initializing TWAI (CAN)...");
    if (!canDriver.init(CAN_BITRATE, CAN_TX_PIN, CAN_RX_PIN)) {
        logger.logMessage("FATAL: Failed to initialize CAN");
        while (1) {
            delay(1000);
        }
    }
    
    logger.logMessage("Initialization complete");
    logger.logMessage("Listening on LS-CAN (GMLAN)...");
    logger.logMessage("Filtered IDs: 0x145 (Engine), 0x500 (Voltage)");
    logger.logMessage("Output: 0x5E8 (IPC ECN Display)");
    logger.logMessage("-------------------------------------------\n");
}

// ============================================================================
// Main Loop
// ============================================================================

void loop() {
    // ========================================================================
    // Process incoming CAN messages
    // ========================================================================
    
    while (canDriver.hasPacket()) {
        uint32_t id;
        uint8_t data[8];
        uint8_t len;
        
        if (canDriver.receive(id, data, len)) {
            // Check if this ID should be processed
            if (canFilter.shouldProcess(id)) {
                // Log received packet
                logger.logReceivedPacket(id, data, len);
                
                // Parse message and update DTO
                dataParser.updateFromMessage(id, data, len, vehicleData);
            }
        }
    }
    
    // ========================================================================
    // Periodic: Send data to IPC ECN display
    // ========================================================================
    
    if (isTimeToUpdate(lastEcnUpdate, ECN_UPDATE_INTERVAL_MS)) {
        // Send current data to IPC
        if (ipcSender.sendToDisplay(vehicleData)) {
            logger.logDisplaySent();
        }
    }
    
    // ========================================================================
    // Periodic: Log status to Serial
    // ========================================================================
    
    if (isTimeToUpdate(lastStatusLog, STATUS_LOG_INTERVAL_MS)) {
        logger.logStatus(vehicleData);
    }
    
    // Small delay to prevent CPU hogging
    delay(5);
}
