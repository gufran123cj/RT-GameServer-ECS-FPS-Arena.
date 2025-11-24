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
    std::chrono::steady_clock::time_point lastDebugLogTime;
    
    // Internal state for collision handling
    sf::Vector2f lastValidPosition;
    bool hasLastValidPosition = false;
    bool serverPositionInvalid = false;
    
    /**
     * Initialize game from LDtk project
     */
    void init(const ldtk::Project& ldtk, bool reloading = false);
};

} // namespace game::client

