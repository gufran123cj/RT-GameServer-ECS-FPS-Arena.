#include "ServerNetworkManager.hpp"
#include "../network/PacketTypes.hpp"
#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <algorithm>

namespace game::server {

ServerNetworkManager::ServerNetworkManager() : nextSequenceNumber(1) {
}

ServerNetworkManager::~ServerNetworkManager() {
    shutdown();
}

bool ServerNetworkManager::initialize(uint16_t port) {
    if (socket.bind(port) != sf::Socket::Status::Done) {
        std::cerr << "Failed to bind socket to port " << port << std::endl;
        return false;
    }
    
    socket.setBlocking(false);  // Non-blocking mode
    std::cout << "Server listening on port " << port << std::endl;
    return true;
}

void ServerNetworkManager::shutdown() {
    connections.clear();
    socket.unbind();
}

int ServerNetworkManager::processPackets() {
    int packetCount = 0;
    
    // Temporary buffer for receiving
    static uint8_t receiveBuffer[MAX_PACKET_SIZE];
    
    while (true) {
        std::size_t received = 0;
        sf::IpAddress senderIp;
        unsigned short senderPort;
        
        sf::Socket::Status status = socket.receive(
            receiveBuffer,
            MAX_PACKET_SIZE,
            received,
            senderIp,
            senderPort
        );
        
        if (status == sf::Socket::Status::NotReady || status == sf::Socket::Status::Disconnected) {
            break;  // No more packets
        }
        
        if (status == sf::Socket::Status::Done && received > 0) {
            // Create address from received data
            game::network::Address from(senderIp, senderPort);
            
            // Create packet from received data
            game::network::Packet packet;
            packet.setData(receiveBuffer, received);
            
            handlePacket(from, packet);
            packetCount++;
        }
    }
    
    return packetCount;
}

ServerNetworkManager::LastInput ServerNetworkManager::getLastInput(const game::network::Address& address) const {
    auto it = lastInputPackets.find(address);
    if (it != lastInputPackets.end() && it->second.valid) {
        LastInput result = it->second;
        it->second.valid = false;  // Mark as consumed
        return result;
    }
    return LastInput{};
}

void ServerNetworkManager::handlePacket(const game::network::Address& from, const game::network::Packet& packet) {
    game::network::PacketType type = packet.getType();
    
    switch (type) {
        case game::network::PacketType::CONNECT: {
            std::cout << "Client connecting from " << from.toString() << std::endl;
            // Read initial position from packet
            game::network::Packet& nonConstPacket = const_cast<game::network::Packet&>(packet);
            nonConstPacket.resetRead();
            float posX = 0, posY = 0;
            nonConstPacket.read(posX);
            nonConstPacket.read(posY);
            sf::Vector2f initialPos(posX, posY);
            game::core::Entity entity = handleConnect(from, initialPos);
            sendConnectAck(from, entity.id);
            break;
        }
        
        case game::network::PacketType::INPUT: {
            // Store INPUT packet for GameServer to process
            lastInputPackets[from] = {from, packet, true};
            break;
        }
        
        case game::network::PacketType::HEARTBEAT: {
            auto it = connections.find(from);
            if (it != connections.end()) {
                it->second.lastHeartbeat = std::chrono::steady_clock::now();
            }
            break;
        }
        
        case game::network::PacketType::DISCONNECT: {
            std::cout << "Client disconnecting from " << from.toString() << std::endl;
            handleDisconnect(from);
            break;
        }
        
        default:
            // Other packet types handled by systems
            break;
    }
}

bool ServerNetworkManager::sendPacket(const game::network::Address& address, const game::network::Packet& packet) {
    sf::Socket::Status status = socket.send(
        packet.getData(),
        static_cast<std::size_t>(packet.getSize()),
        address.getIpAddress(),
        address.getPort()
    );
    
    return status == sf::Socket::Status::Done;
}

void ServerNetworkManager::broadcastPacket(const game::network::Packet& packet) {
    for (const auto& [addr, conn] : connections) {
        if (conn.connected) {
            sendPacket(addr, packet);
        }
    }
}

game::core::Entity ServerNetworkManager::handleConnect(const game::network::Address& address, const sf::Vector2f& initialPosition) {
    // Check if client already connected
    auto it = connections.find(address);
    if (it != connections.end() && it->second.connected) {
        return it->second.entity;  // Already connected
    }
    
    // Store initial position
    clientInitialPositions[address] = initialPosition;
    
    // Create new connection with invalid entity (will be set by GameServer)
    game::core::Entity invalidEntity;  // Invalid entity, will be set by caller
    connections[address] = ClientConnection(address, invalidEntity);
    
    std::cout << "Client connected: " << address.toString() 
              << " (Total clients: " << connections.size() << ")" << std::endl;
    
    return invalidEntity;  // Return invalid to signal new connection
}

sf::Vector2f ServerNetworkManager::getClientInitialPosition(const game::network::Address& address) const {
    auto it = clientInitialPositions.find(address);
    if (it != clientInitialPositions.end()) {
        return it->second;
    }
    return sf::Vector2f(0, 0);  // Default position
}

void ServerNetworkManager::setClientEntity(const game::network::Address& address, const game::core::Entity& entity) {
    auto it = connections.find(address);
    if (it != connections.end()) {
        it->second.entity = entity;
    }
}

void ServerNetworkManager::handleDisconnect(const game::network::Address& address) {
    auto it = connections.find(address);
    if (it != connections.end()) {
        it->second.connected = false;
        connections.erase(it);
        std::cout << "Client disconnected: " << address.toString() 
                  << " (Remaining clients: " << connections.size() << ")" << std::endl;
    }
}

void ServerNetworkManager::checkTimeouts(float timeoutSeconds) {
    auto now = std::chrono::steady_clock::now();
    auto timeout = std::chrono::duration<float>(timeoutSeconds);
    
    auto it = connections.begin();
    while (it != connections.end()) {
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<float>>(
            now - it->second.lastHeartbeat
        );
        
        if (elapsed > timeout) {
            std::cout << "Client timeout: " << it->second.address.toString() << std::endl;
            it = connections.erase(it);
        } else {
            ++it;
        }
    }
}

void ServerNetworkManager::sendConnectAck(const game::network::Address& address, game::core::Entity::ID entityID) {
    game::network::Packet packet(game::network::PacketType::CONNECT_ACK);
    packet.setSequence(nextSequenceNumber++);
    packet.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count()));
    
    packet.write(entityID);
    sendPacket(address, packet);
}

} // namespace game::server

