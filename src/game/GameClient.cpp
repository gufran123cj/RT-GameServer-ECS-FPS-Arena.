#include "GameClient.hpp"
#include <SFML/Graphics.hpp>

namespace game::client {

void GameClient::onConnectAck(game::core::Entity::ID entityID) {
    std::cout << "✅ Connected to server! Entity ID: " << entityID << std::endl;
    myEntityID = entityID;
}

void GameClient::onSnapshot(game::network::Packet& packet) {
    packet.resetRead();
    
    uint32_t entityCount;
    if (!packet.read(entityCount)) {
        return;
    }
    
    // Clear old entities
    remoteEntities.clear();
    
    // Read entities from snapshot
    for (uint32_t i = 0; i < entityCount; ++i) {
        game::core::Entity::ID entityID;
        if (!packet.read(entityID)) break;
        
        float posX, posY;
        if (!packet.read(posX) || !packet.read(posY)) break;
        
        float sizeX, sizeY;
        if (!packet.read(sizeX) || !packet.read(sizeY)) break;
        
        uint8_t r, g, b, a;
        if (!packet.read(r) || !packet.read(g) || !packet.read(b) || !packet.read(a)) break;
        
        RemoteEntity entity;
        entity.position = sf::Vector2f(posX, posY);
        entity.size = sf::Vector2f(sizeX, sizeY);
        entity.color = sf::Color(r, g, b, a);
        
        remoteEntities[entityID] = entity;
        
        // Debug: Log if this is our entity
        if (entityID == myEntityID) {
            std::cout << "[CLIENT] Received snapshot update for MY entity " << entityID 
                      << " at position (" << posX << ", " << posY << ")" << std::endl;
        }
    }
}

void GameClient::onDisconnect() {
    std::cout << "Disconnected from server" << std::endl;
    remoteEntities.clear();
    myEntityID = 0;
}

} // namespace game::client

