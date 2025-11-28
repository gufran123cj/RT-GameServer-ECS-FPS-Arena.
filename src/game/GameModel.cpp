#include "GameModel.hpp"
#include "GameConstants.hpp"
#include <LDtkLoader/Project.hpp>
#include <iostream>

namespace game::client {

void GameModel::init(const ldtk::Project& ldtk, bool reloading) {
    try {
        // Get the world from the project
        auto& world = ldtk.getWorld();
        
        // Get first level from the world
        auto& ldtk_level0 = world.getLevel("World_Level_0");
        
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
        
        // Load colliders from IntGrid "Collisions" layer (new map uses IntGrid instead of Collider entities)
        colliders.clear();
        bool loadedFromIntGrid = false;
        
        try {
            auto& collisions_layer = ldtk_level0.getLayer("Collisions");
            if (collisions_layer.getType() == ldtk::LayerType::IntGrid) {
                // Manually iterate through all grid cells to find walls (value = 1)
                int gridSize = collisions_layer.getCellSize();
                const auto& gridSize_pt = collisions_layer.getGridSize();
                int gridWidth = gridSize_pt.x;
                int gridHeight = gridSize_pt.y;
                
                int wallCount = 0;
                // Check each grid cell
                for (int y = 0; y < gridHeight; ++y) {
                    for (int x = 0; x < gridWidth; ++x) {
                        try {
                            const auto& intGridVal = collisions_layer.getIntGridVal(x, y);
                            // Value 1 = walls
                            if (intGridVal.value == 1) {
                                float pxX = static_cast<float>(x * gridSize);
                                float pxY = static_cast<float>(y * gridSize);
                                float cellSize = static_cast<float>(gridSize);
                                
                                colliders.emplace_back(pxX, pxY, cellSize, cellSize);
                                wallCount++;
                            }
                        } catch (...) {
                            // Skip invalid cells
                            continue;
                        }
                    }
                }
                
                std::cout << "Loading " << wallCount << " collision cells from IntGrid layer..." << std::endl;
                loadedFromIntGrid = true;
            }
        } catch (const std::exception& ex) {
            std::cerr << "WARNING: Could not load collisions from IntGrid layer: " << ex.what() << std::endl;
        }
        
        // Fallback: Try to load from Collider entities (old map format)
        if (!loadedFromIntGrid) {
            std::cerr << "Falling back to Collider entities..." << std::endl;
            for (ldtk::Entity& col : entities_layer.getEntitiesByName("Collider")) {
                colliders.emplace_back(
                    (float)col.getPosition().x, (float)col.getPosition().y,
                    (float)col.getSize().x, (float)col.getSize().y
                );
            }
        }
        
        std::cout << "Total colliders loaded: " << colliders.size() << std::endl;
        
        // Log all collision positions (once at startup)
        std::cout << "\n=== COLLISION POSITIONS ===" << std::endl;
        for (size_t i = 0; i < colliders.size(); ++i) {
            const auto& col = colliders[i];
            std::cout << "Collider[" << i << "]: X=" << col.left << ", Y=" << col.top 
                      << ", W=" << col.width << ", H=" << col.height << std::endl;
        }
        std::cout << "=== END COLLISION POSITIONS ===\n" << std::endl;
        
        // Get the Player entity, and its color
        auto player_entities = entities_layer.getEntitiesByName("Player");
        if (player_entities.empty()) {
            std::cerr << "ERROR: No Player entity found in Entities layer!" << std::endl;
            throw std::runtime_error("Player entity not found");
        }
        auto& player_ent = player_entities[0].get();
        // Use getColor() method instead of field (new map doesn't have "color" field)
        auto& player_color = player_ent.getColor();
    
        // Initialize player shape
        player.setSize(Constants::PLAYER_SIZE);
        player.setOrigin(Constants::PLAYER_SIZE.x * 0.5f, Constants::PLAYER_SIZE.y);
        if (!reloading) {
            initialPlayerPosition = Constants::PLAYER_INITIAL_POSITION;
            player.setPosition(initialPlayerPosition);
        }
        player.setFillColor({player_color.r, player_color.g, player_color.b});
        
        // Create camera view
        camera.setSize(Constants::CAMERA_SIZE);
        camera.zoom(Constants::CAMERA_ZOOM);
        camera.setCenter(player.getPosition());
        camera_bounds.left = 0;
        camera_bounds.top = 0;
        camera_bounds.width = static_cast<float>(ldtk_level0.size.x);
        camera_bounds.height = static_cast<float>(ldtk_level0.size.y);
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR in GameModel::init: " << ex.what() << std::endl;
        throw;  // Re-throw to let main() handle it
    }
}

} // namespace game::client

