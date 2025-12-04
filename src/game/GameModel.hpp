#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "../TileMap.hpp"
#include "GameClient.hpp"

namespace game::client {

/**
 * Game Model
 * 
 * Contains all game data and state
 */
class GameModel {
public:
    // Game entities
    sf::RectangleShape player;
    std::vector<sf::FloatRect> colliders;
    
    // Map
    TileMap tilemap;
    
    // Network
    GameClient networkClient;
    bool connectedToServer = false;
    std::string serverIp = "127.0.0.1";
    uint16_t serverPort = 7777;
    sf::Vector2f initialPlayerPosition;
    
    // Camera
    sf::View camera;
    sf::FloatRect camera_bounds;
    
    // Debug
    bool show_colliders = false;
    
    // Internal state for collision handling
    sf::Vector2f lastValidPosition;
    bool hasLastValidPosition = false;
    bool serverPositionInvalid = false;
    
    // Game state
    // Note: shouldQuit removed - players now respawn instead of quitting
    
    // Player health (from server snapshot)
    float playerHealth = 10.0f;
    float playerMaxHealth = 10.0f;
    bool playerIsDead = false;  // True when health <= 0
    
    // Player kill count (from server snapshot)
    int playerKillCount = 0;
    
    // Frame timing for interpolation
    float deltaTime = 0.016f;  // Default 60 FPS (will be updated each frame)
    
    /**
     * Initialize game from LDtk project
     */
    void init(const ldtk::Project& ldtk, bool reloading = false);
};

} // namespace game::client

