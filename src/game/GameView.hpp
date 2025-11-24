#pragma once

#include <SFML/Graphics.hpp>
#include "GameModel.hpp"
#include "PlayerCollision.hpp"

namespace game::client {

/**
 * Game View
 * 
 * Handles all rendering operations
 */
class GameView {
public:
    /**
     * Render the game
     */
    static void render(sf::RenderTarget& target, GameModel& model);
    
    /**
     * Update camera to follow player
     */
    static void updateCamera(GameModel& model);
};

} // namespace game::client

