#include "GameServer.hpp"
#include "../network/Packet.hpp"
#include "../network/PacketTypes.hpp"
#include <iostream>
#include <thread>
#include <cmath>

namespace game::server {

GameServer::GameServer() 
    : running(false)
    , accumulator(0.0f) {
}

GameServer::~GameServer() {
    shutdown();
}

bool GameServer::initialize(const ServerConfig& cfg) {
    config = cfg;
    
    // Initialize network
    if (!networkManager.initialize(config.port)) {
        return false;
    }
    
    // Initialize world
    // (Systems will be registered later if needed)
    
    running = true;
    lastUpdateTime = std::chrono::steady_clock::now();
    lastSnapshotTime = lastUpdateTime;
    
    std::cout << "GameServer initialized:" << std::endl;
    std::cout << "  Port: " << config.port << std::endl;
    std::cout << "  Tick Rate: " << config.tickRate << " Hz" << std::endl;
    std::cout << "  Snapshot Rate: " << config.snapshotRate << " Hz" << std::endl;
    std::cout << "  Max Players: " << config.maxPlayers << std::endl;
    
    return true;
}

void GameServer::shutdown() {
    if (!running) return;
    
    running = false;
    networkManager.shutdown();
    world.shutdown();
    
    std::cout << "GameServer shutdown" << std::endl;
}

void GameServer::run() {
    std::cout << "GameServer running..." << std::endl;
    
    while (running) {
        auto currentTime = std::chrono::steady_clock::now();
        auto frameTime = std::chrono::duration<float>(
            currentTime - lastUpdateTime
        ).count();
        lastUpdateTime = currentTime;
        
        // Clamp frame time to prevent spiral of death
        frameTime = std::min(frameTime, 0.1f);
        
        // Process network packets
        processNetwork();
        
        // Fixed timestep update
        accumulator += frameTime;
        const float fixedDelta = config.fixedTimestep();
        
        while (accumulator >= fixedDelta) {
            updateGame(fixedDelta);
            accumulator -= fixedDelta;
        }
        
        // Send snapshots (at snapshot rate)
        auto snapshotElapsed = std::chrono::duration<float>(
            currentTime - lastSnapshotTime
        ).count();
        
        if (snapshotElapsed >= config.snapshotInterval()) {
            sendSnapshots();
            lastSnapshotTime = currentTime;
        }
        
        // Small sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void GameServer::stop() {
    running = false;
}

void GameServer::processNetwork() {
    // Process incoming packets
    networkManager.processPackets();
    
    // Check for connection timeouts
    networkManager.checkTimeouts(config.connectionTimeout);
    
    // Handle new connections and spawn entities
    for (const auto& [addr, conn] : networkManager.getConnections()) {
        if (conn.connected && !conn.entity.isValid()) {
            // New client, spawn entity
            game::core::Entity entity = spawnPlayer(addr);
            networkManager.setClientEntity(addr, entity);
        }
    }
}

void GameServer::updateGame(float deltaTime) {
    // Update ECS world
    world.update(deltaTime);
}

void GameServer::sendSnapshots() {
    if (networkManager.getClientCount() == 0) {
        return;  // No clients to send to
    }
    
    game::network::Packet packet(game::network::PacketType::SNAPSHOT);
    createSnapshotPacket(packet);
    
    // Broadcast to all clients
    networkManager.broadcastPacket(packet);
}

game::core::Entity GameServer::spawnPlayer(const game::network::Address& address) {
    // Create entity
    game::core::Entity entity = world.createEntity();
    
    // Add components
    // Spawn at random position (for now, center of map)
    float spawnX = 100.0f + (networkManager.getClientCount() * 50.0f);  // Spawn players side by side
    float spawnY = 100.0f;
    
    game::core::components::PositionComponent posComp(spawnX, spawnY);
    world.addComponent<game::core::components::PositionComponent>(entity.id, posComp);
    
    game::core::components::VelocityComponent velComp(0.0f, 0.0f);
    world.addComponent<game::core::components::VelocityComponent>(entity.id, velComp);
    
    game::core::components::SpriteComponent spriteComp({8.0f, 16.0f}, sf::Color::Green);
    world.addComponent<game::core::components::SpriteComponent>(entity.id, spriteComp);
    
    std::cout << "Spawned player entity " << entity.id 
              << " for client " << address.toString() << std::endl;
    
    return entity;
}

void GameServer::createSnapshotPacket(game::network::Packet& packet) {
    // Get all entities with Position + Sprite components
    auto entities = world.getEntitiesWith<
        game::core::components::PositionComponent,
        game::core::components::SpriteComponent
    >();
    
    // Write entity count
    uint32_t entityCount = static_cast<uint32_t>(entities.size());
    packet.write(entityCount);
    
    // Write each entity
    for (game::core::Entity::ID entityID : entities) {
        // Write entity ID
        packet.write(entityID);
        
        // Write PositionComponent
        const auto* pos = world.getComponent<game::core::components::PositionComponent>(entityID);
        if (pos) {
            packet.write(pos->position.x);
            packet.write(pos->position.y);
        }
        
        // Write SpriteComponent (size and color)
        const auto* sprite = world.getComponent<game::core::components::SpriteComponent>(entityID);
        if (sprite) {
            packet.write(sprite->size.x);
            packet.write(sprite->size.y);
            packet.write(sprite->color.r);
            packet.write(sprite->color.g);
            packet.write(sprite->color.b);
            packet.write(sprite->color.a);
        }
    }
}

} // namespace game::server

