#pragma once

#include <vector>
#include <memory>
#include <tuple>
#include <type_traits>
#include "Entity.hpp"
#include "Component.hpp"
#include "ComponentRegistry.hpp"
#include "SystemManager.hpp"

using game::core::ComponentEnableIf;
using game::core::is_component_v;

namespace game::core {

/**
 * World - ECS Container
 * 
 * The World class is the main entry point for the ECS system.
 * It manages:
 * - Entity creation and destruction
 * - Component storage and access
 * - System registration and execution
 * 
 * Usage:
 *   World world;
 *   Entity player = world.createEntity();
 *   world.addComponent<PositionComponent>(player.id, {10.0f, 20.0f});
 *   world.registerSystem<MovementSystem>();
 *   world.update(0.016f); // 60 FPS
 */
class World {
public:
    World();
    ~World();
    
    // Non-copyable
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    
    // Movable
    World(World&&) noexcept = default;
    World& operator=(World&&) noexcept = default;
    
    // ========== Entity Management ==========
    
    /**
     * Create a new entity
     * Returns Entity with unique ID and generation
     */
    Entity createEntity() {
        Entity entity = entityGenerator.create();
        return entity;
    }
    
    /**
     * Destroy an entity
     * Removes all components and marks ID as free for reuse
     */
    void destroyEntity(const Entity& entity) {
        if (!entity.isValid()) return;
        
        // Remove all components
        registry.removeAll(entity.id);
        
        // Mark entity ID as free
        entityGenerator.destroy(entity);
    }
    
    /**
     * Check if entity is valid (not destroyed or reused)
     */
    bool isValidEntity(const Entity& entity) const {
        return entityGenerator.isValid(entity);
    }
    
    // ========== Component Management ==========
    
    /**
     * Add component to entity
     * @param entity Entity ID
     * @param component Component data (default constructed if not provided)
     * @return Pointer to the component
     */
    template<typename T, ComponentEnableIf<T> = 0>
    T* addComponent(Entity::ID entity, const T& component = T{}) {
        return registry.add<T>(entity, component);
    }
    
    /**
     * Remove component from entity
     */
    template<typename T, ComponentEnableIf<T> = 0>
    void removeComponent(Entity::ID entity) {
        registry.remove<T>(entity);
    }
    
    /**
     * Get component for entity
     * @return Pointer to component, or nullptr if entity doesn't have it
     */
    template<typename T, ComponentEnableIf<T> = 0>
    T* getComponent(Entity::ID entity) {
        return registry.get<T>(entity);
    }
    
    /**
     * Get component for entity (const)
     */
    template<typename T, ComponentEnableIf<T> = 0>
    const T* getComponent(Entity::ID entity) const {
        return registry.get<T>(entity);
    }
    
    /**
     * Check if entity has component
     */
    template<typename T, ComponentEnableIf<T> = 0>
    bool hasComponent(Entity::ID entity) const {
        return registry.has<T>(entity);
    }
    
    /**
     * Get component storage (for systems that need direct access)
     */
    template<typename T, ComponentEnableIf<T> = 0>
    ComponentStorage<T>& getStorage() {
        return registry.getStorage<T>();
    }
    
    /**
     * Get component storage (const)
     */
    template<typename T, ComponentEnableIf<T> = 0>
    const ComponentStorage<T>& getStorage() const {
        return registry.getStorage<T>();
    }
    
    // ========== System Management ==========
    
    /**
     * Register a system
     * System will be initialized and updated each frame
     */
    void registerSystem(std::unique_ptr<System> system) {
        systemManager.registerSystem(std::move(system));
    }
    
    /**
     * Update all systems
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime) {
        systemManager.update(deltaTime, *this);
    }
    
    /**
     * Initialize all systems
     * Call this after all systems are registered
     */
    void initialize() {
        systemManager.initialize(*this);
    }
    
    /**
     * Shutdown all systems
     * Call this before destruction
     */
    void shutdown() {
        systemManager.shutdown(*this);
    }
    
    // ========== Query System ==========
    
    /**
     * Get all entities that have all specified components
     * This is a simple implementation - for better performance, use system queries
     */
    template<typename... Components>
    std::vector<Entity::ID> getEntitiesWith() const {
        static_assert((is_component_v<Components> && ...), "All types must be Components");
        std::vector<Entity::ID> result;
        
        // Get all entities from first component type
        if constexpr (sizeof...(Components) > 0) {
            using FirstComponent = std::tuple_element_t<0, std::tuple<Components...>>;
            const auto& storage = registry.getStorage<FirstComponent>();
            
            for (const auto& pair : storage) {
                Entity::ID entity = pair.first;
                
                // Check if entity has all other components
                bool hasAll = true;
                if constexpr (sizeof...(Components) > 1) {
                    hasAll = hasAllComponentsImpl<Components...>(entity);
                }
                
                if (hasAll) {
                    result.push_back(entity);
                }
            }
        }
        
        return result;
    }
    
    /**
     * Check if entity has all specified components
     */
    template<typename... Components>
    bool hasAllComponents(Entity::ID entity) const {
        static_assert((is_component_v<Components> && ...), "All types must be Components");
        return hasAllComponentsImpl<Components...>(entity);
    }
    
    // ========== Utility ==========
    
    /**
     * Clear all entities and components
     */
    void clear() {
        registry.clear();
        entityGenerator.reset();
    }
    
    /**
     * Get component registry (for advanced usage)
     */
    ComponentRegistry& getRegistry() {
        return registry;
    }
    
    const ComponentRegistry& getRegistry() const {
        return registry;
    }
    
private:
    EntityIDGenerator entityGenerator;
    ComponentRegistry registry;
    SystemManager systemManager;
    
    // Helper for hasAllComponents (fold expression)
    template<typename... Components>
    bool hasAllComponentsImpl(Entity::ID entity) const {
        return (registry.has<Components>(entity) && ...);
    }
};

} // namespace game::core

