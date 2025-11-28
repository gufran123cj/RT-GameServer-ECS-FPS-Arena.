#include "GameView.hpp"

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
    
    // CRITICAL: Draw other players from server snapshot (excluding ourselves)
    // Sadece diğer entity'leri çiz, kendi entity'miz zaten yukarıda çizildi
    if (model.connectedToServer && !model.networkClient.remoteEntities.empty()) {
        for (const auto& [entityID, remoteEntity] : model.networkClient.remoteEntities) {
            // CRITICAL: Skip our own entity (already drawn above as model.player)
            // Bu kontrol çok önemli - kendi entity'mizi iki kez çizmemek için
            // NOTE: Entity ID can be 0 (first client), so we only check for equality
            if (entityID == model.networkClient.myEntityID && model.networkClient.myEntityID != game::INVALID_ENTITY) {
                continue;
            }
            
            // Draw other players (remote entities) - bunlar diğer client'ların entity'leri
            sf::RectangleShape remotePlayer;
            remotePlayer.setSize(remoteEntity.size);
            remotePlayer.setPosition(remoteEntity.position);
            remotePlayer.setFillColor(remoteEntity.color);
            // Set origin to match local player (bottom-center)
            remotePlayer.setOrigin(remoteEntity.size.x * 0.5f, remoteEntity.size.y);
            target.draw(remotePlayer);
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

} // namespace game::client

