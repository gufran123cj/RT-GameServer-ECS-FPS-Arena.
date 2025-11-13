#include "AssetManager.hpp"
#include <iostream>
#include <fstream>
#if __cplusplus >= 201703L && defined(__has_include)
    #if __has_include(<filesystem>)
        #include <filesystem>
        namespace fs = std::filesystem;
    #else
        #include <experimental/filesystem>
        namespace fs = std::experimental::filesystem;
    #endif
#else
    // Fallback: use stat or other method
    #include <sys/stat.h>
    namespace fs {
        inline bool exists(const std::string& path) {
            struct stat buffer;
            return (stat(path.c_str(), &buffer) == 0);
        }
    }
#endif

namespace assets {

AssetManager::AssetManager(const std::string& basePath) : basePath(basePath) {
    // Ensure base path ends with separator
    if (!basePath.empty() && basePath.back() != '/' && basePath.back() != '\\') {
        this->basePath += "/";
    }
}

AssetManager::~AssetManager() {
    unloadAll();
}

std::string AssetManager::resolvePath(const std::string& path) const {
    // If path is absolute, return as-is
    if (path.length() > 0 && (path[0] == '/' || (path.length() > 1 && path[1] == ':'))) {
        return path;
    }
    // Otherwise, prepend base path
    return basePath + path;
}

Texture2D* AssetManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return &it->second;
    }
    
    // Try to load texture
    std::string path = resolvePath(name);
    if (loadTexture(name, path)) {
        return &textures[name];
    }
    
    return nullptr;
}

bool AssetManager::loadTexture(const std::string& name, const std::string& path) {
    // Check if already loaded
    if (textures.find(name) != textures.end()) {
        return true; // Already loaded
    }
    
    std::string fullPath = resolvePath(path);
    
    // Check if file exists
    if (!fs::exists(fullPath)) {
        std::cerr << "[AssetManager] Texture file not found: " << fullPath << std::endl;
        return false;
    }
    
    // Load texture using Raylib
    Texture2D texture = LoadTexture(fullPath.c_str());
    if (texture.id == 0) {
        std::cerr << "[AssetManager] Failed to load texture: " << fullPath << std::endl;
        return false;
    }
    
    textures[name] = texture;
    std::cout << "[AssetManager] Loaded texture: " << name << " from " << fullPath << std::endl;
    return true;
}

void AssetManager::unloadTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        UnloadTexture(it->second);
        textures.erase(it);
    }
}

void AssetManager::unloadAll() {
    // Unload all textures
    for (auto& [name, texture] : textures) {
        UnloadTexture(texture);
    }
    textures.clear();
    
    // Unload all tileset textures
    for (auto& [uid, texture] : tilesetTextures) {
        UnloadTexture(texture);
    }
    tilesetTextures.clear();
}

bool AssetManager::loadLDtkWorld(const std::string& jsonPath) {
    std::string fullPath = resolvePath(jsonPath);
    
    ldtkWorld = std::make_unique<ldtk::World>();
    if (!ldtk::LDtkParser::loadWorld(fullPath, *ldtkWorld)) {
        std::cerr << "[AssetManager] Failed to load LDtk world: " << fullPath << std::endl;
        ldtkWorld.reset();
        return false;
    }
    
    std::cout << "[AssetManager] Loaded LDtk world: " << fullPath << std::endl;
    std::cout << "[AssetManager]   Levels: " << ldtkWorld->levels.size() << std::endl;
    std::cout << "[AssetManager]   Tilesets: " << ldtkWorld->tilesets.size() << std::endl;
    
    // Auto-load tileset textures
    for (const auto& [uid, tileset] : ldtkWorld->tilesets) {
        // Fix tileset path: LDtk sometimes uses absolute paths, extract filename
        std::string tilesetPath = tileset.relPath;
        
        // If path contains "atlas/", extract the filename and use our atlas folder
        size_t atlasPos = tilesetPath.find("atlas/");
        if (atlasPos != std::string::npos) {
            // Extract filename after "atlas/"
            std::string filename = tilesetPath.substr(atlasPos + 6); // "atlas/" is 6 chars
            tilesetPath = "atlas/" + filename;
        } else {
            // If it's just a filename, assume it's in atlas folder
            size_t lastSlash = tilesetPath.find_last_of("/\\");
            if (lastSlash != std::string::npos) {
                tilesetPath = "atlas/" + tilesetPath.substr(lastSlash + 1);
            } else {
                tilesetPath = "atlas/" + tilesetPath;
            }
        }
        
        std::cout << "[AssetManager] Loading tileset UID " << uid 
                  << " from: " << tilesetPath << " (original: " << tileset.relPath << ")" << std::endl;
        loadTilesetTexture(uid, tilesetPath);
    }
    
    return true;
}

ldtk::Level* AssetManager::getLevel(const std::string& identifier) {
    if (!ldtkWorld) {
        return nullptr;
    }
    return ldtk::LDtkParser::getLevelByIdentifier(*ldtkWorld, identifier);
}

Texture2D* AssetManager::getTilesetTexture(int tilesetUid) {
    auto it = tilesetTextures.find(tilesetUid);
    if (it != tilesetTextures.end()) {
        return &it->second;
    }
    return nullptr;
}

bool AssetManager::loadTilesetTexture(int tilesetUid, const std::string& tilesetPath) {
    // Check if already loaded
    if (tilesetTextures.find(tilesetUid) != tilesetTextures.end()) {
        return true; // Already loaded
    }
    
    std::string fullPath = resolvePath(tilesetPath);
    
    // Check if file exists
    if (!fs::exists(fullPath)) {
        std::cerr << "[AssetManager] Tileset file not found: " << fullPath << std::endl;
        return false;
    }
    
    // Load texture using Raylib
    Texture2D texture = LoadTexture(fullPath.c_str());
    if (texture.id == 0) {
        std::cerr << "[AssetManager] Failed to load tileset: " << fullPath << std::endl;
        return false;
    }
    
    tilesetTextures[tilesetUid] = texture;
    std::cout << "[AssetManager] Loaded tileset (UID: " << tilesetUid << "): " << fullPath << std::endl;
    return true;
}

bool AssetManager::textureExists(const std::string& name) const {
    return textures.find(name) != textures.end();
}

void AssetManager::printStatistics() const {
    std::cout << "[AssetManager] Statistics:" << std::endl;
    std::cout << "  Loaded textures: " << textures.size() << std::endl;
    std::cout << "  Loaded tilesets: " << tilesetTextures.size() << std::endl;
    if (ldtkWorld) {
        std::cout << "  LDtk world loaded: Yes" << std::endl;
        std::cout << "  Levels: " << ldtkWorld->levels.size() << std::endl;
    } else {
        std::cout << "  LDtk world loaded: No" << std::endl;
    }
}

} // namespace assets

