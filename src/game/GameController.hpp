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
     */
    static void update(GameModel& model);
    
    /**
     * Process network packets
     */
    static void processNetwork(GameModel& model);
    
    /**
     * Handle keyboard input and send to server
     */
    static void handleInput(GameModel& model);
    
    /**
     * Update player position from server snapshot
     */
    static void updatePlayerPosition(GameModel& model);
    
    /**
     * Check if movement would cause collision
     */
    static bool wouldCollide(const GameModel& model, float velX, float velY);
};

} // namespace game::client

