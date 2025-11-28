#pragma once

#include <SFML/System/Vector2.hpp>

namespace game::client {

/**
 * Game Constants
 * 
 * Centralized constants for game configuration
 */
namespace Constants {
    // Player settings
    constexpr float PLAYER_MOVE_SPEED = 60.0f;
    constexpr float TARGET_FPS = 60.0f;
    constexpr float FIXED_DELTA_TIME = 1.0f / TARGET_FPS;
    const sf::Vector2f PLAYER_SIZE = {3.0f, 5.0f};
    const sf::Vector2f PLAYER_INITIAL_POSITION = {150.0f, 100.0f};
    
    // Collision prediction
    constexpr float COLLISION_CHECK_FRAMES_AHEAD = 2.0f;
    
    // Camera settings
    const sf::Vector2f CAMERA_SIZE = {400.0f, 250.0f};
    constexpr float CAMERA_ZOOM = 1.0f;
    
    // Window settings
    constexpr unsigned int WINDOW_WIDTH = 800;
    constexpr unsigned int WINDOW_HEIGHT = 500;
    constexpr unsigned int WINDOW_FPS_LIMIT = 60;
    
    // Network settings
    constexpr float HEARTBEAT_INTERVAL = 1.0f;  // seconds
}

} // namespace game::client

