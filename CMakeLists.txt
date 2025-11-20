cmake_minimum_required(VERSION 3.20)
project(GameServer VERSION 1.0.0 LANGUAGES CXX)

# Generate compile_commands.json for IDE IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-Wall -Wextra -O3 -march=native)
    add_compile_options(-D__USE_MINGW_ANSI_STDIO=1)
    add_compile_options(-fext-numeric-literals)
    add_compile_options(-fpermissive)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/ecs
    ${CMAKE_SOURCE_DIR}/net
    ${CMAKE_SOURCE_DIR}/physics
    ${CMAKE_SOURCE_DIR}/matchmaker
    ${CMAKE_SOURCE_DIR}/anti-cheat-lite
    ${CMAKE_SOURCE_DIR}/components
    ${CMAKE_SOURCE_DIR}/systems
    # TileMap.hpp src/ klasöründe
)

# LDtkLoader (SFML için)
include(FetchContent)
FetchContent_Declare(
    LDtkLoader
    GIT_REPOSITORY https://github.com/Madour/LDtkLoader
    GIT_TAG 1.5.3.1
)
FetchContent_MakeAvailable(LDtkLoader)

# SFML
# Windows için SFML path'i - D:\SFML-2.6.0 konumunda
set(SFML_DIR "D:/SFML-2.6.0/lib/cmake/SFML")
find_package(SFML 2.6 COMPONENTS graphics system window REQUIRED)

# SFML include ve lib path'lerini ekle
if(SFML_FOUND)
    # SFML_INCLUDE_DIR bazen boş olabilir, manuel olarak ayarlayalım
    if(NOT SFML_INCLUDE_DIR)
        get_filename_component(SFML_ROOT_DIR ${SFML_DIR} DIRECTORY)  # lib/cmake
        get_filename_component(SFML_ROOT_DIR ${SFML_ROOT_DIR} DIRECTORY)  # lib
        get_filename_component(SFML_ROOT_DIR ${SFML_ROOT_DIR} DIRECTORY)  # SFML-2.6.0
        set(SFML_INCLUDE_DIR "${SFML_ROOT_DIR}/include")
    endif()
    include_directories(${SFML_INCLUDE_DIR})
    if(SFML_LIBRARY_DIRS)
        link_directories(${SFML_LIBRARY_DIRS})
    endif()
    message(STATUS "SFML found: ${SFML_VERSION}")
    message(STATUS "SFML include dir: ${SFML_INCLUDE_DIR}")
    if(SFML_LIBRARY_DIRS)
        message(STATUS "SFML lib dir: ${SFML_LIBRARY_DIRS}")
    endif()
endif()

# Source files - Sadece SFML main.cpp için gerekli dosyalar
# GameClient.cpp, Server.cpp, MiniGameViewer.cpp, TestClient.cpp hariç (bunlar Raylib kullanıyor)
set(SOURCES
    "${CMAKE_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_SOURCE_DIR}/src/TileMap.cpp"
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Platform-specific libraries ve SFML/LDtkLoader link
target_link_libraries(${PROJECT_NAME} PRIVATE 
    LDtkLoader::LDtkLoader 
    sfml-graphics
    sfml-window
    sfml-system
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()

message(STATUS "SFML libraries linked: sfml-graphics, sfml-window, sfml-system")

# Assets klasörünü kopyala (eğer varsa)
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_CURRENT_LIST_DIR}/assets/ 
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets/
    )
endif()

# SFML DLL'lerini output dizinine kopyala (Windows için)
if(WIN32 AND SFML_FOUND)
    # SFML bin klasörünü bul (genellikle SFML-2.6.0/bin klasöründe)
    # SFML_DIR = D:/SFML-2.6.0/lib/cmake/SFML
    # Root = D:/SFML-2.6.0
    get_filename_component(SFML_ROOT_DIR ${SFML_DIR} DIRECTORY)  # lib/cmake
    get_filename_component(SFML_ROOT_DIR ${SFML_ROOT_DIR} DIRECTORY)  # lib
    get_filename_component(SFML_ROOT_DIR ${SFML_ROOT_DIR} DIRECTORY)  # SFML-2.6.0
    set(SFML_BIN_DIR "${SFML_ROOT_DIR}/bin")
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_BIN_DIR}/sfml-graphics-2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_BIN_DIR}/sfml-system-2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SFML_BIN_DIR}/sfml-window-2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        COMMENT "Copying SFML DLLs to output directory"
    )
    message(STATUS "SFML DLLs will be copied from: ${SFML_BIN_DIR}")
endif()

