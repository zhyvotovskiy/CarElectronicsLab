#pragma once

#include <stdint.h>

// ============================================================================
// CAN Message Filter
// ============================================================================

class CanFilter {
public:
    CanFilter();
    
    // Check if message ID should be processed
    bool shouldProcess(uint32_t id) const;
    
private:
    static const uint32_t WHITELIST[];
    static const size_t WHITELIST_SIZE;
};
