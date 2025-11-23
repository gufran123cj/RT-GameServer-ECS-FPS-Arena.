#pragma once

#include <SFML/System/Vector2.hpp>
#include "../Component.hpp"

namespace game::core::components {

/**
 * Position Component
 * 
 * Stores 2D position of an entity.
 * Uses SFML's Vector2f for compatibility with SFML rendering.
 */
struct PositionComponent {
    sf::Vector2f position;
    
    PositionComponent() : position(0.0f, 0.0f) {}
    PositionComponent(float x, float y) : position(x, y) {}
    PositionComponent(const sf::Vector2f& pos) : position(pos) {}
};

} // namespace game::core::components

