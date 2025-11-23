#include "World.hpp"

namespace game::core {

World::World() = default;
World::~World() {
    shutdown();
}

} // namespace game::core

