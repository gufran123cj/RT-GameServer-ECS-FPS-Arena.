#include "CollisionHelper.hpp"
#include <algorithm>

namespace game::server {

sf::FloatRect CollisionHelper::getPlayerCollider(const sf::Vector2f& position, const sf::Vector2f& playerSize) {
    // Player position is center-bottom, so calculate bounds
    sf::FloatRect playerBounds;
    playerBounds.left = position.x - playerSize.x * 0.5f;
    playerBounds.top = position.y - playerSize.y;
    playerBounds.width = playerSize.x;
    playerBounds.height = playerSize.y;
    
    // Return bottom half (collider area)
    sf::FloatRect playerCollider;
    playerCollider.left = playerBounds.left;
    playerCollider.width = playerBounds.width;
    playerCollider.top = playerBounds.top + playerBounds.height * 0.5f;
    playerCollider.height = playerBounds.height * 0.5f;
    
    return playerCollider;
}

bool CollisionHelper::checkCollision(const sf::Vector2f& position, const sf::Vector2f& playerSize,
                                      const std::vector<sf::FloatRect>& colliders) {
    sf::FloatRect playerCollider = getPlayerCollider(position, playerSize);
    
    for (const auto& collider : colliders) {
        if (playerCollider.intersects(collider)) {
            return true;
        }
    }
    return false;
}

bool CollisionHelper::wouldCollideAt(const sf::Vector2f& position, const sf::Vector2f& playerSize,
                                      const std::vector<sf::FloatRect>& colliders) {
    return checkCollision(position, playerSize, colliders);
}

bool CollisionHelper::resolveCollision(sf::Vector2f& currentPos, const sf::Vector2f& lastValidPos,
                                        const sf::Vector2f& playerSize,
                                        const std::vector<sf::FloatRect>& colliders) {
    // If current position has collision, revert to last valid position
    if (checkCollision(currentPos, playerSize, colliders)) {
        currentPos = lastValidPos;
        return true;  // Collision resolved
    }
    return false;  // No collision to resolve
}

} // namespace game::server

