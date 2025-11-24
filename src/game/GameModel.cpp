#include "GameModel.hpp"
#include <LDtkLoader/Project.hpp>
#include <iostream>

namespace game::client {

void GameModel::init(const ldtk::Project& ldtk, bool reloading) {
    // Get the world from the project
    auto& world = ldtk.getWorld();
    
    // Get first level from the world
    auto& ldtk_level0 = world.getLevel("Level_0");
    
    // Load the TileMap from the level
    TileMap::path = ldtk.getFilePath().directory();
    tilemap.load(ldtk_level0);
    
    // Initialize network client
    if (!reloading) {
        if (networkClient.initialize()) {
            // Connect with initial player position from LDtk
            if (networkClient.connect(serverIp, serverPort, initialPlayerPosition)) {
                connectedToServer = true;
                std::cout << "Connecting to server " << serverIp << ":" << serverPort 
                          << " with initial position (" << initialPlayerPosition.x 
                          << ", " << initialPlayerPosition.y << ")..." << std::endl;
            } else {
                std::cerr << "Failed to connect to server" << std::endl;
            }
        } else {
            std::cerr << "Failed to initialize network client" << std::endl;
        }
    }
    
    // Get Entities layer from level_0
    auto& entities_layer = ldtk_level0.getLayer("Entities");
    
    // Retrieve collider entities from entities layer and store them in the colliders vector
    colliders.clear();
    for (ldtk::Entity& col : entities_layer.getEntitiesByName("Collider")) {
        colliders.emplace_back(
            (float)col.getPosition().x, (float)col.getPosition().y,
            (float)col.getSize().x, (float)col.getSize().y
        );
    }
    
    // Get the Player entity, and its 'color' field
    auto& player_ent = entities_layer.getEntitiesByName("Player")[0].get();
    auto& player_color = player_ent.getField<ldtk::Color>("color").value();
    
    // Initialize player shape
    player.setSize({8, 16});
    player.setOrigin(4, 16);
    if (!reloading) {
        // Use fixed position (176, 256) as requested by user
        initialPlayerPosition = sf::Vector2f(184.0f, 272.0f);
        player.setPosition(initialPlayerPosition);
    }
    player.setFillColor({player_color.r, player_color.g, player_color.b});
    
    // Create camera view
    camera.setSize({400, 250});
    camera.zoom(0.55f);
    camera.setCenter(player.getPosition());
    camera_bounds.left = 0;
    camera_bounds.top = 0;
    camera_bounds.width = static_cast<float>(ldtk_level0.size.x);
    camera_bounds.height = static_cast<float>(ldtk_level0.size.y);
}

} // namespace game::client

