#include "GameView.hpp"
#include "GameController.hpp"

namespace game::client {

void GameView::render(sf::RenderTarget& target, GameModel& model) {
    try {
        target.setView(model.camera);
        
        // Draw map background layers (try-catch for each layer in case names differ)
        // New map uses "Default_floor" instead of "Ground"
        try {
            target.draw(model.tilemap.getLayer("Default_floor"));
        } catch (const std::exception& ex) {
            // Try old layer name as fallback
            try {
                target.draw(model.tilemap.getLayer("Ground"));
            } catch (const std::exception& ex2) {
                std::cerr << "WARNING: Could not draw floor layer: " << ex2.what() << std::endl;
            }
        }
    
    if (model.show_colliders) {
        // Draw map colliders
        for (auto& rect : model.colliders) {
            target.draw(PlayerCollision::getColliderShape(rect));
        }
    }
    
    // CRITICAL: Draw local player (SADECE KENDİ ENTITY'MİZİN pozisyonu)
    // model.player pozisyonu updatePlayerPosition() tarafından sadece kendi entity'mizin pozisyonu ile güncelleniyor
    target.draw(model.player);
    
    // CRITICAL: Draw other players and projectiles from server snapshot (excluding ourselves)
    // Sadece diğer entity'leri çiz, kendi entity'miz zaten yukarıda çizildi
    if (model.connectedToServer && !model.networkClient.remoteEntities.empty()) {
        for (const auto& [entityID, remoteEntity] : model.networkClient.remoteEntities) {
            // CRITICAL: Skip our own entity (already drawn above as model.player)
            // Bu kontrol çok önemli - kendi entity'mizi iki kez çizmemek için
            // NOTE: Entity ID can be 0 (first client), so we only check for equality
            if (entityID == model.networkClient.myEntityID && model.networkClient.myEntityID != game::INVALID_ENTITY) {
                continue;
            }
            
            // Interpolate position for smooth rendering
            sf::Vector2f renderPos = GameController::interpolateEntityPosition(remoteEntity, model.deltaTime);
            
            // Draw remote entities (players and projectiles)
            // Projectile'lar küçük (2x2), player'lar büyük (3x5) - size'a göre ayırt edilebilir
            sf::RectangleShape entityShape;
            entityShape.setSize(remoteEntity.size);
            entityShape.setPosition(renderPos);  // Use interpolated position
            entityShape.setFillColor(remoteEntity.color);
            
            // Player'lar için origin bottom-center, projectile'lar için center
            if (remoteEntity.size.y > 4.0f) {
                // Player (size.y > 4 means it's a player, not a projectile)
                entityShape.setOrigin(remoteEntity.size.x * 0.5f, remoteEntity.size.y);
            } else {
                // Projectile (small size, center origin)
                entityShape.setOrigin(remoteEntity.size.x * 0.5f, remoteEntity.size.y * 0.5f);
            }
            
            target.draw(entityShape);
        }
    }
    
    if (model.show_colliders) {
        // Draw player collider
        target.draw(PlayerCollision::getColliderShape(
            PlayerCollision::getPlayerCollider(model.player)
        ));
    }
    
        // Draw map top layer (walls, trees, etc.)
        // New map uses "Wall_tops" instead of "Trees_top"
        try {
            target.draw(model.tilemap.getLayer("Wall_tops"));
        } catch (const std::exception& ex) {
            // Try old layer name as fallback
            try {
                target.draw(model.tilemap.getLayer("Trees_top"));
            } catch (const std::exception& ex2) {
                std::cerr << "WARNING: Could not draw top layer: " << ex2.what() << std::endl;
            }
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR in GameView::render: " << ex.what() << std::endl;
        throw;
    }
    
    // Render HUD (health bar, etc.) - always on top, in screen coordinates
    sf::View defaultView = target.getDefaultView();
    target.setView(defaultView);
    
    renderHealthBar(target, model);
    
    if (model.playerIsDead) {
        renderDeathMessage(target, model);
    }
}

void GameView::updateCamera(GameModel& model) {
    // Update camera (follow player position from server)
    model.camera.move((model.player.getPosition() - model.camera.getCenter()) / 5.f);
    
    auto cam_size = model.camera.getSize();
    
    // Check for camera X limit
    {
        auto cam_pos = model.camera.getCenter();
        if (cam_pos.x - cam_size.x / 2 < model.camera_bounds.left) {
            model.camera.setCenter(model.camera_bounds.left + cam_size.x / 2, cam_pos.y);
        }
        else if (cam_pos.x + cam_size.x / 2 > model.camera_bounds.left + model.camera_bounds.width) {
            model.camera.setCenter(model.camera_bounds.left + model.camera_bounds.width - cam_size.x / 2, cam_pos.y);
        }
    }
    
    // Check for camera Y limit
    {
        auto cam_pos = model.camera.getCenter();
        if (cam_pos.y - cam_size.y / 2 < model.camera_bounds.top) {
            model.camera.setCenter(cam_pos.x, model.camera_bounds.top + cam_size.y / 2);
        }
        else if (cam_pos.y + cam_size.y / 2 > model.camera_bounds.top + model.camera_bounds.height) {
            model.camera.setCenter(cam_pos.x, model.camera_bounds.top + model.camera_bounds.height - cam_size.y / 2);
        }
    }
}

void GameView::renderHealthBar(sf::RenderTarget& target, const GameModel& model) {
    if (!model.connectedToServer) {
        return;  // Don't show health bar if not connected
    }
    
    const float barWidth = 200.0f;
    const float barHeight = 20.0f;
    const float barX = 10.0f;
    const float barY = 10.0f;
    const float borderThickness = 2.0f;
    
    // Health percentage
    float healthPercent = (model.playerMaxHealth > 0.0f) ? 
        (model.playerHealth / model.playerMaxHealth) : 0.0f;
    if (healthPercent < 0.0f) healthPercent = 0.0f;
    if (healthPercent > 1.0f) healthPercent = 1.0f;
    
    // Background (black border)
    sf::RectangleShape background;
    background.setSize(sf::Vector2f(barWidth + borderThickness * 2, barHeight + borderThickness * 2));
    background.setPosition(barX - borderThickness, barY - borderThickness);
    background.setFillColor(sf::Color::Black);
    target.draw(background);
    
    // Health bar background (red/dark)
    sf::RectangleShape healthBarBg;
    healthBarBg.setSize(sf::Vector2f(barWidth, barHeight));
    healthBarBg.setPosition(barX, barY);
    healthBarBg.setFillColor(sf::Color(100, 0, 0, 255));  // Dark red
    target.draw(healthBarBg);
    
    // Health bar fill (green to red based on health)
    if (healthPercent > 0.0f) {
        sf::RectangleShape healthBar;
        healthBar.setSize(sf::Vector2f(barWidth * healthPercent, barHeight));
        healthBar.setPosition(barX, barY);
        
        // Color interpolation: green (100%) -> yellow (50%) -> red (0%)
        if (healthPercent > 0.5f) {
            // Green to yellow
            float t = (healthPercent - 0.5f) * 2.0f;  // 0.5-1.0 -> 0.0-1.0
            healthBar.setFillColor(sf::Color(
                static_cast<uint8_t>(255 * (1.0f - t)),  // R: 0 -> 255
                255,  // G: always 255
                0     // B: 0
            ));
        } else {
            // Yellow to red
            float t = healthPercent * 2.0f;  // 0.0-0.5 -> 0.0-1.0
            healthBar.setFillColor(sf::Color(
                255,  // R: always 255
                static_cast<uint8_t>(255 * t),  // G: 255 -> 0
                0     // B: 0
            ));
        }
        
        target.draw(healthBar);
    }
    
    // Health text (optional - can be added later with font)
    // For now, just the bar is enough
}

void GameView::renderDeathMessage(sf::RenderTarget& target, const GameModel& model) {
    if (!model.playerIsDead) {
        return;
    }
    
    // Dark overlay (semi-transparent black)
    sf::RectangleShape overlay;
    sf::Vector2u windowSize = target.getSize();
    overlay.setSize(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
    overlay.setPosition(0, 0);
    overlay.setFillColor(sf::Color(0, 0, 0, 180));  // Semi-transparent black
    target.draw(overlay);
    
    // "YOU DIED" text would go here if we had a font
    // For now, we'll use a simple rectangle as placeholder
    // TODO: Add font rendering for "YOU DIED" text
}

void GameView::renderFPS(sf::RenderTarget& target, const GameModel& model) {
    // Calculate FPS from deltaTime
    float fps = 0.0f;
    if (model.deltaTime > 0.0f) {
        fps = 1.0f / model.deltaTime;
    }
    
    // Position: top-right corner
    sf::Vector2u windowSize = target.getSize();
    const float boxX = static_cast<float>(windowSize.x) - 120.0f;
    const float boxY = 10.0f;
    const float boxWidth = 110.0f;
    const float boxHeight = 30.0f;
    
    // Text box style (simple box, no bar, no indicator)
    sf::RectangleShape textBox;
    textBox.setSize(sf::Vector2f(boxWidth, boxHeight));
    textBox.setPosition(boxX, boxY);
    textBox.setFillColor(sf::Color(0, 0, 0, 200));  // Semi-transparent black
    textBox.setOutlineThickness(2.0f);
    textBox.setOutlineColor(sf::Color::White);
    target.draw(textBox);
    
    // Note: FPS text would be displayed here if we had a font
    // For now, just the text box is shown
    // Format would be: "FPS: XX"
}

} // namespace game::client

