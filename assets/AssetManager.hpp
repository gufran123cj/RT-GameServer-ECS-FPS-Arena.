#pragma once

#include "../ldtk/LDtkParser.hpp"
#include "../raylib/include/raylib.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace assets {

class AssetManager {
public:
    AssetManager(const std::string& basePath = "sprites/");
    ~AssetManager();
    
    // Texture operations
    Texture2D* getTexture(const std::string& name);
    bool loadTexture(const std::string& name, const std::string& path);
    void unloadTexture(const std::string& name);
    void unloadAll();
    
    // LDtk operations
    bool loadLDtkWorld(const std::string& jsonPath);
    ldtk::World* getLDtkWorld() { return ldtkWorld.get(); }
    ldtk::Level* getLevel(const std::string& identifier);
    
    // Tileset operations
    Texture2D* getTilesetTexture(int tilesetUid);
    bool loadTilesetTexture(int tilesetUid, const std::string& tilesetPath);
    
    // Utilities
    bool textureExists(const std::string& name) const;
    size_t getLoadedTextureCount() const { return textures.size(); }
    void printStatistics() const;
    
private:
    std::string basePath;
    std::unordered_map<std::string, Texture2D> textures;
    std::unordered_map<int, Texture2D> tilesetTextures; // Key: tileset UID
    std::unique_ptr<ldtk::World> ldtkWorld;
    
    std::string resolvePath(const std::string& path) const;
};

} // namespace assets

