#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "System.hpp"

namespace game::core {

class World;

/**
 * System Manager
 * 
 * Manages all systems and their execution order.
 * Systems are updated in priority order (lower priority = earlier execution).
 */
class SystemManager {
public:
    SystemManager() = default;
    ~SystemManager() = default;
    
    // Non-copyable
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;
    
    // Movable
    SystemManager(SystemManager&&) noexcept = default;
    SystemManager& operator=(SystemManager&&) noexcept = default;
    
    /**
     * Register a system
     * System will be initialized and updated each frame
     * @param system Unique pointer to system (ownership transferred)
     */
    void registerSystem(std::unique_ptr<System> system) {
        if (!system) return;
        
        systems.push_back(std::move(system));
        sortSystems();
    }
    
    /**
     * Update all systems
     * Systems are updated in priority order
     * @param deltaTime Time since last update in seconds
     * @param world Reference to the ECS world
     */
    void update(float deltaTime, World& world) {
        for (auto& system : systems) {
            if (system->isEnabled()) {
                system->update(deltaTime, world);
            }
        }
    }
    
    /**
     * Initialize all systems
     * Called once after all systems are registered
     * @param world Reference to the ECS world
     */
    void initialize(World& world) {
        for (auto& system : systems) {
            system->initialize(world);
        }
    }
    
    /**
     * Shutdown all systems
     * Called once before destruction
     * @param world Reference to the ECS world
     */
    void shutdown(World& world) {
        for (auto& system : systems) {
            system->shutdown(world);
        }
    }
    
    /**
     * Get number of registered systems
     */
    size_t getSystemCount() const {
        return systems.size();
    }
    
    /**
     * Clear all systems
     */
    void clear() {
        systems.clear();
    }
    
private:
    /**
     * Sort systems by priority (lower priority = earlier execution)
     */
    void sortSystems() {
        std::sort(systems.begin(), systems.end(),
            [](const std::unique_ptr<System>& a, const std::unique_ptr<System>& b) {
                return a->getPriority() < b->getPriority();
            }
        );
    }
    
    std::vector<std::unique_ptr<System>> systems;
};

} // namespace game::core

