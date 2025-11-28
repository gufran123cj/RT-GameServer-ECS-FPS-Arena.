#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <vector>

namespace game::server {

/**
 * Collision Helper
 * 
 * Server-side collision detection utilities
 * Similar to client-side PlayerCollision but for server authority
 */
class CollisionHelper {
public:
    /**
     * Get player collider rectangle (bottom half of player)
     * @param position Player position (center-bottom)
     * @param playerSize Player size (width, height)
     * @return Collider rectangle (bottom half)
     */
    static sf::FloatRect getPlayerCollider(const sf::Vector2f& position, const sf::Vector2f& playerSize);
    
    /**
     * Check if player collider intersects with any collider
     * @param position Player position
     * @param playerSize Player size
     * @param colliders List of static colliders
     * @return True if collision detected
     */
    static bool checkCollision(const sf::Vector2f& position, const sf::Vector2f& playerSize,
                               const std::vector<sf::FloatRect>& colliders);
    
    /**
     * Check if player would collide at given position
     * @param position Target position
     * @param playerSize Player size
     * @param colliders List of static colliders
     * @return True if would collide
     */
    static bool wouldCollideAt(const sf::Vector2f& position, const sf::Vector2f& playerSize,
                                const std::vector<sf::FloatRect>& colliders);
    
    /**
     * Resolve collision by adjusting position
     * Moves player back to last valid position
     * @param currentPos Current position (will be modified)
     * @param lastValidPos Last valid position (no collision)
     * @param playerSize Player size
     * @param colliders List of static colliders
     * @return True if collision was resolved
     */
    static bool resolveCollision(sf::Vector2f& currentPos, const sf::Vector2f& lastValidPos,
                                 const sf::Vector2f& playerSize,
                                 const std::vector<sf::FloatRect>& colliders);
};

} // namespace game::server

