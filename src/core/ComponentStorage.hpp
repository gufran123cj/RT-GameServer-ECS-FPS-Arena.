#pragma once

#include <vector>
#include <cassert>
#include <algorithm>
#include "Entity.hpp"
#include "Component.hpp"

namespace game::core {

/**
 * Component Storage using SparseSet data structure
 * 
 * SparseSet provides:
 * - O(1) lookup (EntityID → Component)
 * - O(1) insertion/deletion
 * - Cache-friendly (dense array for components)
 * - Memory efficient (sparse array only where needed)
 * 
 * Structure:
 * - dense: Array of actual components (contiguous, cache-friendly)
 * - sparse: Array mapping EntityID → dense index
 * - reverse: Array mapping dense index → EntityID
 */
template<typename T, ComponentEnableIf<T> = 0>
class ComponentStorage {
public:
    using ComponentType = T;
    using EntityID = Entity::ID;
    
    ComponentStorage() = default;
    ~ComponentStorage() = default;
    
    // Non-copyable (components are owned)
    ComponentStorage(const ComponentStorage&) = delete;
    ComponentStorage& operator=(const ComponentStorage&) = delete;
    
    // Movable
    ComponentStorage(ComponentStorage&&) noexcept = default;
    ComponentStorage& operator=(ComponentStorage&&) noexcept = default;
    
    /**
     * Add component to entity
     * Returns pointer to the component
     */
    T* add(EntityID entity, const T& component = T{}) {
        assert(entity != INVALID_ENTITY);
        
        // Check if entity already has this component
        if (has(entity)) {
            return get(entity);
        }
        
        // Resize sparse array if needed
        if (entity >= sparse.size()) {
            sparse.resize(entity + 1, INVALID_INDEX);
        }
        
        // Add component to dense array
        size_t denseIndex = dense.size();
        dense.push_back(component);
        reverse.push_back(entity);
        sparse[entity] = denseIndex;
        
        return &dense[denseIndex];
    }
    
    /**
     * Remove component from entity
     */
    void remove(EntityID entity) {
        if (!has(entity)) return;
        
        size_t denseIndex = sparse[entity];
        size_t lastDenseIndex = dense.size() - 1;
        
        // Swap with last element (for O(1) removal)
        if (denseIndex != lastDenseIndex) {
            dense[denseIndex] = std::move(dense[lastDenseIndex]);
            EntityID lastEntity = reverse[lastDenseIndex];
            reverse[denseIndex] = lastEntity;
            sparse[lastEntity] = denseIndex;
        }
        
        // Remove last element
        dense.pop_back();
        reverse.pop_back();
        sparse[entity] = INVALID_INDEX;
    }
    
    /**
     * Get component for entity (non-const)
     * Returns nullptr if entity doesn't have this component
     */
    T* get(EntityID entity) {
        if (entity >= sparse.size()) return nullptr;
        
        size_t denseIndex = sparse[entity];
        if (denseIndex == INVALID_INDEX) return nullptr;
        
        return &dense[denseIndex];
    }
    
    /**
     * Get component for entity (const)
     */
    const T* get(EntityID entity) const {
        if (entity >= sparse.size()) return nullptr;
        
        size_t denseIndex = sparse[entity];
        if (denseIndex == INVALID_INDEX) return nullptr;
        
        return &dense[denseIndex];
    }
    
    /**
     * Check if entity has this component
     */
    bool has(EntityID entity) const {
        if (entity >= sparse.size()) return false;
        return sparse[entity] != INVALID_INDEX;
    }
    
    /**
     * Get number of components stored
     */
    size_t size() const {
        return dense.size();
    }
    
    /**
     * Check if storage is empty
     */
    bool empty() const {
        return dense.empty();
    }
    
    /**
     * Clear all components
     */
    void clear() {
        dense.clear();
        reverse.clear();
        sparse.clear();
    }
    
    /**
     * Iterator support for range-based for loops
     */
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<EntityID, T*>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        
        ComponentStorage* storage;
        size_t index;
        
        Iterator(ComponentStorage* s, size_t i) : storage(s), index(i) {}
        
        value_type operator*() {
            EntityID entity = storage->reverse[index];
            return {entity, &storage->dense[index]};
        }
        
        Iterator& operator++() {
            ++index;
            return *this;
        }
        
        bool operator==(const Iterator& other) const {
            return storage == other.storage && index == other.index;
        }
        
        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };
    
    Iterator begin() {
        return Iterator(this, 0);
    }
    
    Iterator end() {
        return Iterator(this, dense.size());
    }
    
    /**
     * Const iterator support
     */
    struct ConstIterator {
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<EntityID, const T*>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        
        const ComponentStorage* storage;
        size_t index;
        
        ConstIterator(const ComponentStorage* s, size_t i) : storage(s), index(i) {}
        
        value_type operator*() {
            EntityID entity = storage->reverse[index];
            return {entity, &storage->dense[index]};
        }
        
        ConstIterator& operator++() {
            ++index;
            return *this;
        }
        
        bool operator==(const ConstIterator& other) const {
            return storage == other.storage && index == other.index;
        }
        
        bool operator!=(const ConstIterator& other) const {
            return !(*this == other);
        }
    };
    
    ConstIterator begin() const {
        return ConstIterator(this, 0);
    }
    
    ConstIterator end() const {
        return ConstIterator(this, dense.size());
    }
    
private:
    static constexpr size_t INVALID_INDEX = std::numeric_limits<size_t>::max();
    
    std::vector<T> dense;              // Actual components (contiguous, cache-friendly)
    std::vector<size_t> sparse;        // EntityID → dense index mapping
    std::vector<EntityID> reverse;     // dense index → EntityID mapping
};

} // namespace game::core

