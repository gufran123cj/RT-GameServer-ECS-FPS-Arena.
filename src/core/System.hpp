#pragma once

#include <cstdint>

namespace game::core {

class World;

/**
 * System Base Class
 * 
 * Systems contain game logic that operates on entities with specific components.
 * Systems are updated each frame in a specific order.
 * 
 * Example: MovementSystem updates PositionComponent based on VelocityComponent
 */
class System {
public:
    System() = default;
    virtual ~System() = default;
    
    // Non-copyable
    System(const System&) = delete;
    System& operator=(const System&) = delete;
    
    // Movable
    System(System&&) noexcept = default;
    System& operator=(System&&) noexcept = default;
    
    /**
     * Update system (called each frame)
     * @param deltaTime Time since last update in seconds
     * @param world Reference to the ECS world
     */
    virtual void update(float deltaTime, World& world) = 0;
    
    /**
     * Initialize system (called once after registration)
     * @param world Reference to the ECS world
     */
    virtual void initialize(World& world) {}
    
    /**
     * Shutdown system (called once before destruction)
     * @param world Reference to the ECS world
     */
    virtual void shutdown(World& world) {}
    
    /**
     * Get system priority (lower = earlier execution)
     * Default: 0 (no priority)
     */
    virtual int getPriority() const {
        return 0;
    }
    
    /**
     * Check if system is enabled
     */
    bool isEnabled() const {
        return enabled;
    }
    
    /**
     * Enable/disable system
     */
    void setEnabled(bool value) {
        enabled = value;
    }
    
private:
    bool enabled = true;
};

} // namespace game::core

