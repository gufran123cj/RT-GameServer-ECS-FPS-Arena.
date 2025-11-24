#include "GameController.hpp"
#include <iostream>
#include <chrono>

namespace game::client {

void GameController::update(GameModel& model) {
    // Update player position from server snapshot
    updatePlayerPosition(model);
    
    // Handle input and send to server
    handleInput(model);
}

void GameController::processNetwork(GameModel& model) {
    if (model.connectedToServer) {
        model.networkClient.processPackets();
    }
}

void GameController::updatePlayerPosition(GameModel& model) {
    if (!model.connectedToServer || model.networkClient.myEntityID == game::INVALID_ENTITY) {
        if (model.connectedToServer && model.networkClient.myEntityID == game::INVALID_ENTITY) {
            // Debug: Entity ID not set yet
            static int debugCounter = 0;
            if (debugCounter++ % 60 == 0) {
                std::cout << "[CLIENT] WARNING: Connected but myEntityID is still INVALID!" << std::endl;
            }
        }
        return;
    }
    
    auto it = model.networkClient.remoteEntities.find(model.networkClient.myEntityID);
    if (it == model.networkClient.remoteEntities.end()) {
        // Debug: Entity not found in snapshot
        static int debugCounter = 0;
        if (debugCounter++ % 60 == 0) {
            std::cout << "[CLIENT] WARNING: My entity ID " << model.networkClient.myEntityID 
                      << " not found in snapshot! Total entities: " 
                      << model.networkClient.remoteEntities.size() << std::endl;
        }
        return;
    }
    
    // Server'dan gelen pozisyonu kullan (mavi player'ın pozisyonu)
    sf::Vector2f serverPos = it->second.position;
    
    // Collider kontrolü: Server pozisyonunu uygulamadan önce kontrol et
    sf::Vector2f oldPos = model.player.getPosition();
    model.player.setPosition(serverPos);
    
    // Check collision with colliders
    bool hasCollision = PlayerCollision::checkCollision(model.player, model.colliders);
    
    // Eğer collider çakışması varsa, son geçerli pozisyonu kullan
    if (hasCollision) {
        model.serverPositionInvalid = true;
        if (model.hasLastValidPosition) {
            model.player.setPosition(model.lastValidPosition);
        } else {
            model.player.setPosition(oldPos);
            model.lastValidPosition = oldPos;
            model.hasLastValidPosition = true;
        }
    } else {
        // Geçerli pozisyon, sakla
        model.lastValidPosition = serverPos;
        model.hasLastValidPosition = true;
        model.serverPositionInvalid = false;
    }
}

void GameController::handleInput(GameModel& model) {
    // Calculate input velocity from keyboard
    float velX = 0.0f, velY = 0.0f;
    const float moveSpeed = 60.0f;  // pixels per second (server will use this in fixed timestep)
    const float deltaTime = 1.0f / 60.0f;  // Fixed timestep (server tick rate)
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        velY = -moveSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        velY = moveSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        velX = -moveSpeed;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        velX = moveSpeed;
    
    // Collider kontrolü: Eğer INPUT yönü collider'a doğruysa, velocity'yi sıfırla
    if (model.connectedToServer && model.networkClient.isConnected() && 
        !model.serverPositionInvalid && (velX != 0 || velY != 0)) {
        if (wouldCollide(model, velX, velY)) {
            velX = 0.0f;
            velY = 0.0f;
        }
    }
    
    // EK KONTROL: Server pozisyonu geçersizse INPUT paketlerini hiç gönderme
    if (model.serverPositionInvalid) {
        velX = 0.0f;
        velY = 0.0f;
    }
    
    // Send input to server if connected (sadece server pozisyonu geçerliyse)
    if (model.connectedToServer && model.networkClient.isConnected() && !model.serverPositionInvalid) {
        game::network::Packet inputPacket(game::network::PacketType::INPUT);
        inputPacket.setSequence(1);
        inputPacket.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count()));
        inputPacket.write(velX);
        inputPacket.write(velY);
        
        // Debug: Log sent velocity
        if (velX != 0 || velY != 0) {
            std::cout << "[CLIENT] Sending INPUT: Velocity (" << velX << ", " << velY 
                      << "), Packet size: " << inputPacket.getSize() << " bytes" << std::endl;
        }
        
        model.networkClient.sendPacket(inputPacket);
    }
}

bool GameController::wouldCollide(const GameModel& model, float velX, float velY) {
    const float moveSpeed = 60.0f;
    const float deltaTime = 1.0f / 60.0f;
    
    // Mevcut pozisyonu al
    sf::Vector2f currentPos = model.player.getPosition();
    
    // Bir sonraki pozisyonu hesapla (daha büyük adım - birkaç frame ilerisi)
    const float checkDistance = moveSpeed * deltaTime * 2.0f;  // 2 frame ilerisi
    sf::Vector2f nextPos = currentPos + sf::Vector2f(
        velX != 0 ? (velX > 0 ? checkDistance : -checkDistance) : 0.0f,
        velY != 0 ? (velY > 0 ? checkDistance : -checkDistance) : 0.0f
    );
    
    // Check if would collide at next position
    return PlayerCollision::wouldCollideAt(nextPos, {8.0f, 16.0f}, model.colliders);
}

} // namespace game::client

