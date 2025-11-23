#pragma once

#include <unordered_map>
#include <memory>
#include <typeindex>
#include "Entity.hpp"
#include "Component.hpp"
#include "ComponentStorage.hpp"

namespace game::core {

/**
 * Type-erased component storage interface
 * Allows storing different component types in a single container
 */
class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;
    virtual void remove(Entity::ID entity) = 0;
    virtual bool has(Entity::ID entity) const = 0;
    virtual size_t size() const = 0;
    virtual void clear() = 0;
};

/**
 * Type-erased wrapper for ComponentStorage<T>
 */
template<typename T, ComponentEnableIf<T> = 0>
class ComponentStorageWrapper : public IComponentStorage {
public:
    ComponentStorageWrapper() = default;
    
    void remove(Entity::ID entity) override {
        storage.remove(entity);
    }
    
    bool has(Entity::ID entity) const override {
        return storage.has(entity);
    }
    
    size_t size() const override {
        return storage.size();
    }
    
    void clear() override {
        storage.clear();
    }
    
    ComponentStorage<T>& getStorage() {
        return storage;
    }
    
    const ComponentStorage<T>& getStorage() const {
        return storage;
    }
    
private:
    ComponentStorage<T> storage;
};

/**
 * Component Registry
 * 
 * Manages component storage for all component types.
 * Provides type-safe access to component storage.
 * 
 * Uses type erasure to store different component types in a single registry.
 */
class ComponentRegistry {
public:
    ComponentRegistry() = default;
    ~ComponentRegistry() = default;
    
    // Non-copyable
    ComponentRegistry(const ComponentRegistry&) = delete;
    ComponentRegistry& operator=(const ComponentRegistry&) = delete;
    
    // Movable
    ComponentRegistry(ComponentRegistry&&) noexcept = default;
    ComponentRegistry& operator=(ComponentRegistry&&) noexcept = default;
    
    /**
     * Get or create storage for component type T
     * Returns reference to ComponentStorage<T>
     */
    template<typename T, ComponentEnableIf<T> = 0>
    ComponentStorage<T>& getStorage() {
        auto typeIndex = std::type_index(typeid(T));
        
        auto it = storages.find(typeIndex);
        if (it == storages.end()) {
            // Create new storage wrapper for this component type
            auto wrapper = std::make_unique<ComponentStorageWrapper<T>>();
            ComponentStorageWrapper<T>* wrapperPtr = wrapper.get();
            storages[typeIndex] = std::move(wrapper);
            return wrapperPtr->getStorage();
        }
        
        // Cast to ComponentStorageWrapper<T> and get storage
        auto* wrapper = static_cast<ComponentStorageWrapper<T>*>(it->second.get());
        return wrapper->getStorage();
    }
    
    /**
     * Get storage for component type T (const)
     */
    template<typename T, ComponentEnableIf<T> = 0>
    const ComponentStorage<T>& getStorage() const {
        auto typeIndex = std::type_index(typeid(T));
        
        auto it = storages.find(typeIndex);
        if (it == storages.end()) {
            // Return empty storage (shouldn't happen in practice)
            static const ComponentStorage<T> empty;
            return empty;
        }
        
        // Cast to ComponentStorageWrapper<T> and get storage
        auto* wrapper = static_cast<const ComponentStorageWrapper<T>*>(it->second.get());
        return wrapper->getStorage();
    }
    
    /**
     * Add component to entity
     */
    template<typename T, ComponentEnableIf<T> = 0>
    T* add(Entity::ID entity, const T& component = T{}) {
        return getStorage<T>().add(entity, component);
    }
    
    /**
     * Remove component from entity
     */
    template<typename T, ComponentEnableIf<T> = 0>
    void remove(Entity::ID entity) {
        if (hasStorage<T>()) {
            getStorage<T>().remove(entity);
        }
    }
    
    /**
     * Get component for entity
     */
    template<typename T, ComponentEnableIf<T> = 0>
    T* get(Entity::ID entity) {
        if (!hasStorage<T>()) return nullptr;
        return getStorage<T>().get(entity);
    }
    
    /**
     * Get component for entity (const)
     */
    template<typename T, ComponentEnableIf<T> = 0>
    const T* get(Entity::ID entity) const {
        if (!hasStorage<T>()) return nullptr;
        return getStorage<T>().get(entity);
    }
    
    /**
     * Check if entity has component
     */
    template<typename T, ComponentEnableIf<T> = 0>
    bool has(Entity::ID entity) const {
        if (!hasStorage<T>()) return false;
        return getStorage<T>().has(entity);
    }
    
    /**
     * Check if storage exists for component type
     */
    template<typename T, ComponentEnableIf<T> = 0>
    bool hasStorage() const {
        auto typeIndex = std::type_index(typeid(T));
        return storages.find(typeIndex) != storages.end();
    }
    
    /**
     * Remove all components for an entity
     */
    void removeAll(Entity::ID entity) {
        for (auto& [typeIndex, storage] : storages) {
            storage->remove(entity);
        }
    }
    
    /**
     * Clear all component storages
     */
    void clear() {
        storages.clear();
    }
    
    /**
     * Get number of component types registered
     */
    size_t getTypeCount() const {
        return storages.size();
    }
    
private:
    // Type-erased storage map
    // Key: std::type_index (component type)
    // Value: std::unique_ptr to ComponentStorage<T> (type-erased)
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> storages;
};

} // namespace game::core

