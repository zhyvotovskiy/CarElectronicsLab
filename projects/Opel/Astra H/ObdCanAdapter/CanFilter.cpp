#include "CanFilter.h"
#include "config.h"

// ============================================================================
// Whitelist of CAN IDs to process
// ============================================================================

const uint32_t CanFilter::WHITELIST[] = {
    LS_ID_ENGINE,   // 0x145 - Engine temperature
    LS_ID_VOLTAGE   // 0x500 - Battery voltage
};

const size_t CanFilter::WHITELIST_SIZE = sizeof(WHITELIST) / sizeof(WHITELIST[0]);

// ============================================================================
// Constructor
// ============================================================================

CanFilter::CanFilter() {
}

// ============================================================================
// Check if message ID should be processed
// ============================================================================

bool CanFilter::shouldProcess(uint32_t id) const {
    for (size_t i = 0; i < WHITELIST_SIZE; i++) {
        if (WHITELIST[i] == id) {
            return true;
        }
    }
    return false;
}
