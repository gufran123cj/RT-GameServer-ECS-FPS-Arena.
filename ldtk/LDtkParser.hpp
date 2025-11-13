#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// Forward declarations
namespace nlohmann {
    class json;
}

namespace ldtk {

// LDtk Tile data
struct Tile {
    int px[2];      // Pixel position [x, y]
    int src[2];     // Source position in tileset [x, y]
    int f;          // Flip bits
    int t;          // Tile ID
    int d[2];       // Destination [x, y] (optional)
    float a;        // Alpha (optional)
};

// LDtk Entity instance
struct EntityInstance {
    std::string identifier;
    int px[2];      // Pixel position [x, y]
    int widPx;      // Width in pixels
    int heiPx;      // Height in pixels
    std::unordered_map<std::string, std::string> fields; // Custom fields
};

// LDtk Layer
struct Layer {
    std::string identifier;
    std::string type;           // "Tiles", "Entities", "IntGrid", "AutoLayer"
    int cWid;                   // Cell width
    int cHei;                   // Cell height
    int gridSize;               // Grid size
    float opacity;
    bool visible;
    int tilesetDefUid;          // Tileset UID
    std::string tilesetRelPath; // Relative path to tileset
    std::vector<Tile> gridTiles;
    std::vector<Tile> autoLayerTiles;
    std::vector<EntityInstance> entityInstances;
    std::vector<int> intGridCsv; // IntGrid values
};

// LDtk Level
struct Level {
    std::string identifier;
    int pxWid;      // Pixel width
    int pxHei;      // Pixel height
    std::vector<Layer> layers;
};

// LDtk Tileset definition
struct TilesetDef {
    int uid;
    std::string identifier;
    std::string relPath;
    int pxWid;      // Pixel width
    int pxHei;      // Pixel height
    int tileGridSize;
    int spacing;
    int padding;
};

// LDtk World (main container)
struct World {
    std::string worldLayout;    // "Free", "GridVania", "LinearHorizontal", "LinearVertical"
    int worldGridWidth;
    int worldGridHeight;
    int defaultLevelWidth;
    int defaultLevelHeight;
    std::vector<Level> levels;
    std::unordered_map<int, TilesetDef> tilesets; // Key: UID
};

// LDtk Parser class
class LDtkParser {
public:
    // Load and parse LDtk JSON file
    static bool loadWorld(const std::string& jsonPath, World& world);
    
    // Helper: Get level by identifier
    static Level* getLevelByIdentifier(World& world, const std::string& identifier);
    
    // Helper: Get layer by identifier from level
    static Layer* getLayerByIdentifier(Level& level, const std::string& identifier);
    
    // Helper: Get all entities of a specific type
    static std::vector<EntityInstance> getEntitiesByType(const Level& level, const std::string& entityType);
    
private:
    // Internal parsing helpers
    static bool parseWorld(const nlohmann::json& json, World& world);
    static bool parseLevel(const nlohmann::json& json, Level& level);
    static bool parseLayer(const nlohmann::json& json, Layer& layer);
    static bool parseTile(const nlohmann::json& json, Tile& tile);
    static bool parseEntity(const nlohmann::json& json, EntityInstance& entity);
    static bool parseTileset(const nlohmann::json& json, TilesetDef& tileset);
};

} // namespace ldtk

