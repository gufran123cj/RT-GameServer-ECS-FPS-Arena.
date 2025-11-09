#pragma once

#include "../ecs/Component.hpp"
#include "../physics/Physics.hpp"
#include "../net/Packet.hpp"
#include <cstddef>

namespace game::components {

// Collision component - stores AABB bounds for collision detection
class CollisionComponent : public ecs::ComponentBase<CollisionComponent> {
public:
    physics::AABB bounds;
    bool isStatic;  // Static objects don't move (walls, obstacles)
    bool isTrigger; // Trigger volumes (don't block movement, just detect)

    CollisionComponent() : isStatic(false), isTrigger(false) {
        bounds = physics::AABB(physics::Vec3(-0.5f, -0.5f, -0.5f), physics::Vec3(0.5f, 0.5f, 0.5f));
    }
    
    CollisionComponent(const physics::AABB& aabb, bool isStatic = false, bool isTrigger = false)
        : bounds(aabb), isStatic(isStatic), isTrigger(isTrigger) {}

    // Create AABB from center and size
    static CollisionComponent fromCenterSize(const physics::Vec3& center, const physics::Vec3& size, 
                                             bool isStatic = false, bool isTrigger = false) {
        physics::Vec3 halfSize = size * 0.5f;
        physics::AABB aabb(center - halfSize, center + halfSize);
        return CollisionComponent(aabb, isStatic, isTrigger);
    }

    std::unique_ptr<ecs::Component> clone() const override {
        return std::make_unique<CollisionComponent>(bounds, isStatic, isTrigger);
    }
    
    // Serialization (for network sync if needed)
    bool serialize(net::PacketWriter& writer) const override {
        return writer.write(bounds.min.x) && writer.write(bounds.min.y) && writer.write(bounds.min.z) &&
               writer.write(bounds.max.x) && writer.write(bounds.max.y) && writer.write(bounds.max.z) &&
               writer.write(isStatic) && writer.write(isTrigger);
    }
    
    bool deserialize(net::PacketReader& reader) override {
        return reader.read(bounds.min.x) && reader.read(bounds.min.y) && reader.read(bounds.min.z) &&
               reader.read(bounds.max.x) && reader.read(bounds.max.y) && reader.read(bounds.max.z) &&
               reader.read(isStatic) && reader.read(isTrigger);
    }
    
    size_t getSerializedSize() const override {
        return sizeof(float) * 6 + sizeof(bool) * 2; // 6 floats (min/max) + 2 bools
    }
};

} // namespace game::components

