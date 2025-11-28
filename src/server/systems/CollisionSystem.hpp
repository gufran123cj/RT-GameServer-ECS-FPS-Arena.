#pragma once

#include "../../core/System.hpp"
#include "../../core/Entity.hpp"
#include "../CollisionHelper.hpp"
#include <vector>
#include <SFML/Graphics/Rect.hpp>

namespace game::core {
    class World;
}

namespace game::server::systems {

/**
 * Collision System
 * 
 * Authoritative server-side collision detection.
 * Runs BEFORE MovementSystem to prevent entities from moving into colliders.
 * 
 * Priority: 50 (higher than MovementSystem's 100, so runs first)
 */
class CollisionSystem : public game::core::System {
public:
    /**
     * Constructor
     * @param colliders List of static colliders (walls, obstacles)
     */
    explicit CollisionSystem(const std::vector<sf::FloatRect>& colliders);
    
    ~CollisionSystem() override = default;
    
    /**
     * Update collision system
     * Checks if entities would collide, and prevents movement if so
     */
    void update(float deltaTime, game::core::World& world) override;
    
    /**
     * Get system priority (lower = earlier execution)
     * CollisionSystem must run BEFORE MovementSystem
     */
    int getPriority() const override {
        return 50;  // Lower than MovementSystem (100), so runs first
    }
    
    /**
     * Set colliders (for dynamic map loading)
     */
    void setColliders(const std::vector<sf::FloatRect>& colliders) {
        this->colliders = colliders;
    }

private:
    std::vector<sf::FloatRect> colliders;
    
    /**
     * Check and resolve collision for a single entity
     * @param entityID Entity to check
     * @param world ECS world reference
     * @param deltaTime Time step for position prediction
     * @return True if collision was detected and resolved
     */
    bool checkAndResolveCollision(game::core::Entity::ID entityID, game::core::World& world, float deltaTime);
};

} // namespace game::server::systems

