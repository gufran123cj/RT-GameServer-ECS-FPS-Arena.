#pragma once

#include <SFML/Graphics.hpp>
#include "GameModel.hpp"
#include "PlayerCollision.hpp"
#include "../network/Packet.hpp"
#include "../network/PacketTypes.hpp"
#include "../../include/common/types.hpp"

namespace game::client {

/**
 * Game Controller
 * 
 * Handles input processing, game logic updates, and network communication
 */
class GameController {
public:
    /**
     * Update game state
     * @param window The render window to check focus state for input
     */
    static void update(GameModel& model, const sf::Window& window);
    
    /**
     * Process network packets
     */
    static void processNetwork(GameModel& model);
    
    /**
     * Handle keyboard input and send to server
     * @param window The render window to check focus state
     */
    static void handleInput(GameModel& model, const sf::Window& window);
    
    /**
     * Update player position from server snapshot
     */
    static void updatePlayerPosition(GameModel& model);
    
    /**
     * Check if movement would cause collision
     */
    static bool wouldCollide(const GameModel& model, float velX, float velY);
    
    /**
     * Handle mouse click shooting input
     * @param window The render window (for mouse position)
     * @param camera The camera view (for world position conversion)
     */
    static void handleShoot(GameModel& model, const sf::RenderWindow& window, const sf::View& camera);
    
    /**
     * Interpolate entity position for smooth movement
     * @param entity Remote entity with position data
     * @param deltaTime Time since last frame
     * @return Interpolated position
     */
    static sf::Vector2f interpolateEntityPosition(const GameClient::RemoteEntity& entity, float deltaTime);
};

} // namespace game::client

