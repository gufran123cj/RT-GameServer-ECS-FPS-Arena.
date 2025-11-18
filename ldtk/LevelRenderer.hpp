#pragma once

#include "LDtkParser.hpp"
#include "../raylib/include/raylib.h"
#include "../assets/AssetManager.hpp"
#include <vector>
#include <unordered_set>
#include <string>

namespace ldtk {

// Background layer data for parallax scrolling
struct BackgroundLayer {
    Texture2D texture;
    Vector2 position;
    float parallaxSpeed;
    float scale;
    bool enabled;
    
    BackgroundLayer() : position{0, 0}, parallaxSpeed(0.0f), scale(1.0f), enabled(false) {
        texture = {0}; // Empty texture
    }
};

// Tile collision data
struct TileCollision {
    Rectangle bounds;
    int tileID;
    bool hasCollision;
    
    TileCollision() : bounds{0, 0, 0, 0}, tileID(0), hasCollision(false) {}
};

// Level renderer with collision detection and parallax support
class LevelRenderer {
public:
    LevelRenderer();
    ~LevelRenderer();
    
    // Initialize renderer with level data
    bool initialize(World* world, assets::AssetManager* assetManager, const std::string& levelIdentifier);
    
    // Parse collision tiles from enumTags (like the downloaded project)
    void parseCollisionTiles(World* world);
    
    // Render level with all layers
    void renderLevel(Camera2D& camera, float deltaTime = 0.0f);
    
    // Render background layers with parallax
    void renderBackground(Camera2D& camera, float deltaTime);
    
    // Check collision at position
    bool checkCollision(const Vector2& position, float radius = 0.5f) const;
    
    // Get collision tiles (for debug visualization)
    const std::vector<TileCollision>& getCollisionTiles() const { return collisionTiles; }
    
    // Set parallax scrolling enabled
    void setParallaxEnabled(bool enabled) { parallaxEnabled = enabled; }
    bool isParallaxEnabled() const { return parallaxEnabled; }
    
    // Set background layer
    void setBackgroundLayer(int index, const std::string& texturePath, float parallaxSpeed = 0.0f, float scale = 1.0f);
    void clearBackgroundLayers();
    
    // Get current level
    Level* getCurrentLevel() { return currentLevel; }
    
    // Set background layer identifier filter (like "Background_layer")
    void setBackgroundLayerFilter(const std::string& filter) { backgroundLayerFilter = filter; }
    
    // Manually set collision tile IDs (for enumTags support)
    void setCollisionTileIDs(const std::vector<int>& tileIDs);
    
    // Parse collision tiles from JSON file (enumTags)
    bool parseCollisionTilesFromJSON(const std::string& jsonPath);
    
    // Rebuild collision tiles list (call after setting collision tile IDs)
    void rebuildCollisionTiles();
    
private:
    World* ldtkWorld;
    assets::AssetManager* assetManager;
    Level* currentLevel;
    
    // Collision data
    std::unordered_set<int> collisionTileIDs; // Tile IDs that have collision
    std::vector<TileCollision> collisionTiles; // All tiles with collision data
    
    // Background layers
    static constexpr int MAX_BACKGROUND_LAYERS = 5;
    BackgroundLayer backgroundLayers[MAX_BACKGROUND_LAYERS];
    bool parallaxEnabled;
    std::string backgroundLayerFilter;
    
    // Rendering helpers
    void renderLayer(const Layer& layer, Texture2D* tilesetTex, int tileSize, float mapScale, 
                     const Level& level, int screenWidth, int screenHeight);
    void updateParallax(Camera2D& camera, float deltaTime);
    void loadBackgroundTexture(BackgroundLayer& bgLayer, const std::string& path);
};

} // namespace ldtk

