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
    
    /**
     * Render health bar (HUD overlay)
     */
    static void renderHealthBar(sf::RenderTarget& target, const GameModel& model);
    
    /**
     * Render "YOU DIED" message
     */
    static void renderDeathMessage(sf::RenderTarget& target, const GameModel& model);
    
    /**
     * Render FPS counter (top-right corner, text box style)
     */
    static void renderFPS(sf::RenderTarget& target, const GameModel& model);
};

} // namespace game::client

