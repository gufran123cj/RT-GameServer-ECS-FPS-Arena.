#include "CollisionSystem.hpp"
#include "../../core/World.hpp"
#include "../../core/components/PositionComponent.hpp"
#include "../../core/components/VelocityComponent.hpp"
#include "../../core/components/SpriteComponent.hpp"
#include "../CollisionHelper.hpp"

namespace game::server::systems {

CollisionSystem::CollisionSystem(const std::vector<sf::FloatRect>& colliders)
    : colliders(colliders) {
}

void CollisionSystem::update(float deltaTime, game::core::World& world) {
    // Get all entities with Position, Velocity, and Sprite components
    auto entities = world.getEntitiesWith<
        game::core::components::PositionComponent,
        game::core::components::VelocityComponent,
        game::core::components::SpriteComponent
    >();
    
    // Check collision for each entity
    for (game::core::Entity::ID entityID : entities) {
        checkAndResolveCollision(entityID, world, deltaTime);
    }
}

bool CollisionSystem::checkAndResolveCollision(game::core::Entity::ID entityID, game::core::World& world, float deltaTime) {
    auto* posComp = world.getComponent<game::core::components::PositionComponent>(entityID);
    auto* velComp = world.getComponent<game::core::components::VelocityComponent>(entityID);
    auto* spriteComp = world.getComponent<game::core::components::SpriteComponent>(entityID);
    
    if (!posComp || !velComp || !spriteComp) {
        return false;
    }
    
    // Calculate next position based on velocity
    sf::Vector2f nextPosition = posComp->position + velComp->velocity * deltaTime;
    
    // Check if would collide at next position
    if (CollisionHelper::wouldCollideAt(nextPosition, spriteComp->size, colliders)) {
        // Collision detected - stop movement
        velComp->velocity.x = 0.0f;
        velComp->velocity.y = 0.0f;
        return true;  // Collision resolved
    }
    
    return false;  // No collision
}

} // namespace game::server::systems

