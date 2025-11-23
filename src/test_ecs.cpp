#include <iostream>
#include "core/World.hpp"
#include "core/components/PositionComponent.hpp"
#include "core/components/VelocityComponent.hpp"
#include "core/components/SpriteComponent.hpp"
#include "core/systems/MovementSystem.hpp"

using namespace game::core;
using namespace game::core::components;
using namespace game::core::systems;

int main() {
    std::cout << "=== ECS Core Test ===" << std::endl;
    
    // World oluştur
    World world;
    
    // Entity oluştur
    Entity player = world.createEntity();
    std::cout << "Created entity: ID=" << player.id << ", Generation=" << player.generation << std::endl;
    
    // Component'ler ekle
    auto* pos = world.addComponent<PositionComponent>(player.id, {10.0f, 20.0f});
    auto* vel = world.addComponent<VelocityComponent>(player.id, {1.5f, 0.5f});
    auto* sprite = world.addComponent<SpriteComponent>(player.id);
    
    std::cout << "Added components:" << std::endl;
    std::cout << "  Position: (" << pos->position.x << ", " << pos->position.y << ")" << std::endl;
    std::cout << "  Velocity: (" << vel->velocity.x << ", " << vel->velocity.y << ")" << std::endl;
    std::cout << "  Sprite: size=(" << sprite->size.x << ", " << sprite->size.y << ")" << std::endl;
    
    // MovementSystem ekle
    world.registerSystem(std::make_unique<MovementSystem>());
    world.initialize();
    
    std::cout << "\n=== Simulating 5 frames (60 FPS) ===" << std::endl;
    
    // 5 frame simüle et (60 FPS = 0.016s per frame)
    const float deltaTime = 1.0f / 60.0f;
    for (int i = 0; i < 5; ++i) {
        world.update(deltaTime);
        
        // Position'ı kontrol et
        const auto* updatedPos = world.getComponent<PositionComponent>(player.id);
        std::cout << "Frame " << (i + 1) << ": Position=(" 
                  << updatedPos->position.x << ", " << updatedPos->position.y << ")" << std::endl;
    }
    
    // Entity query test
    std::cout << "\n=== Entity Query Test ===" << std::endl;
    auto entities = world.getEntitiesWith<PositionComponent, VelocityComponent>();
    std::cout << "Found " << entities.size() << " entities with Position + Velocity" << std::endl;
    
    // Component erişim testi
    std::cout << "\n=== Component Access Test ===" << std::endl;
    bool hasPos = world.hasComponent<PositionComponent>(player.id);
    bool hasVel = world.hasComponent<VelocityComponent>(player.id);
    bool hasSprite = world.hasComponent<SpriteComponent>(player.id);
    
    std::cout << "Has PositionComponent: " << (hasPos ? "YES" : "NO") << std::endl;
    std::cout << "Has VelocityComponent: " << (hasVel ? "YES" : "NO") << std::endl;
    std::cout << "Has SpriteComponent: " << (hasSprite ? "YES" : "NO") << std::endl;
    
    // Entity destroy test
    std::cout << "\n=== Entity Destroy Test ===" << std::endl;
    world.destroyEntity(player);
    bool isValid = world.isValidEntity(player);
    std::cout << "Entity valid after destroy: " << (isValid ? "YES" : "NO") << std::endl;
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    return 0;
}

