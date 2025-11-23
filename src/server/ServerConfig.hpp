#pragma once

#include <cstdint>

namespace game::server {

/**
 * Server Configuration
 * 
 * Server ayarları (port, tick rate, max players, etc.)
 */
struct ServerConfig {
    // Network settings
    uint16_t port = 7777;
    
    // Game settings
    int tickRate = 60;  // Ticks per second
    int maxPlayers = 128;
    
    // Snapshot settings
    int snapshotRate = 20;  // Snapshots per second (client update rate)
    
    // Timeout settings
    float connectionTimeout = 10.0f;  // seconds
    float heartbeatInterval = 1.0f;  // seconds
    
    // Fixed timestep
    float fixedTimestep() const {
        return 1.0f / static_cast<float>(tickRate);
    }
    
    // Snapshot interval
    float snapshotInterval() const {
        return 1.0f / static_cast<float>(snapshotRate);
    }
};

} // namespace game::server

