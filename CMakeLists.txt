cmake_minimum_required(VERSION 3.12)
project(LDtkSFMLGame)

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

add_executable(LDtkSFMLGame src/main.cpp src/TileMap.cpp)
set_target_properties(LDtkSFMLGame PROPERTIES DEBUG_POSTFIX -d RUNTIME_OUTPUT_DIRECTORY bin)
target_link_libraries(LDtkSFMLGame PRIVATE LDtkLoader::LDtkLoader sfml-graphics)

# SFML bin directory (where DLLs are located)
set(SFML_BIN_DIR "D:/SFML-2.6.0/bin")

# Copy SFML DLLs to output directory (Debug and Release versions)
add_custom_command(TARGET LDtkSFMLGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_BIN_DIR}/sfml-graphics-2.dll"
        "${SFML_BIN_DIR}/sfml-graphics-d-2.dll"
        "${SFML_BIN_DIR}/sfml-window-2.dll"
        "${SFML_BIN_DIR}/sfml-window-d-2.dll"
        "${SFML_BIN_DIR}/sfml-system-2.dll"
        "${SFML_BIN_DIR}/sfml-system-d-2.dll"
        $<TARGET_FILE_DIR:LDtkSFMLGame>
    COMMENT "Copying SFML DLLs to output directory"
)

# Copy assets directory
add_custom_command(TARGET LDtkSFMLGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/assets/ $<TARGET_FILE_DIR:LDtkSFMLGame>/assets/
    COMMENT "Copying assets directory"
)
