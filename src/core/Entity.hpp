#pragma once

#include <cstdint>
#include <limits>
#include <vector>
#include <unordered_map>
#include "../../include/common/types.hpp"

namespace game::core {

/**
 * Entity ID structure with generation counter for reuse detection
 * 
 * Entity ID consists of:
 * - id: The actual entity ID (0-based index)
 * - generation: Generation counter to detect ID reuse
 * 
 * This prevents issues when an entity is destroyed and its ID is reused,
 * but network packets still reference the old entity.
 */
struct Entity {
    using ID = game::EntityID;
    using Generation = uint32_t;
    
    ID id;
    Generation generation;
    
    Entity() : id(INVALID_ENTITY), generation(0) {}
    Entity(ID id, Generation gen) : id(id), generation(gen) {}
    
    bool isValid() const {
        return id != INVALID_ENTITY;
    }
    
    bool operator==(const Entity& other) const {
        return id == other.id && generation == other.generation;
    }
    
    bool operator!=(const Entity& other) const {
        return !(*this == other);
    }
    
    // Hash support for use in containers
    struct Hash {
        std::size_t operator()(const Entity& e) const {
            return std::hash<ID>{}(e.id) ^ (std::hash<Generation>{}(e.generation) << 1);
        }
    };
};

/**
 * Entity ID Generator
 * 
 * Generates unique entity IDs with generation counters.
 * When an entity is destroyed, its generation is incremented.
 * This allows detection of stale entity references.
 */
class EntityIDGenerator {
public:
    EntityIDGenerator() : nextID(0) {}
    
    /**
     * Generate a new entity ID
     */
    Entity create() {
        if (freeIDs.empty()) {
            // No free IDs, create new one
            EntityID id = nextID++;
            return Entity(id, 0);
        } else {
            // Reuse a free ID with incremented generation
            EntityID id = freeIDs.back();
            freeIDs.pop_back();
            Generation gen = generations[id] + 1;
            generations[id] = gen;
            return Entity(id, gen);
        }
    }
    
    /**
     * Destroy an entity (mark ID as free for reuse)
     */
    void destroy(const Entity& entity) {
        if (!entity.isValid()) return;
        
        EntityID id = entity.id;
        if (id < nextID) {
            freeIDs.push_back(id);
            // Generation is already incremented in create()
        }
    }
    
    /**
     * Check if an entity is valid (not destroyed or reused)
     */
    bool isValid(const Entity& entity) const {
        if (!entity.isValid()) return false;
        
        EntityID id = entity.id;
        if (id >= nextID) return false;
        
        // Check if generation matches
        auto it = generations.find(id);
        if (it == generations.end()) {
            // First generation
            return entity.generation == 0;
        }
        return entity.generation == it->second;
    }
    
    /**
     * Reset generator (for testing/cleanup)
     */
    void reset() {
        nextID = 0;
        freeIDs.clear();
        generations.clear();
    }
    
private:
    using EntityID = Entity::ID;
    using Generation = Entity::Generation;
    
    EntityID nextID;
    std::vector<EntityID> freeIDs;  // IDs available for reuse
    std::unordered_map<EntityID, Generation> generations;  // Current generation for each ID
};

} // namespace game::core

