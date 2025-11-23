#pragma once

#include <SFML/System/Vector2.hpp>
#include "../Component.hpp"

namespace game::core::components {

/**
 * Velocity Component
 * 
 * Stores 2D velocity of an entity (pixels per second).
 * Used by MovementSystem to update PositionComponent.
 */
struct VelocityComponent {
    sf::Vector2f velocity;
    
    VelocityComponent() : velocity(0.0f, 0.0f) {}
    VelocityComponent(float x, float y) : velocity(x, y) {}
    VelocityComponent(const sf::Vector2f& vel) : velocity(vel) {}
};

} // namespace game::core::components

