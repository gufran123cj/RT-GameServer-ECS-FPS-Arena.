#pragma once

#include <cstdint>
#include <type_traits>
#include "../../include/common/types.hpp"
#include "Entity.hpp"

namespace game::core {

/**
 * Component Type Trait (C++17 compatible)
 * 
 * All components must be:
 * - Copyable (for efficient copying)
 * - Default constructible (for component storage)
 * - Destructible (for cleanup)
 * 
 * Note: We allow non-trivial types (like SFML Vector2f) for compatibility,
 * but prefer trivial types for better performance.
 */
template<typename T>
constexpr bool is_component_v = 
    std::is_copy_constructible_v<T> && 
    std::is_default_constructible_v<T> &&
    std::is_destructible_v<T>;

// SFINAE helper for template constraints
template<typename T>
using ComponentEnableIf = std::enable_if_t<is_component_v<T>, int>;

/**
 * Component Type ID Generator
 * 
 * Generates unique type IDs for each component type at compile-time.
 * This allows type-safe component access without runtime type information.
 * 
 * For network serialization, fixed type IDs are defined in types.hpp
 * (ComponentType::Position, ComponentType::Velocity, etc.)
 */
template<typename T>
struct ComponentTypeID {
    static_assert(is_component_v<T>, "T must be a Component");
    
    static ComponentTypeID get() {
        static ComponentTypeID id = nextID++;
        return id;
    }
    
    operator game::ComponentTypeID() const {
        return value;
    }
    
    game::ComponentTypeID value;
    
private:
    static inline game::ComponentTypeID nextID = 0;
};

/**
 * Component Tag
 * 
 * Base class for all components (optional, for type erasure if needed)
 * Components don't need to inherit from this, but it can be useful
 * for generic component handling.
 */
struct ComponentBase {
    virtual ~ComponentBase() = default;
};

/**
 * Component Metadata
 * 
 * Stores metadata about a component type (size, alignment, type ID)
 * Used for type-erased component storage and serialization.
 */
struct ComponentMetadata {
    game::ComponentTypeID typeID;
    size_t size;
    size_t alignment;
    const char* name;
    
    ComponentMetadata(game::ComponentTypeID id, size_t s, size_t a, const char* n)
        : typeID(id), size(s), alignment(a), name(n) {}
};

} // namespace game::core

