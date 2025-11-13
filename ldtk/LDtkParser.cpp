#include "LDtkParser.hpp"
#include "../include/json/json.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

using json = nlohmann::json;

namespace ldtk {

bool LDtkParser::loadWorld(const std::string& jsonPath, World& world) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        std::cerr << "[LDtk] Failed to open file: " << jsonPath << std::endl;
        return false;
    }
    
    json j;
    try {
        file >> j;
    } catch (const json::parse_error& e) {
        std::cerr << "[LDtk] JSON parse error: " << e.what() << std::endl;
        return false;
    }
    
    return parseWorld(j, world);
}

bool LDtkParser::parseWorld(const json& j, World& world) {
    try {
        world.worldLayout = j.value("worldLayout", "Free");
        world.worldGridWidth = j.value("worldGridWidth", 0);
        world.worldGridHeight = j.value("worldGridHeight", 0);
        world.defaultLevelWidth = j.value("defaultLevelWidth", 0);
        world.defaultLevelHeight = j.value("defaultLevelHeight", 0);
        
        // Parse tilesets
        if (j.contains("defs") && j["defs"].contains("tilesets")) {
            for (const auto& tilesetJson : j["defs"]["tilesets"]) {
                TilesetDef tileset;
                if (parseTileset(tilesetJson, tileset)) {
                    world.tilesets[tileset.uid] = tileset;
                }
            }
        }
        
        // Parse levels
        if (j.contains("levels")) {
            for (const auto& levelJson : j["levels"]) {
                Level level;
                if (parseLevel(levelJson, level)) {
                    world.levels.push_back(level);
                }
            }
        }
        
        return true;
    } catch (const json::exception& e) {
        std::cerr << "[LDtk] Error parsing world: " << e.what() << std::endl;
        return false;
    }
}

bool LDtkParser::parseLevel(const json& j, Level& level) {
    try {
        level.identifier = j.value("identifier", "");
        level.pxWid = j.value("pxWid", 0);
        level.pxHei = j.value("pxHei", 0);
        
        // Parse layers
        if (j.contains("layerInstances")) {
            for (const auto& layerJson : j["layerInstances"]) {
                Layer layer;
                if (parseLayer(layerJson, layer)) {
                    level.layers.push_back(layer);
                }
            }
        }
        
        return true;
    } catch (const json::exception& e) {
        std::cerr << "[LDtk] Error parsing level: " << e.what() << std::endl;
        return false;
    }
}

bool LDtkParser::parseLayer(const json& j, Layer& layer) {
    try {
        layer.identifier = j.value("__identifier", "");
        layer.type = j.value("__type", "");
        layer.cWid = j.value("__cWid", 0);
        layer.cHei = j.value("__cHei", 0);
        layer.gridSize = j.value("__gridSize", 16);
        layer.opacity = j.value("__opacity", 1.0f);
        layer.visible = j.value("visible", true);
        layer.tilesetDefUid = j.value("__tilesetDefUid", -1);
        layer.tilesetRelPath = j.value("__tilesetRelPath", "");
        
        // Parse grid tiles
        if (j.contains("gridTiles")) {
            for (const auto& tileJson : j["gridTiles"]) {
                Tile tile;
                if (parseTile(tileJson, tile)) {
                    layer.gridTiles.push_back(tile);
                }
            }
        }
        
        // Parse auto layer tiles
        if (j.contains("autoLayerTiles")) {
            for (const auto& tileJson : j["autoLayerTiles"]) {
                Tile tile;
                if (parseTile(tileJson, tile)) {
                    layer.autoLayerTiles.push_back(tile);
                }
            }
        }
        
        // Parse entities
        if (j.contains("entityInstances")) {
            for (const auto& entityJson : j["entityInstances"]) {
                EntityInstance entity;
                if (parseEntity(entityJson, entity)) {
                    layer.entityInstances.push_back(entity);
                }
            }
        }
        
        // Parse IntGrid
        if (j.contains("intGridCsv")) {
            layer.intGridCsv = j["intGridCsv"].get<std::vector<int>>();
        }
        
        return true;
    } catch (const json::exception& e) {
        std::cerr << "[LDtk] Error parsing layer: " << e.what() << std::endl;
        return false;
    }
}

bool LDtkParser::parseTile(const json& j, Tile& tile) {
    try {
        if (j.contains("px") && j["px"].is_array() && j["px"].size() >= 2) {
            tile.px[0] = j["px"][0];
            tile.px[1] = j["px"][1];
        }
        if (j.contains("src") && j["src"].is_array() && j["src"].size() >= 2) {
            tile.src[0] = j["src"][0];
            tile.src[1] = j["src"][1];
        }
        tile.f = j.value("f", 0);
        tile.t = j.value("t", 0);
        if (j.contains("d") && j["d"].is_array() && j["d"].size() >= 2) {
            tile.d[0] = j["d"][0];
            tile.d[1] = j["d"][1];
        }
        tile.a = j.value("a", 1.0f);
        return true;
    } catch (const json::exception& e) {
        std::cerr << "[LDtk] Error parsing tile: " << e.what() << std::endl;
        return false;
    }
}

bool LDtkParser::parseEntity(const json& j, EntityInstance& entity) {
    try {
        entity.identifier = j.value("__identifier", "");
        if (j.contains("px") && j["px"].is_array() && j["px"].size() >= 2) {
            entity.px[0] = j["px"][0];
            entity.px[1] = j["px"][1];
        }
        entity.widPx = j.value("widPx", 0);
        entity.heiPx = j.value("heiPx", 0);
        
        // Parse custom fields (simplified - can be extended)
        if (j.contains("fieldInstances")) {
            for (const auto& fieldJson : j["fieldInstances"]) {
                std::string fieldName = fieldJson.value("__identifier", "");
                // Field value parsing can be extended based on field type
                // For now, we'll just store the field name
            }
        }
        
        return true;
    } catch (const json::exception& e) {
        std::cerr << "[LDtk] Error parsing entity: " << e.what() << std::endl;
        return false;
    }
}

bool LDtkParser::parseTileset(const json& j, TilesetDef& tileset) {
    try {
        tileset.uid = j.value("uid", -1);
        tileset.identifier = j.value("identifier", "");
        tileset.relPath = j.value("relPath", "");
        tileset.pxWid = j.value("pxWid", 0);
        tileset.pxHei = j.value("pxHei", 0);
        tileset.tileGridSize = j.value("tileGridSize", 16);
        tileset.spacing = j.value("spacing", 0);
        tileset.padding = j.value("padding", 0);
        return true;
    } catch (const json::exception& e) {
        std::cerr << "[LDtk] Error parsing tileset: " << e.what() << std::endl;
        return false;
    }
}

Level* LDtkParser::getLevelByIdentifier(World& world, const std::string& identifier) {
    for (auto& level : world.levels) {
        if (level.identifier == identifier) {
            return &level;
        }
    }
    return nullptr;
}

Layer* LDtkParser::getLayerByIdentifier(Level& level, const std::string& identifier) {
    for (auto& layer : level.layers) {
        if (layer.identifier == identifier) {
            return &layer;
        }
    }
    return nullptr;
}

std::vector<EntityInstance> LDtkParser::getEntitiesByType(const Level& level, const std::string& entityType) {
    std::vector<EntityInstance> result;
    for (const auto& layer : level.layers) {
        if (layer.type == "Entities") {
            for (const auto& entity : layer.entityInstances) {
                if (entity.identifier == entityType) {
                    result.push_back(entity);
                }
            }
        }
    }
    return result;
}

} // namespace ldtk

