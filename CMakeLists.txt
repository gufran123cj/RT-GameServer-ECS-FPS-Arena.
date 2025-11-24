cmake_minimum_required(VERSION 3.12)
project(LDtkSFMLGame)

# Set C++17 standard (required for ECS Core)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for clangd and other tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(
    LDtkLoader
    GIT_REPOSITORY https://github.com/Madour/LDtkLoader
    GIT_TAG 1.5.3.1
)
FetchContent_MakeAvailable(LDtkLoader)

# SFML path configuration
# Add SFML installation directory to CMAKE_PREFIX_PATH
# This allows CMake to find SFMLConfig.cmake in lib/cmake/SFML/
if(NOT DEFINED CMAKE_PREFIX_PATH OR NOT "D:/SFML-2.6.0" IN_LIST CMAKE_PREFIX_PATH)
    list(APPEND CMAKE_PREFIX_PATH "D:/SFML-2.6.0")
endif()

# Alternative: Set SFML_DIR directly if SFMLConfig.cmake exists
# SFML 2.6.0 typically has config files in lib/cmake/SFML/
if(EXISTS "D:/SFML-2.6.0/lib/cmake/SFML/SFMLConfig.cmake")
    set(SFML_DIR "D:/SFML-2.6.0/lib/cmake/SFML" CACHE PATH "SFML CMake config directory")
elseif(EXISTS "D:/SFML-2.6.0/cmake/Modules/FindSFML.cmake")
    # Fallback: Use FindSFML.cmake module for older SFML installations
    list(APPEND CMAKE_MODULE_PATH "D:/SFML-2.6.0/cmake/Modules")
endif()

# set(SFML_STATIC_LIBRARIES TRUE)
find_package(SFML COMPONENTS graphics REQUIRED)

# ECS Core source files
set(ECS_CORE_SOURCES
    src/core/World.cpp
)

# ECS Core header files (header-only, but listed for IDE support)
set(ECS_CORE_HEADERS
    src/core/Entity.hpp
    src/core/Component.hpp
    src/core/ComponentStorage.hpp
    src/core/ComponentRegistry.hpp
    src/core/System.hpp
    src/core/SystemManager.hpp
    src/core/World.hpp
    src/core/components/PositionComponent.hpp
    src/core/components/VelocityComponent.hpp
    src/core/components/SpriteComponent.hpp
    src/core/systems/MovementSystem.hpp
)

# Client executable (now with network support)
set(CLIENT_NETWORK_SOURCES
    src/client/ClientNetworkManager.cpp
)

# Game MVC sources
set(GAME_SOURCES
    src/game/PlayerCollision.cpp
    src/game/GameClient.cpp
    src/game/GameModel.cpp
    src/game/GameView.cpp
    src/game/GameController.cpp
)

add_executable(LDtkSFMLGame 
    src/main.cpp 
    src/TileMap.cpp
    ${ECS_CORE_SOURCES}
    ${CLIENT_NETWORK_SOURCES}
    ${GAME_SOURCES}
)
set_target_properties(LDtkSFMLGame PROPERTIES DEBUG_POSTFIX -d RUNTIME_OUTPUT_DIRECTORY bin)
target_link_libraries(LDtkSFMLGame PRIVATE LDtkLoader::LDtkLoader sfml-graphics sfml-network)

# ECS Test executable
add_executable(test_ecs 
    src/test_ecs.cpp
    ${ECS_CORE_SOURCES}
)
set_target_properties(test_ecs PROPERTIES DEBUG_POSTFIX -d RUNTIME_OUTPUT_DIRECTORY bin)
target_link_libraries(test_ecs PRIVATE sfml-graphics)

# Server executable
set(SERVER_SOURCES
    src/server/main.cpp
    src/server/GameServer.cpp
    src/server/ServerNetworkManager.cpp
)

add_executable(gameserver
    ${SERVER_SOURCES}
    ${ECS_CORE_SOURCES}
)
set_target_properties(gameserver PROPERTIES DEBUG_POSTFIX -d RUNTIME_OUTPUT_DIRECTORY bin)
target_link_libraries(gameserver PRIVATE sfml-graphics sfml-network)

# Client executable (test client)
set(CLIENT_SOURCES
    src/client/main.cpp
    src/client/ClientNetworkManager.cpp
)

add_executable(testclient
    ${CLIENT_SOURCES}
)
set_target_properties(testclient PROPERTIES DEBUG_POSTFIX -d RUNTIME_OUTPUT_DIRECTORY bin)
target_link_libraries(testclient PRIVATE sfml-network)

# SFML bin directory (where DLLs are located)
set(SFML_BIN_DIR "D:/SFML-2.6.0/bin")

# Copy SFML DLLs to output directory (Debug and Release versions)
# For LDtkSFMLGame (client - needs network DLL too)
add_custom_command(TARGET LDtkSFMLGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_BIN_DIR}/sfml-graphics-2.dll"
        "${SFML_BIN_DIR}/sfml-graphics-d-2.dll"
        "${SFML_BIN_DIR}/sfml-window-2.dll"
        "${SFML_BIN_DIR}/sfml-window-d-2.dll"
        "${SFML_BIN_DIR}/sfml-system-2.dll"
        "${SFML_BIN_DIR}/sfml-system-d-2.dll"
        "${SFML_BIN_DIR}/sfml-network-2.dll"
        "${SFML_BIN_DIR}/sfml-network-d-2.dll"
        $<TARGET_FILE_DIR:LDtkSFMLGame>
    COMMENT "Copying SFML DLLs to LDtkSFMLGame output directory"
)

# For gameserver (server - needs network DLL too)
add_custom_command(TARGET gameserver POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_BIN_DIR}/sfml-graphics-2.dll"
        "${SFML_BIN_DIR}/sfml-graphics-d-2.dll"
        "${SFML_BIN_DIR}/sfml-window-2.dll"
        "${SFML_BIN_DIR}/sfml-window-d-2.dll"
        "${SFML_BIN_DIR}/sfml-system-2.dll"
        "${SFML_BIN_DIR}/sfml-system-d-2.dll"
        "${SFML_BIN_DIR}/sfml-network-2.dll"
        "${SFML_BIN_DIR}/sfml-network-d-2.dll"
        $<TARGET_FILE_DIR:gameserver>
    COMMENT "Copying SFML DLLs to gameserver output directory"
)

# For testclient (client - needs network DLL)
add_custom_command(TARGET testclient POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_BIN_DIR}/sfml-system-2.dll"
        "${SFML_BIN_DIR}/sfml-system-d-2.dll"
        "${SFML_BIN_DIR}/sfml-network-2.dll"
        "${SFML_BIN_DIR}/sfml-network-d-2.dll"
        $<TARGET_FILE_DIR:testclient>
    COMMENT "Copying SFML DLLs to testclient output directory"
)

# Copy assets directory
add_custom_command(TARGET LDtkSFMLGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets/ $<TARGET_FILE_DIR:LDtkSFMLGame>/assets/
    COMMENT "Copying assets directory"
)
