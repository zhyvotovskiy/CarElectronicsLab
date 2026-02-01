#pragma once

// ============================================================================
// Vehicle Data Transfer Object
// ============================================================================

struct VehicleDto {
    // Engine data
    int engine_temp;           // Engine coolant temperature (°C)
    bool has_engine_data;      // Flag: valid engine temperature data received
    
    // Electrical data
    float battery_voltage;     // Battery voltage (V)
    bool has_voltage_data;     // Flag: valid voltage data received
    
    // Constructor - initialize with invalid data
    VehicleDto() 
        : engine_temp(0)
        , has_engine_data(false)
        , battery_voltage(0.0f)
        , has_voltage_data(false)
    {}
};
