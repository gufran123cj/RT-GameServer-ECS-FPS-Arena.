#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

namespace game::client {

/**
 * Player Collision Helper
 * 
 * Collision detection utilities for player entity
 */
class PlayerCollision {
public:
    /**
     * Get player collider rectangle (bottom half of player)
     */
    static sf::FloatRect getPlayerCollider(const sf::Shape& player);
    
    /**
     * Check if player collider intersects with any collider
     */
    static bool checkCollision(const sf::Shape& player, const std::vector<sf::FloatRect>& colliders);
    
    /**
     * Check if player would collide at given position
     */
    static bool wouldCollideAt(const sf::Vector2f& position, const sf::Vector2f& playerSize, 
                               const std::vector<sf::FloatRect>& colliders);
    
    /**
     * Get collider shape for rendering (debug)
     */
    static sf::RectangleShape getColliderShape(const sf::FloatRect& rect);
};

} // namespace game::client

