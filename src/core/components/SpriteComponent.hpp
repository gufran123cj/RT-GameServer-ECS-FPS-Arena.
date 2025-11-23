#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>
#include "../Component.hpp"

namespace game::core::components {

/**
 * Sprite Component
 * 
 * Stores rendering information for an entity.
 * Used by RenderSystem to draw entities.
 * 
 * Note: For now, uses simple rectangle shape data.
 * Can be extended later with texture support.
 */
struct SpriteComponent {
    sf::Vector2f size;      // Width and height
    sf::Color color;        // Fill color
    sf::Vector2f origin;    // Origin point (for rotation/positioning)
    
    SpriteComponent() 
        : size(8.0f, 16.0f)
        , color(sf::Color::White)
        , origin(4.0f, 16.0f)  // Default: bottom-center origin
    {}
    
    SpriteComponent(const sf::Vector2f& s, const sf::Color& c = sf::Color::White)
        : size(s)
        , color(c)
        , origin(s.x * 0.5f, s.y)  // Default: bottom-center origin
    {}
};

} // namespace game::core::components

