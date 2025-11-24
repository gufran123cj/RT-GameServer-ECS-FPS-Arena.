#pragma once

#include <unordered_map>
#include <chrono>
#include <SFML/Network/UdpSocket.hpp>
#include <SFML/System/Vector2.hpp>
#include "../network/Address.hpp"
#include "../network/Packet.hpp"
#include "../core/Entity.hpp"

namespace game::server {

/**
 * Client Connection Info
 */
struct ClientConnection {
    game::network::Address address;
    game::core::Entity entity;
    std::chrono::steady_clock::time_point lastHeartbeat;
    bool connected;
    
    ClientConnection() : connected(false) {}
    ClientConnection(const game::network::Address& addr, const game::core::Entity& ent)
        : address(addr), entity(ent), connected(true) {
        lastHeartbeat = std::chrono::steady_clock::now();
    }
};

/**
 * Server Network Manager
 * 
 * UDP socket yönetimi, client bağlantıları, packet gönderme/alma
 */
class ServerNetworkManager {
public:
    ServerNetworkManager();
    ~ServerNetworkManager();
    
    // Non-copyable
    ServerNetworkManager(const ServerNetworkManager&) = delete;
    ServerNetworkManager& operator=(const ServerNetworkManager&) = delete;
    
    // Movable
    ServerNetworkManager(ServerNetworkManager&&) noexcept = default;
    ServerNetworkManager& operator=(ServerNetworkManager&&) noexcept = default;
    
    /**
     * Initialize network (bind socket)
     */
    bool initialize(uint16_t port);
    
    /**
     * Shutdown network
     */
    void shutdown();
    
    /**
     * Process incoming packets
     * Returns number of packets processed
     */
    int processPackets();
    
    /**
     * Send packet to specific client
     */
    bool sendPacket(const game::network::Address& address, const game::network::Packet& packet);
    
    /**
     * Broadcast packet to all connected clients
     */
    void broadcastPacket(const game::network::Packet& packet);
    
    /**
     * Handle client connection
     * Returns entity for the new client (invalid if already connected)
     * @param initialPosition Optional initial position from client
     */
    game::core::Entity handleConnect(const game::network::Address& address, const sf::Vector2f& initialPosition = sf::Vector2f(0, 0));
    
    /**
     * Get initial position for a client (stored during CONNECT)
     */
    sf::Vector2f getClientInitialPosition(const game::network::Address& address) const;
    
    /**
     * Handle client disconnect
     */
    void handleDisconnect(const game::network::Address& address);
    
    /**
     * Check for connection timeouts
     */
    void checkTimeouts(float timeoutSeconds);
    
    /**
     * Get number of connected clients
     */
    size_t getClientCount() const {
        return connections.size();
    }
    
    /**
     * Get all connected clients
     */
    const std::unordered_map<game::network::Address, ClientConnection, game::network::Address::Hash>& getConnections() const {
        return connections;
    }
    
    /**
     * Get client entity by address
     */
    game::core::Entity getClientEntity(const game::network::Address& address) const {
        auto it = connections.find(address);
        if (it != connections.end() && it->second.connected) {
            return it->second.entity;
        }
        return game::core::Entity();  // Invalid entity
    }
    
    /**
     * Set entity for a client connection
     */
    void setClientEntity(const game::network::Address& address, const game::core::Entity& entity);
    
    /**
     * Get last received INPUT packet for an address (for GameServer processing)
     */
    struct LastInput {
        game::network::Address from;
        game::network::Packet packet;
        bool valid = false;
    };
    LastInput getLastInput(const game::network::Address& address) const;
    
private:
    sf::UdpSocket socket;
    std::unordered_map<game::network::Address, ClientConnection, game::network::Address::Hash> connections;
    mutable std::unordered_map<game::network::Address, sf::Vector2f, game::network::Address::Hash> clientInitialPositions;
    mutable std::unordered_map<game::network::Address, LastInput, game::network::Address::Hash> lastInputPackets;
    uint32_t nextSequenceNumber;
    
    /**
     * Handle incoming packet
     */
    void handlePacket(const game::network::Address& from, const game::network::Packet& packet);
    
    /**
     * Send connect acknowledgment
     */
    void sendConnectAck(const game::network::Address& address, game::core::Entity::ID entityID);
};

} // namespace game::server

