#include "GameController.hpp"
#include "GameConstants.hpp"
#include <chrono>
#include <iostream>

namespace game::client {

// Static timer for position logging
static std::chrono::steady_clock::time_point lastPositionLogTime = std::chrono::steady_clock::now();
constexpr float POSITION_LOG_INTERVAL = 5.0f;  // seconds

void GameController::update(GameModel& model, const sf::Window& window) {
    // Update player position from server snapshot
    updatePlayerPosition(model);
    
    // Log player position every 5 seconds
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<float>(now - lastPositionLogTime).count();
    if (elapsed >= POSITION_LOG_INTERVAL) {
        sf::Vector2f pos = model.player.getPosition();
        std::cout << "[Position Log] Player X: " << pos.x << ", Y: " << pos.y << std::endl;
        lastPositionLogTime = now;
    }
    
    // Handle input and send to server (only if window has focus)
    handleInput(model, window);
}

void GameController::processNetwork(GameModel& model) {
    if (model.connectedToServer) {
        model.networkClient.processPackets();
    }
}

void GameController::updatePlayerPosition(GameModel& model) {
    if (!model.connectedToServer) {
        return;
    }
    
    if (model.networkClient.myEntityID == game::INVALID_ENTITY) {
        return;
    }
    
    auto it = model.networkClient.remoteEntities.find(model.networkClient.myEntityID);
    if (it == model.networkClient.remoteEntities.end()) {
        return;
    }
    
    sf::Vector2f serverPos = it->second.position;
    
    sf::Vector2f oldPos = model.player.getPosition();
    model.player.setPosition(serverPos);
    
    bool hasCollision = PlayerCollision::checkCollision(model.player, model.colliders);
    
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
        model.lastValidPosition = serverPos;
        model.hasLastValidPosition = true;
        model.serverPositionInvalid = false;
    }
}

void GameController::handleInput(GameModel& model, const sf::Window& window) {
    // CRITICAL: Only process input if this window has focus
    // This prevents all clients from responding to the same keyboard input
    if (!window.hasFocus()) {
        return;  // This window is not active, don't process input
    }
    
    float velX = 0.0f, velY = 0.0f;
    const float moveSpeed = Constants::PLAYER_MOVE_SPEED;
    
    bool wPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool sPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    bool aPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool dPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    bool upPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
    bool downPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
    bool leftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
    bool rightPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
    
    
    if (upPressed || wPressed)
        velY = -moveSpeed;
    if (downPressed || sPressed)
        velY = moveSpeed;
    if (leftPressed || aPressed)
        velX = -moveSpeed;
    if (rightPressed || dPressed)
        velX = moveSpeed;
    
    // Collider kontrolü: Eğer INPUT yönü collider'a doğruysa, velocity'yi sıfırla
    if (model.connectedToServer && model.networkClient.isConnected() && 
        !model.serverPositionInvalid && (velX != 0 || velY != 0)) {
        if (wouldCollide(model, velX, velY)) {
            velX = 0.0f;
            velY = 0.0f;
        }
    }
    
    // EK KONTROL: Server pozisyonu geçersizse, sadece collider'dan UZAKLAŞMA hareketlerine izin ver
    // Bu sayede player collider içine girdiğinde çıkabilir
    if (model.serverPositionInvalid) {
        // Eğer hareket collider'dan UZAKLAŞIYORSA (ters yönde), izin ver
        // Aksi halde input gönderme
        bool movingAwayFromCollider = false;
        if (velX != 0 || velY != 0) {
            // Mevcut pozisyonda collision var mı kontrol et
            bool currentCollision = PlayerCollision::checkCollision(model.player, model.colliders);
            if (currentCollision) {
                // Collider içindeyiz, sadece uzaklaşma hareketlerine izin ver
                // Basit kontrol: eğer hareket yönünde collision yoksa, uzaklaşıyoruz demektir
                sf::Vector2f testPos = model.player.getPosition() + sf::Vector2f(
                    velX != 0 ? (velX > 0 ? 1.0f : -1.0f) : 0.0f,
                    velY != 0 ? (velY > 0 ? 1.0f : -1.0f) : 0.0f
                );
                movingAwayFromCollider = !PlayerCollision::wouldCollideAt(testPos, Constants::PLAYER_SIZE, model.colliders);
            }
        }
        
        if (!movingAwayFromCollider) {
            velX = 0.0f;
            velY = 0.0f;
        }
    }
    
    // CRITICAL: Only send INPUT if we have a valid entity ID assigned by the server
    // Each client should ONLY control its own entity, not others
    // NOTE: Entity ID can be 0 (first client), so we only check for INVALID_ENTITY
    if (model.connectedToServer && 
        model.networkClient.isConnected() && 
        !model.serverPositionInvalid &&
        model.networkClient.myEntityID != game::INVALID_ENTITY) {
        
        game::network::Packet inputPacket(game::network::PacketType::INPUT);
        inputPacket.setSequence(1);
        inputPacket.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count()));
        inputPacket.write(velX);
        inputPacket.write(velY);
        
        model.networkClient.sendPacket(inputPacket);
    }
}

bool GameController::wouldCollide(const GameModel& model, float velX, float velY) {
    const float moveSpeed = Constants::PLAYER_MOVE_SPEED;
    const float deltaTime = Constants::FIXED_DELTA_TIME;
    
    // Mevcut pozisyonu al
    sf::Vector2f currentPos = model.player.getPosition();
    
    // Sadece bir frame ilerisini kontrol et (daha hassas kontrol)
    // Çok erken durdurmak yerine, gerçekten collider'a girecekse durdur
    const float checkDistance = moveSpeed * deltaTime;  // Sadece 1 frame ahead
    sf::Vector2f nextPos = currentPos + sf::Vector2f(
        velX != 0 ? (velX > 0 ? checkDistance : -checkDistance) : 0.0f,
        velY != 0 ? (velY > 0 ? checkDistance : -checkDistance) : 0.0f
    );
    
    // Check if would collide at next position
    return PlayerCollision::wouldCollideAt(nextPos, Constants::PLAYER_SIZE, model.colliders);
}

} // namespace game::client

