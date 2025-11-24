#include "GameView.hpp"

namespace game::client {

void GameView::render(sf::RenderTarget& target, GameModel& model) {
    target.setView(model.camera);
    
    // Draw map background layers
    target.draw(model.tilemap.getLayer("Ground"));
    target.draw(model.tilemap.getLayer("Trees"));
    
    if (model.show_colliders) {
        // Draw map colliders
        for (auto& rect : model.colliders) {
            target.draw(PlayerCollision::getColliderShape(rect));
        }
    }
    
    // Draw local player (always visible, position updated from server if connected)
    target.draw(model.player);
    
    // Draw other players from server snapshot (excluding ourselves)
    if (model.connectedToServer && !model.networkClient.remoteEntities.empty()) {
        for (const auto& [entityID, remoteEntity] : model.networkClient.remoteEntities) {
            // Skip our own entity (already drawn above)
            if (entityID == model.networkClient.myEntityID && model.networkClient.myEntityID != 0) {
                continue;
            }
            
            // Draw other players (remote entities)
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
    
    // Draw map top layer
    target.draw(model.tilemap.getLayer("Trees_top"));
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

