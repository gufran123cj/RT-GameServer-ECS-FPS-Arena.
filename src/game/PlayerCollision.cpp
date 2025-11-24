#include "PlayerCollision.hpp"

namespace game::client {

sf::FloatRect PlayerCollision::getPlayerCollider(const sf::Shape& player) {
    auto bounds = player.getGlobalBounds();
    sf::FloatRect rect;
    rect.left = bounds.left;
    rect.width = bounds.width;
    rect.top = bounds.top + bounds.height / 2;
    rect.height = bounds.height / 2;
    return rect;
}

bool PlayerCollision::checkCollision(const sf::Shape& player, const std::vector<sf::FloatRect>& colliders) {
    auto playerCollider = getPlayerCollider(player);
    for (const auto& collider : colliders) {
        if (playerCollider.intersects(collider)) {
            return true;
        }
    }
    return false;
}

bool PlayerCollision::wouldCollideAt(const sf::Vector2f& position, const sf::Vector2f& playerSize,
                                     const std::vector<sf::FloatRect>& colliders) {
    // Create temporary player bounds at given position
    sf::FloatRect playerBounds;
    playerBounds.left = position.x - playerSize.x * 0.5f;
    playerBounds.top = position.y - playerSize.y;
    playerBounds.width = playerSize.x;
    playerBounds.height = playerSize.y;
    
    // Check bottom half (collider area)
    sf::FloatRect playerCollider;
    playerCollider.left = playerBounds.left;
    playerCollider.width = playerBounds.width;
    playerCollider.top = playerBounds.top + playerBounds.height / 2;
    playerCollider.height = playerBounds.height / 2;
    
    for (const auto& collider : colliders) {
        if (playerCollider.intersects(collider)) {
            return true;
        }
    }
    return false;
}

sf::RectangleShape PlayerCollision::getColliderShape(const sf::FloatRect& rect) {
    sf::RectangleShape r;
    r.setSize({rect.width, rect.height});
    r.setPosition(rect.left, rect.top);
    r.setFillColor({200, 0, 0, 95});
    return r;
}

} // namespace game::client

