// Top-down 2D Game Client with Raylib
// Simple 2D visualization of the game world

#include "../net/Socket.hpp"
#include "../net/Packet.hpp"
#include "../include/common/types.hpp"
#include "../components/Position.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../components/CollisionComponent.hpp"
#include "../physics/Physics.hpp"
#include "../ecs/Component.hpp"
#include "../assets/AssetManager.hpp"
#include "../ldtk/LDtkParser.hpp"
#include "../ldtk/LevelRenderer.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>

// Raylib header (header-only mode)
#define RAYLIB_STANDALONE
#include "../raylib/include/raylib.h"

using namespace game;
using namespace game::net;
using namespace game::components;

// Simple player view structure
struct PlayerView {
    PlayerID id;
    float x, y, z;
    float yaw;
    uint16_t inputFlags;
    Color color;
    
    PlayerView() : id(INVALID_PLAYER), x(0), y(0), z(0), yaw(0), inputFlags(0), 
                   color(WHITE) {}
};

// Wall view structure
struct WallView {
    float x, y, z;
    float width, height, depth;
    
    WallView() : x(0), y(0), z(0), width(0), height(0), depth(0) {}
};

// Camera settings for top-down view (using Raylib's Camera2D)

int main(int argc, char* argv[]) {
    // Server address
    std::string serverIP = "127.0.0.1";
    uint16_t serverPort = 7777;
    
    if (argc > 1) serverIP = argv[1];
    if (argc > 2) serverPort = static_cast<uint16_t>(std::atoi(argv[2]));
    
    // Initialize Raylib window
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    
    InitWindow(screenWidth, screenHeight, "Top-Down 2D Game Client");
    SetTargetFPS(60);
    
    // Initialize Asset Manager and load LDtk map
    assets::AssetManager assetManager("sprites");
    ldtk::World* ldtkWorld = nullptr;
    ldtk::Level* currentLevel = nullptr;
    float mapWorldWidth = 32.0f;
    float mapWorldHeight = 16.0f;
    
    // Initialize Level Renderer
    ldtk::LevelRenderer levelRenderer;
    bool levelRendererInitialized = false;
    
    if (assetManager.loadLDtkWorld("map.json")) {
        ldtkWorld = assetManager.getLDtkWorld();
        if (ldtkWorld && !ldtkWorld->levels.empty()) {
            // Load first level
            currentLevel = &ldtkWorld->levels[0];
            
            // Initialize level renderer
            if (levelRenderer.initialize(ldtkWorld, &assetManager, currentLevel->identifier)) {
                // Parse collision tiles from JSON (enumTags support)
                levelRenderer.parseCollisionTilesFromJSON("sprites/map.json");
                levelRendererInitialized = true;
                
                std::cout << "[GameClient] LevelRenderer initialized successfully" << std::endl;
            }
            
            // Calculate actual map size in world units
            // LDtk uses pixels, we convert to world units (16 pixels = 1 world unit)
            // For rendering, we treat the map as if it's 1920x1080 pixels to fill screen
            float actualMapPixelWidth = static_cast<float>(currentLevel->pxWid);
            float actualMapPixelHeight = static_cast<float>(currentLevel->pxHei);
            
            // Scale map to fill screen (1920x1080)
            // Calculate scale factor to make map fill screen
            float scaleX = screenWidth / actualMapPixelWidth;
            float scaleY = screenHeight / actualMapPixelHeight;
            float scale = std::max(scaleX, scaleY); // Use larger scale to fill screen
            
            // Adjust world units based on scale
            mapWorldWidth = (actualMapPixelWidth * scale) / 16.0f;
            mapWorldHeight = (actualMapPixelHeight * scale) / 16.0f;
            
            std::cout << "[GameClient] Map scale factor: " << scale << "x (actual: " 
                      << actualMapPixelWidth << "x" << actualMapPixelHeight 
                      << " -> scaled: " << (actualMapPixelWidth * scale) << "x" 
                      << (actualMapPixelHeight * scale) << ")" << std::endl;
            std::cout << "[GameClient] Loaded LDtk level: " << currentLevel->identifier 
                      << " (" << currentLevel->pxWid << "x" << currentLevel->pxHei << ")" << std::endl;
            std::cout << "[GameClient] Level has " << currentLevel->layers.size() << " layers" << std::endl;
            
            // Debug: Print layer info
            for (const auto& layer : currentLevel->layers) {
                std::cout << "[GameClient] Layer: " << layer.identifier 
                          << " type: " << layer.type 
                          << " visible: " << layer.visible
                          << " tilesetUID: " << layer.tilesetDefUid
                          << " gridTiles: " << layer.gridTiles.size()
                          << " autoTiles: " << layer.autoLayerTiles.size() << std::endl;
            }
            
            // Debug: Print tileset info
            std::cout << "[GameClient] Tilesets loaded: " << ldtkWorld->tilesets.size() << std::endl;
            for (const auto& [uid, tileset] : ldtkWorld->tilesets) {
                std::cout << "[GameClient] Tileset UID: " << uid 
                          << " path: " << tileset.relPath 
                          << " size: " << tileset.pxWid << "x" << tileset.pxHei << std::endl;
                Texture2D* tex = assetManager.getTilesetTexture(uid);
                if (tex && tex->id != 0) {
                    std::cout << "[GameClient] Tileset texture loaded: " << tex->width << "x" << tex->height << std::endl;
                } else {
                    std::cerr << "[GameClient] WARNING: Tileset texture NOT loaded for UID " << uid << std::endl;
                }
            }
        }
    } else {
        std::cerr << "[GameClient] Failed to load LDtk map!" << std::endl;
    }
    
    // UDP socket
    UDPSocket socket;
    Address clientAddress("0.0.0.0", 0);
    if (!socket.bind(clientAddress)) {
        std::cerr << "Failed to bind client socket" << std::endl;
        CloseWindow();
        return 1;
    }
    
    Address serverAddress(serverIP, serverPort);
    
    // Send CONNECT packet
    PacketHeader header;
    header.type = PacketType::CONNECT;
    header.sequence = 0;
    header.serverTick = 0;
    header.playerID = INVALID_PLAYER;
    
    socket.send(serverAddress, &header, sizeof(header));
    
    // Player data storage
    std::vector<PlayerView> players;
    std::vector<WallView> walls;  // Wall/obstacle data storage
    
    // Camera for top-down view (Raylib Camera2D)
    Camera2D camera = {0};
    // Camera target will be set to map center (0,0) or player position
    camera.target = (Vector2){0, 0};
    camera.offset = (Vector2){screenWidth / 2.0f, screenHeight / 2.0f};
    camera.rotation = 0.0f;
    
    // Calculate zoom to fill entire screen (100% - no borders)
    // Map is scaled during rendering to fill screen, so zoom is 1.0
    // The scaling happens in the rendering loop, not here
    camera.zoom = 1.0f;
    std::cout << "[GameClient] Camera zoom (fixed): " << camera.zoom << "x (map: " 
              << mapWorldWidth << "x" << mapWorldHeight << " world units, screen: " 
              << screenWidth << "x" << screenHeight << ")" << std::endl;
    
    // Game state
    Tick lastServerTick = 0;
    int snapshotCount = 0;
    uint32_t heartbeatSequence = 0;
    auto lastHeartbeat = std::chrono::steady_clock::now();
    
    // Player ID (will be assigned by server, assume 0 for now)
    PlayerID playerID = 0;
    
    // Input state
    uint16_t inputFlags = 0;
    float mouseYaw = 0.0f; // Start looking up (North, +Y direction) in top-down 2D
    float mousePitch = 0.0f;
    uint32_t inputSequence = 1;
    auto lastInputSend = std::chrono::steady_clock::now();
    
    // Player colors
    Color playerColors[] = {
        RED, BLUE, GREEN, YELLOW, PURPLE, ORANGE, PINK, SKYBLUE
    };
    
    // Main game loop
    while (!WindowShouldClose()) {
        auto now = std::chrono::steady_clock::now();
        
        // Send HEARTBEAT every second
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHeartbeat).count() >= 1000) {
            PacketHeader heartbeatHeader;
            heartbeatHeader.type = PacketType::HEARTBEAT;
            heartbeatHeader.sequence = heartbeatSequence++;
            heartbeatHeader.serverTick = 0;
            heartbeatHeader.playerID = INVALID_PLAYER;
            
            socket.send(serverAddress, &heartbeatHeader, sizeof(heartbeatHeader));
            lastHeartbeat = now;
        }
        
        // Receive packets from server
        Packet packet;
        int receiveAttempts = 0;
        while (receiveAttempts < 5 && socket.receive(packet, 50)) {
            receiveAttempts++;
            if (packet.size >= sizeof(PacketHeader)) {
                PacketReader reader(packet.data.data(), packet.size);
                PacketHeader recvHeader;
                if (reader.read(recvHeader)) {
                    if (recvHeader.type == PacketType::SNAPSHOT) {
                        lastServerTick = recvHeader.serverTick;
                        snapshotCount++;
                        
                        // Update playerID from first snapshot if not set
                        if (playerID == 0 && recvHeader.playerID != INVALID_PLAYER) {
                            playerID = recvHeader.playerID;
                        }
                        
                        // Deserialize snapshot
                        uint8_t entityCount = 0;
                        if (reader.read(entityCount)) {
                            players.clear();
                            walls.clear();
                            players.reserve(entityCount);
                            walls.reserve(entityCount);
                            
                            for (uint8_t i = 0; i < entityCount; i++) {
                                EntityID entityID = INVALID_ENTITY;
                                if (!reader.read(entityID)) break;
                                
                                uint8_t componentCount = 0;
                                if (!reader.read(componentCount)) break;
                                
                                PlayerView view;
                                view.id = INVALID_PLAYER;
                                
                                WallView wallView;
                                bool isWall = false;
                                
                                for (uint8_t j = 0; j < componentCount; j++) {
                                    ComponentTypeID typeID = 0;
                                    if (!reader.read(typeID)) break;
                                    
                                    uint16_t componentSize = 0;
                                    if (!reader.read(componentSize)) break;
                                    
                                    size_t componentDataStart = reader.getOffset();
                                    
                                    if (typeID == Position::getStaticTypeID()) {
                                        Position pos;
                                        if (pos.deserialize(reader)) {
                                            view.x = pos.value.x;
                                            view.y = pos.value.y;  // Y eksenini kullan
                                            view.z = 0.0f;  // Z kullanma
                                            wallView.x = pos.value.x;
                                            wallView.y = pos.value.y;
                                            wallView.z = pos.value.z;
                                        }
                                    } else if (typeID == PlayerComponent::getStaticTypeID()) {
                                        PlayerComponent playerComp;
                                        if (playerComp.deserialize(reader)) {
                                            view.id = playerComp.playerID;
                                            view.color = playerColors[view.id % 8];
                                        }
                                    } else if (typeID == InputComponent::getStaticTypeID()) {
                                        InputComponent input;
                                        if (input.deserialize(reader)) {
                                            view.yaw = input.mouseYaw;
                                            view.inputFlags = input.flags;
                                        }
                                    } else if (typeID == CollisionComponent::getStaticTypeID()) {
                                        CollisionComponent collision;
                                        if (collision.deserialize(reader)) {
                                            // Check if it's a static wall
                                            if (collision.isStatic) {
                                                isWall = true;
                                                physics::Vec3 size = collision.bounds.size();
                                                wallView.width = size.x;
                                                wallView.height = size.y;
                                                wallView.depth = size.z;
                                            }
                                        }
                                    } else {
                                        // Skip unknown components
                                        reader.setPosition(componentDataStart + componentSize);
                                    }
                                    
                                    // Ensure reader position is correct
                                    size_t expectedEndPos = componentDataStart + componentSize;
                                    if (reader.getOffset() != expectedEndPos) {
                                        reader.setPosition(expectedEndPos);
                                    }
                                }
                                
                                // Add to appropriate list
                                if (view.id != INVALID_PLAYER) {
                                    players.push_back(view);
                                } else if (isWall) {
                                    walls.push_back(wallView);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        // Handle input
        inputFlags = 0;
        
        // Movement keys (WASD)
        if (IsKeyDown(KEY_W)) inputFlags |= INPUT_FORWARD;
        if (IsKeyDown(KEY_S)) inputFlags |= INPUT_BACKWARD;
        if (IsKeyDown(KEY_A)) inputFlags |= INPUT_LEFT;
        if (IsKeyDown(KEY_D)) inputFlags |= INPUT_RIGHT;
        if (IsKeyDown(KEY_SPACE)) inputFlags |= INPUT_JUMP;
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) inputFlags |= INPUT_CROUCH;
        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) inputFlags |= INPUT_SPRINT;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) inputFlags |= INPUT_SHOOT;
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) inputFlags |= INPUT_AIM;
        
        // Mouse movement for yaw (horizontal rotation)
        Vector2 mouseDelta = GetMouseDelta();
        mouseYaw += mouseDelta.x * 0.1f; // Sensitivity adjustment
        if (mouseYaw > 360.0f) mouseYaw -= 360.0f;
        if (mouseYaw < 0.0f) mouseYaw += 360.0f;
        
        // Send INPUT packet every frame (or at high rate)
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastInputSend).count() >= 16) { // ~60fps
            PacketHeader inputHeader;
            inputHeader.type = PacketType::INPUT;
            inputHeader.sequence = inputSequence++;
            inputHeader.serverTick = lastServerTick;
            inputHeader.playerID = playerID;
            
            InputPacket inputPacket;
            inputPacket.flags = inputFlags;
            inputPacket.mouseYaw = mouseYaw;
            inputPacket.mousePitch = mousePitch;
            
            PacketWriter writer;
            writer.write(inputHeader);
            writer.write(inputPacket);
            
            socket.send(serverAddress, writer.getData(), writer.getSize());
            lastInputSend = now;
        }
        
        // Update camera to follow own player (or average if no own player)
        PlayerView* ownPlayer = nullptr;
        for (auto& player : players) {
            if (player.id == playerID) {
                ownPlayer = &player;
                break;
            }
        }
        
        if (ownPlayer) {
            // Raylib'de Y ekseni aşağı pozitif, bizim sistemde yukarı pozitif
            camera.target = (Vector2){ownPlayer->x, -ownPlayer->y};
        } else if (!players.empty()) {
            float avgX = 0, avgY = 0;
            for (const auto& player : players) {
                avgX += player.x;
                avgY += player.y;
            }
            avgX /= players.size();
            avgY /= players.size();
            // Raylib'de Y ekseni aşağı pozitif, bizim sistemde yukarı pozitif
            camera.target = (Vector2){avgX, -avgY};
        }
        
        // Zoom is fixed - no zoom controls (map fills entire screen)
        
        // Rendering
        BeginDrawing();
        ClearBackground((Color){20, 20, 30, 255}); // Dark blue-gray background
        
        // Draw LDtk map
        BeginMode2D(camera);
        
        // Use LevelRenderer if initialized, otherwise fallback to old rendering
        if (levelRendererInitialized) {
            float deltaTime = GetFrameTime();
            levelRenderer.renderLevel(camera, deltaTime);
        } else if (currentLevel) {
            // Fallback to old rendering method
            static bool debugPrinted = false;
            if (!debugPrinted) {
                std::cout << "[GameClient] Using fallback rendering (LevelRenderer not initialized)" << std::endl;
                debugPrinted = true;
            }
            
            // Calculate scale once for all tiles (map should fill screen 1920x1080)
            float scaleX = screenWidth / static_cast<float>(currentLevel->pxWid);
            float scaleY = screenHeight / static_cast<float>(currentLevel->pxHei);
            float mapScale = std::max(scaleX, scaleY); // Use larger to fill screen
            
            for (const auto& layer : currentLevel->layers) {
                if (!layer.visible) continue;
                
                // Get tileset texture
                Texture2D* tilesetTex = assetManager.getTilesetTexture(layer.tilesetDefUid);
                if (!tilesetTex || tilesetTex->id == 0) {
                    continue;
                }
                
                // Get tileset definition for tile size
                int tileSize = layer.gridSize;
                if (ldtkWorld && ldtkWorld->tilesets.find(layer.tilesetDefUid) != ldtkWorld->tilesets.end()) {
                    tileSize = ldtkWorld->tilesets.at(layer.tilesetDefUid).tileGridSize;
                }
                
                // Render grid tiles
                for (const auto& tile : layer.gridTiles) {
                    float scaledPxWid = currentLevel->pxWid * mapScale;
                    float scaledPxHei = currentLevel->pxHei * mapScale;
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
                    Rectangle dstRect = {
                        worldX - (scaledTileSize / 32.0f),
                        worldY - (scaledTileSize / 32.0f),
                        scaledTileSize / 16.0f,
                        scaledTileSize / 16.0f
                    };
                    
                    Color tint = WHITE;
                    tint.a = static_cast<unsigned char>(layer.opacity * 255);
                    
                    DrawTexturePro(*tilesetTex, srcRect, dstRect, 
                                  (Vector2){dstRect.width * 0.5f, dstRect.height * 0.5f}, 
                                  0.0f, tint);
                }
                
                // Render auto layer tiles
                for (const auto& tile : layer.autoLayerTiles) {
                    float scaledPxWid = currentLevel->pxWid * mapScale;
                    float scaledPxHei = currentLevel->pxHei * mapScale;
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
        }
        
        // Draw walls/obstacles
        for (const auto& wall : walls) {
            // Convert 3D position to 2D (top-down: X->X, Y->Y)
            Vector2 pos2D = {wall.x, -wall.y};  // Y eksenini ters çevir
            
            // Draw wall rectangle (top-down view: width x height)
            Rectangle wallRect = {
                pos2D.x - wall.width * 0.5f,
                pos2D.y - wall.height * 0.5f,
                wall.width,
                wall.height
            };
            
            // Draw filled rectangle (dark gray)
            DrawRectangleRec(wallRect, (Color){60, 60, 80, 255});
            // Draw outline (lighter gray)
            DrawRectangleLinesEx(wallRect, 0.1f, (Color){100, 100, 120, 255});
        }
        
        // Draw players
        for (const auto& player : players) {
            // Convert 3D position to 2D (top-down: X->X, Y->Y)
            // Raylib'de Y ekseni aşağı pozitif, bizim sistemde yukarı pozitif
            // Bu yüzden Y'yi ters çeviriyoruz
            Vector2 pos2D = {player.x, -player.y};
            
            // Draw player circle
            DrawCircleV(pos2D, 0.5f, player.color);
            DrawCircleLinesV(pos2D, 0.5f, WHITE);
            
            // Draw direction indicator (based on yaw) - 2D oyunda yaw'a gerek yok ama gösteriyoruz
            // Y ekseni ters olduğu için direction'ı da ters çeviriyoruz
            if (player.yaw != 0.0f) {
                float adjustedYaw = player.yaw - 90.0f;
                float yawRad = adjustedYaw * (PI / 180.0f);
                Vector2 dir = {cosf(yawRad) * 0.8f, -sinf(yawRad) * 0.8f}; // Y'yi ters çevir
                DrawLineV(pos2D, {pos2D.x + dir.x, pos2D.y + dir.y}, WHITE);
            }
            
            // Draw player ID
            char idText[16];
            snprintf(idText, sizeof(idText), "%u", player.id);
            DrawText(idText, pos2D.x + 0.7f, pos2D.y - 0.7f, 0.3f, WHITE);
        }
        
        EndMode2D();
        
        // Draw UI overlay - Sol üst köşe
        DrawRectangle(10, 10, 320, 160, (Color){0, 0, 0, 180});
        DrawText(TextFormat("Server: %s:%d", serverIP.c_str(), serverPort), 20, 20, 16, WHITE);
        DrawText(TextFormat("Tick: %llu", lastServerTick), 20, 40, 16, WHITE);
        DrawText(TextFormat("Players: %zu | Your ID: %u", players.size(), playerID), 20, 60, 16, WHITE);
        DrawText(TextFormat("Snapshots: %d", snapshotCount), 20, 80, 16, WHITE);
        DrawText("Controls:", 20, 100, 14, GRAY);
        DrawText("WASD = Move | Mouse = Look", 20, 120, 12, LIGHTGRAY);
        DrawText("Space = Jump | Shift = Sprint", 20, 140, 12, LIGHTGRAY);
        
        // Draw UI overlay - Sağ üst köşe (Koordinatlar)
        if (ownPlayer) {
            int coordBoxWidth = 200;
            int coordBoxHeight = 80;
            int coordBoxX = screenWidth - coordBoxWidth - 10;
            int coordBoxY = 10;
            
            DrawRectangle(coordBoxX, coordBoxY, coordBoxWidth, coordBoxHeight, (Color){0, 0, 0, 180});
            DrawText("Position:", coordBoxX + 10, coordBoxY + 10, 14, GRAY);
            DrawText(TextFormat("X: %.2f", ownPlayer->x), coordBoxX + 10, coordBoxY + 30, 16, WHITE);
            DrawText(TextFormat("Y: %.2f", ownPlayer->y), coordBoxX + 10, coordBoxY + 50, 16, WHITE);
            
            // Yaw açısını da göster
            DrawText(TextFormat("Yaw: %.1f°", ownPlayer->yaw), coordBoxX + 120, coordBoxY + 30, 14, LIGHTGRAY);
        }
        
        EndDrawing();
    }
    
    socket.close();
    CloseWindow();
    return 0;
}

