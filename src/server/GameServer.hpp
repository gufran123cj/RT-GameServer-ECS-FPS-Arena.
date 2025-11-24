#pragma once

#include <chrono>
#include <memory>
#include "ServerConfig.hpp"
#include "ServerNetworkManager.hpp"
#include "../core/World.hpp"
#include "../core/components/PositionComponent.hpp"
#include "../core/components/VelocityComponent.hpp"
#include "../core/components/SpriteComponent.hpp"

namespace game::server {

/**
 * Game Server
 * 
 * Main server class that manages:
 * - Fixed timestep game loop (60 tick/s)
 * - ECS World
 * - Network manager
 * - Client connections
 * - Entity spawning
 */
class GameServer {
public:
    GameServer();
    ~GameServer();
    
    // Non-copyable
    GameServer(const GameServer&) = delete;
    GameServer& operator=(const GameServer&) = delete;
    
    // Movable
    GameServer(GameServer&&) noexcept = default;
    GameServer& operator=(GameServer&&) noexcept = default;
    
    /**
     * Initialize server
     */
    bool initialize(const ServerConfig& config = ServerConfig{});
    
    /**
     * Shutdown server
     */
    void shutdown();
    
    /**
     * Run server main loop
     * Blocks until shutdown
     */
    void run();
    
    /**
     * Stop server (called from another thread)
     */
    void stop();
    
    /**
     * Get server config
     */
    const ServerConfig& getConfig() const { return config; }
    
    /**
     * Get ECS World
     */
    game::core::World& getWorld() { return world; }
    const game::core::World& getWorld() const { return world; }
    
private:
    ServerConfig config;
    ServerNetworkManager networkManager;
    game::core::World world;
    
    bool running;
    std::chrono::steady_clock::time_point lastUpdateTime;
    std::chrono::steady_clock::time_point lastSnapshotTime;
    std::chrono::steady_clock::time_point lastDebugLogTime;
    float accumulator;  // For fixed timestep
    
    /**
     * Process network packets
     */
    void processNetwork();
    
    /**
     * Update game logic (fixed timestep)
     */
    void updateGame(float deltaTime);
    
    /**
     * Send snapshots to clients
     */
    void sendSnapshots();
    
    /**
     * Spawn player entity for new client
     * @param initialPosition Initial position from client (LDtk player position)
     */
    game::core::Entity spawnPlayer(const game::network::Address& address, const sf::Vector2f& initialPosition = sf::Vector2f(0, 0));
    
    /**
     * Create snapshot packet from world state
     */
    void createSnapshotPacket(game::network::Packet& packet);
};

} // namespace game::server

