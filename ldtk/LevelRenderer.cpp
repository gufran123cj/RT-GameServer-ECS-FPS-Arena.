#include "LevelRenderer.hpp"
#include "../assets/AssetManager.hpp"
#include "../include/json/json.hpp"
#include <iostream>
#include <algorithm>
#include <fstream>

namespace ldtk {

LevelRenderer::LevelRenderer() 
    : ldtkWorld(nullptr), assetManager(nullptr), currentLevel(nullptr),
      parallaxEnabled(false), backgroundLayerFilter("Background_layer") {
    // Initialize background layers
    for (int i = 0; i < MAX_BACKGROUND_LAYERS; ++i) {
        backgroundLayers[i] = BackgroundLayer();
    }
}

LevelRenderer::~LevelRenderer() {
    clearBackgroundLayers();
}

bool LevelRenderer::initialize(World* world, assets::AssetManager* assetMgr, const std::string& levelIdentifier) {
    ldtkWorld = world;
    assetManager = assetMgr;
    
    if (!ldtkWorld || !assetManager) {
        std::cerr << "[LevelRenderer] Invalid world or asset manager" << std::endl;
        return false;
    }
    
    // Find level by identifier
    currentLevel = LDtkParser::getLevelByIdentifier(*ldtkWorld, levelIdentifier);
    if (!currentLevel) {
        // Try first level if identifier not found
        if (!ldtkWorld->levels.empty()) {
            currentLevel = &ldtkWorld->levels[0];
            std::cout << "[LevelRenderer] Level '" << levelIdentifier << "' not found, using first level: " 
                      << currentLevel->identifier << std::endl;
        } else {
            std::cerr << "[LevelRenderer] No levels found in world" << std::endl;
            return false;
        }
    }
    
    std::cout << "[LevelRenderer] Initialized with level: " << currentLevel->identifier << std::endl;
    
    // Parse collision tiles
    parseCollisionTiles(ldtkWorld);
    
    // Build collision tile list
    collisionTiles.clear();
    if (currentLevel) {
        for (const auto& layer : currentLevel->layers) {
            // Skip background layers
            if (!backgroundLayerFilter.empty() && layer.identifier.find(backgroundLayerFilter) != std::string::npos) {
                continue;
            }
            
            int tileSize = layer.gridSize;
            if (ldtkWorld && ldtkWorld->tilesets.find(layer.tilesetDefUid) != ldtkWorld->tilesets.end()) {
                tileSize = ldtkWorld->tilesets.at(layer.tilesetDefUid).tileGridSize;
            }
            
            // Process grid tiles
            for (const auto& tile : layer.gridTiles) {
                if (collisionTileIDs.find(tile.t) != collisionTileIDs.end()) {
                    TileCollision tileColl;
                    tileColl.tileID = tile.t;
                    tileColl.hasCollision = true;
                    
                    // Calculate bounds (in pixel coordinates, will be converted during collision check)
                    tileColl.bounds.x = static_cast<float>(tile.px[0]);
                    tileColl.bounds.y = static_cast<float>(tile.px[1]);
                    tileColl.bounds.width = static_cast<float>(tileSize);
                    tileColl.bounds.height = static_cast<float>(tileSize);
                    
                    collisionTiles.push_back(tileColl);
                }
            }
            
            // Process auto layer tiles
            for (const auto& tile : layer.autoLayerTiles) {
                if (collisionTileIDs.find(tile.t) != collisionTileIDs.end()) {
                    TileCollision tileColl;
                    tileColl.tileID = tile.t;
                    tileColl.hasCollision = true;
                    
                    tileColl.bounds.x = static_cast<float>(tile.px[0]);
                    tileColl.bounds.y = static_cast<float>(tile.px[1]);
                    tileColl.bounds.width = static_cast<float>(tileSize);
                    tileColl.bounds.height = static_cast<float>(tileSize);
                    
                    collisionTiles.push_back(tileColl);
                }
            }
        }
    }
    
    std::cout << "[LevelRenderer] Found " << collisionTiles.size() << " collision tiles" << std::endl;
    
    return true;
}

void LevelRenderer::parseCollisionTiles(World* world) {
    if (!world) return;
    
    collisionTileIDs.clear();
    
    // Parse collision tiles from enumTags
    // This requires loading the JSON file directly to access enumTags
    // The enumTags are in defs.tilesets[i].enumTags where enumTags[j].enumValueId == "Collision"
    
    // Note: This is a simplified approach - in production you might want to extend LDtkParser
    // to parse and store enumTags in the World structure
    
    std::cout << "[LevelRenderer] Collision tile parsing initialized" << std::endl;
    std::cout << "[LevelRenderer] Note: enumTags parsing requires JSON file access" << std::endl;
    std::cout << "[LevelRenderer] Use setCollisionTileIDs() or parse from IntGrid for now" << std::endl;
}

bool LevelRenderer::parseCollisionTilesFromJSON(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "[LevelRenderer] Failed to open JSON file for collision parsing: " << jsonPath << std::endl;
        return false;
    }
    
    nlohmann::json jsonData;
    try {
        file >> jsonData;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "[LevelRenderer] JSON parse error: " << e.what() << std::endl;
        return false;
    }
    
    collisionTileIDs.clear();
    
    // Parse enumTags from tilesets (like the downloaded project)
    if (jsonData.contains("defs") && jsonData["defs"].contains("tilesets")) {
        for (const auto& tileset : jsonData["defs"]["tilesets"]) {
            if (tileset.contains("enumTags")) {
                for (const auto& enumTag : tileset["enumTags"]) {
                    // Check if this enum tag is for collision
                    // The downloaded project uses enumTags to mark collision tiles
                    // We'll check for "Collision" enumValueId or similar
                    std::string enumValueId = enumTag.value("enumValueId", "");
                    
                    // Check if it's a collision-related enum (case-insensitive check)
                    std::string lowerEnum = enumValueId;
                    std::transform(lowerEnum.begin(), lowerEnum.end(), lowerEnum.begin(), ::tolower);
                    
                    if (lowerEnum.find("collision") != std::string::npos || 
                        lowerEnum.find("solid") != std::string::npos ||
                        lowerEnum.find("wall") != std::string::npos) {
                        
                        // Add all tile IDs from this enum tag
                        if (enumTag.contains("tileIds")) {
                            for (const auto& tileID : enumTag["tileIds"]) {
                                int id = tileID.get<int>();
                                collisionTileIDs.insert(id);
                            }
                        }
                    }
                }
            }
        }
    }
    
    std::cout << "[LevelRenderer] Parsed " << collisionTileIDs.size() << " collision tile IDs from enumTags" << std::endl;
    
    // Rebuild collision tiles list with new IDs
    rebuildCollisionTiles();
    
    return true;
}

void LevelRenderer::setCollisionTileIDs(const std::vector<int>& tileIDs) {
    collisionTileIDs.clear();
    for (int id : tileIDs) {
        collisionTileIDs.insert(id);
    }
    rebuildCollisionTiles();
}

void LevelRenderer::rebuildCollisionTiles() {
    collisionTiles.clear();
    
    if (!currentLevel) return;
    
    for (const auto& layer : currentLevel->layers) {
        // Skip background layers
        if (!backgroundLayerFilter.empty() && layer.identifier.find(backgroundLayerFilter) != std::string::npos) {
            continue;
        }
        
        int tileSize = layer.gridSize;
        if (ldtkWorld && ldtkWorld->tilesets.find(layer.tilesetDefUid) != ldtkWorld->tilesets.end()) {
            tileSize = ldtkWorld->tilesets.at(layer.tilesetDefUid).tileGridSize;
        }
        
        // Process grid tiles
        for (const auto& tile : layer.gridTiles) {
            if (collisionTileIDs.find(tile.t) != collisionTileIDs.end()) {
                TileCollision tileColl;
                tileColl.tileID = tile.t;
                tileColl.hasCollision = true;
                
                tileColl.bounds.x = static_cast<float>(tile.px[0]);
                tileColl.bounds.y = static_cast<float>(tile.px[1]);
                tileColl.bounds.width = static_cast<float>(tileSize);
                tileColl.bounds.height = static_cast<float>(tileSize);
                
                collisionTiles.push_back(tileColl);
            }
        }
        
        // Process auto layer tiles
        for (const auto& tile : layer.autoLayerTiles) {
            if (collisionTileIDs.find(tile.t) != collisionTileIDs.end()) {
                TileCollision tileColl;
                tileColl.tileID = tile.t;
                tileColl.hasCollision = true;
                
                tileColl.bounds.x = static_cast<float>(tile.px[0]);
                tileColl.bounds.y = static_cast<float>(tile.px[1]);
                tileColl.bounds.width = static_cast<float>(tileSize);
                tileColl.bounds.height = static_cast<float>(tileSize);
                
                collisionTiles.push_back(tileColl);
            }
        }
    }
    
    std::cout << "[LevelRenderer] Rebuilt collision tiles list: " << collisionTiles.size() << " tiles" << std::endl;
}

void LevelRenderer::renderLevel(Camera2D& camera, float deltaTime) {
    if (!currentLevel || !assetManager) return;
    
    // Update parallax if enabled
    if (parallaxEnabled) {
        updateParallax(camera, deltaTime);
    }
    
    // Render background layers first
    if (parallaxEnabled) {
        renderBackground(camera, deltaTime);
    }
    
    // Calculate map scale (to fill screen)
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    
    float scaleX = static_cast<float>(screenWidth) / static_cast<float>(currentLevel->pxWid);
    float scaleY = static_cast<float>(screenHeight) / static_cast<float>(currentLevel->pxHei);
    float mapScale = std::max(scaleX, scaleY);
    
    // Render all layers
    for (const auto& layer : currentLevel->layers) {
        // Skip background layers (they're rendered separately)
        if (!backgroundLayerFilter.empty() && layer.identifier.find(backgroundLayerFilter) != std::string::npos) {
            continue;
        }
        
        if (!layer.visible) continue;
        
        // Get tileset texture
        Texture2D* tilesetTex = assetManager->getTilesetTexture(layer.tilesetDefUid);
        if (!tilesetTex || tilesetTex->id == 0) {
            continue;
        }
        
        // Get tile size
        int tileSize = layer.gridSize;
        if (ldtkWorld && ldtkWorld->tilesets.find(layer.tilesetDefUid) != ldtkWorld->tilesets.end()) {
            tileSize = ldtkWorld->tilesets.at(layer.tilesetDefUid).tileGridSize;
        }
        
        renderLayer(layer, tilesetTex, tileSize, mapScale, *currentLevel, screenWidth, screenHeight);
    }
}

void LevelRenderer::renderLayer(const Layer& layer, Texture2D* tilesetTex, int tileSize, 
                                float mapScale, const Level& level, int screenWidth, int screenHeight) {
    if (!tilesetTex || tilesetTex->id == 0) return;
    
    // Render grid tiles
    for (const auto& tile : layer.gridTiles) {
        // Scale calculations
        float scaledPxWid = level.pxWid * mapScale;
        float scaledPxHei = level.pxHei * mapScale;
        float scaledTilePxX = tile.px[0] * mapScale;
        float scaledTilePxY = tile.px[1] * mapScale;
        
        // Convert to world coordinates
        float worldX = (scaledTilePxX - scaledPxWid / 2.0f) / 16.0f;
        float worldY = -(scaledTilePxY - scaledPxHei / 2.0f) / 16.0f;
        
        // Source rectangle
        Rectangle srcRect = {
            static_cast<float>(tile.src[0]),
            static_cast<float>(tile.src[1]),
            static_cast<float>(tileSize),
            static_cast<float>(tileSize)
        };
        
        // Destination rectangle
        float scaledTileSize = tileSize * mapScale;
        Rectangle dstRect = {
            worldX - (scaledTileSize / 32.0f),
            worldY - (scaledTileSize / 32.0f),
            scaledTileSize / 16.0f,
            scaledTileSize / 16.0f
        };
        
        // Apply opacity
        Color tint = WHITE;
        tint.a = static_cast<unsigned char>(layer.opacity * 255);
        
        // Draw tile
        DrawTexturePro(*tilesetTex, srcRect, dstRect,
                      (Vector2){dstRect.width * 0.5f, dstRect.height * 0.5f},
                      0.0f, tint);
    }
    
    // Render auto layer tiles
    for (const auto& tile : layer.autoLayerTiles) {
        float scaledPxWid = level.pxWid * mapScale;
        float scaledPxHei = level.pxHei * mapScale;
        float scaledTilePxX = tile.px[0] * mapScale;
        float scaledTilePxY = tile.px[1] * mapScale;
        
        float worldX = (scaledTilePxX - scaledPxWid / 2.0f) / 16.0f;
        float worldY = -(scaledTilePxY - scaledPxHei / 2.0f) / 16.0f;
        
        Rectangle srcRect = {
            static_cast<float>(tile.src[0]),
            static_cast<float>(tile.src[1]),
            static_cast<float>(tileSize),
            static_cast<float>(tileSize)
        };
        
        float scaledTileSize = tileSize * mapScale;
        float tileWorldSize = scaledTileSize / 16.0f;
        Rectangle dstRect = {
            worldX - tileWorldSize * 0.5f,
            worldY - tileWorldSize * 0.5f,
            tileWorldSize,
            tileWorldSize
        };
        
        Color tint = WHITE;
        tint.a = static_cast<unsigned char>(layer.opacity * tile.a * 255);
        
        DrawTexturePro(*tilesetTex, srcRect, dstRect,
                      (Vector2){0, 0},
                      0.0f, tint);
    }
}

void LevelRenderer::renderBackground(Camera2D& camera, float deltaTime) {
    for (int i = 0; i < MAX_BACKGROUND_LAYERS; ++i) {
        BackgroundLayer& bg = backgroundLayers[i];
        if (!bg.enabled || bg.texture.id == 0) continue;
        
        // Draw background texture
        DrawTextureEx(bg.texture, bg.position, 0.0f, bg.scale, WHITE);
        
        // If parallax scrolling, draw second copy for seamless looping
        if (bg.parallaxSpeed != 0.0f) {
            Vector2 secondPos = bg.position;
            secondPos.x += bg.texture.width * bg.scale;
            DrawTextureEx(bg.texture, secondPos, 0.0f, bg.scale, WHITE);
        }
    }
}

void LevelRenderer::updateParallax(Camera2D& camera, float deltaTime) {
    float baseSpeed = 20.0f; // Base parallax speed multiplier
    
    for (int i = 0; i < MAX_BACKGROUND_LAYERS; ++i) {
        BackgroundLayer& bg = backgroundLayers[i];
        if (!bg.enabled || bg.parallaxSpeed == 0.0f) continue;
        
        // Update position based on camera movement and parallax speed
        float parallaxOffset = bg.parallaxSpeed * baseSpeed * deltaTime;
        bg.position.x -= parallaxOffset;
        
        // Reset position for seamless looping
        if (bg.position.x <= -bg.texture.width * bg.scale * 2.0f) {
            bg.position.x = 0.0f;
        }
    }
}

bool LevelRenderer::checkCollision(const Vector2& position, float radius) const {
    // Convert world position to pixel coordinates for collision check
    // This is a simplified collision check - you may need to adjust based on your coordinate system
    
    for (const auto& tileColl : collisionTiles) {
        if (!tileColl.hasCollision) continue;
        
        // Convert tile bounds to world coordinates (simplified)
        // In a real implementation, you'd need to account for map scale and coordinate transforms
        Rectangle worldBounds = tileColl.bounds; // This needs proper conversion
        
        // Simple circle-rectangle collision
        Vector2 closestPoint;
        closestPoint.x = std::max(worldBounds.x, std::min(position.x, worldBounds.x + worldBounds.width));
        closestPoint.y = std::max(worldBounds.y, std::min(position.y, worldBounds.y + worldBounds.height));
        
        float dx = position.x - closestPoint.x;
        float dy = position.y - closestPoint.y;
        float distanceSq = dx * dx + dy * dy;
        
        if (distanceSq < radius * radius) {
            return true;
        }
    }
    
    return false;
}

void LevelRenderer::setBackgroundLayer(int index, const std::string& texturePath, float parallaxSpeed, float scale) {
    if (index < 0 || index >= MAX_BACKGROUND_LAYERS) return;
    
    BackgroundLayer& bg = backgroundLayers[index];
    loadBackgroundTexture(bg, texturePath);
    bg.parallaxSpeed = parallaxSpeed;
    bg.scale = scale;
    bg.enabled = (bg.texture.id != 0);
}

void LevelRenderer::clearBackgroundLayers() {
    for (int i = 0; i < MAX_BACKGROUND_LAYERS; ++i) {
        if (backgroundLayers[i].texture.id != 0) {
            UnloadTexture(backgroundLayers[i].texture);
            backgroundLayers[i] = BackgroundLayer();
        }
    }
}

void LevelRenderer::loadBackgroundTexture(BackgroundLayer& bgLayer, const std::string& path) {
    if (bgLayer.texture.id != 0) {
        UnloadTexture(bgLayer.texture);
    }
    
    if (path.empty()) {
        bgLayer.texture = {0};
        bgLayer.enabled = false;
        return;
    }
    
    bgLayer.texture = LoadTexture(path.c_str());
    if (bgLayer.texture.id == 0) {
        std::cerr << "[LevelRenderer] Failed to load background texture: " << path << std::endl;
        bgLayer.enabled = false;
    } else {
        bgLayer.enabled = true;
        std::cout << "[LevelRenderer] Loaded background texture: " << path << std::endl;
    }
}

} // namespace ldtk

