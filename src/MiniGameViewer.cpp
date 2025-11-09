#include "../net/Socket.hpp"
#include "../net/Packet.hpp"
#include "../include/common/types.hpp"
#include "../components/Position.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/InputComponent.hpp"
#include "../ecs/Component.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace game;
using namespace game::net;
using namespace game::components;

// Simple player position structure for viewer
struct PlayerView {
    PlayerID id;
    float x, y, z;
    float yaw;
    uint16_t inputFlags;
    
    PlayerView() : id(INVALID_PLAYER), x(0), y(0), z(0), yaw(0), inputFlags(0) {}
};

int main(int argc, char* argv[]) {
    std::cout << "=== Mini Game Viewer ===" << std::endl;
    std::cout << "Connecting to server to view game map..." << std::endl;
    
    // Server address
    std::string serverIP = "127.0.0.1";
    uint16_t serverPort = 7777;
    
    if (argc > 1) serverIP = argv[1];
    if (argc > 2) serverPort = static_cast<uint16_t>(std::atoi(argv[2]));
    
    // UDP socket
    UDPSocket socket;
    Address clientAddress("0.0.0.0", 0);
    if (!socket.bind(clientAddress)) {
        std::cerr << "Failed to bind viewer socket" << std::endl;
        return 1;
    }
    
    Address serverAddress(serverIP, serverPort);
    
    // Send CONNECT to get player ID
    PacketHeader header;
    header.type = PacketType::CONNECT;
    header.sequence = 0;
    header.serverTick = 0;
    header.playerID = INVALID_PLAYER;
    
    if (socket.send(serverAddress, &header, sizeof(header))) {
        // std::cout << "[DEBUG] CONNECT packet sent to server" << std::endl;
    } else {
        std::cout << "[ERROR] Failed to send CONNECT packet!" << std::endl;
    }
    std::cout << "Connected to server " << serverIP << ":" << serverPort << std::endl;
    std::cout << "Viewing game map (updates continuously)...\n" << std::endl;
    
    // Wait a bit for server to process CONNECT
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Player data storage
    std::vector<PlayerView> players;
    
    // Request snapshot every second
    uint32_t heartbeatSequence = 0;
    auto lastHeartbeat = std::chrono::steady_clock::now() - std::chrono::seconds(2); // Send immediately
    auto lastRender = std::chrono::steady_clock::now() - std::chrono::seconds(2); // Render immediately
    Tick lastServerTick = 0;
    int snapshotCount = 0;
    
    // Main loop - render map every second
    while (true) {
        auto now = std::chrono::steady_clock::now();
        
        // Send HEARTBEAT every second to request snapshot
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastHeartbeat).count() >= 1000) {
            PacketHeader heartbeatHeader;
            heartbeatHeader.type = PacketType::HEARTBEAT;
            heartbeatHeader.sequence = heartbeatSequence++;
            heartbeatHeader.serverTick = 0;
            heartbeatHeader.playerID = INVALID_PLAYER;
            
            if (socket.send(serverAddress, &heartbeatHeader, sizeof(heartbeatHeader))) {
                lastHeartbeat = now;
                
                // Debug: Log heartbeat sending (only first few times)
                // static int debugHeartbeatCount = 0;
                // if (debugHeartbeatCount < 3) {
                //     std::cout << "[DEBUG] HEARTBEAT sent to server (seq=" << heartbeatSequence - 1 << ")" << std::endl;
                //     debugHeartbeatCount++;
                // }
            } else {
                std::cout << "[ERROR] Failed to send HEARTBEAT!" << std::endl;
            }
        }
        
        // Check for packets from server (longer timeout to catch snapshots)
        Packet packet;
        // Try to receive with longer timeout to catch snapshots
        int receiveAttempts = 0;
        while (receiveAttempts < 5 && socket.receive(packet, 200)) {
            receiveAttempts++;
            if (packet.size >= sizeof(PacketHeader)) {
                PacketReader reader(packet.data.data(), packet.size);
                PacketHeader recvHeader;
                if (reader.read(recvHeader)) {
                    if (recvHeader.type == PacketType::SNAPSHOT) {
                        lastServerTick = recvHeader.serverTick;
                        snapshotCount++;
                        
                        // Debug: Log snapshot reception (only first few times)
                        // static int debugSnapshotReceiveCount = 0;
                        // if (debugSnapshotReceiveCount < 3) {
                        //     std::cout << "[DEBUG] SNAPSHOT received! Tick=" << recvHeader.serverTick 
                        //               << ", Size=" << packet.size << " bytes" << std::endl;
                        //     debugSnapshotReceiveCount++;
                        // }
                        
                        // Phase 4: Component-based snapshot deserialization
                        uint8_t entityCount = 0;
                        if (reader.read(entityCount)) {
                            // if (debugSnapshotReceiveCount <= 3) {
                            //     std::cout << "[DEBUG] Snapshot contains " << (int)entityCount << " entities" << std::endl;
                            // }
                            
                            players.clear();
                            players.reserve(entityCount);
                            
                            for (uint8_t i = 0; i < entityCount; i++) {
                                EntityID entityID = INVALID_ENTITY;
                                if (!reader.read(entityID)) {
                                    std::cout << "[ERROR] Failed to read entity ID!" << std::endl;
                                    break;
                                }
                                
                                uint8_t componentCount = 0;
                                if (!reader.read(componentCount)) {
                                    std::cout << "[ERROR] Failed to read component count!" << std::endl;
                                    break;
                                }
                                
                                // Deserialize components
                                PlayerView view;
                                view.id = INVALID_PLAYER;
                                
                                for (uint8_t j = 0; j < componentCount; j++) {
                                    ComponentTypeID typeID = 0;
                                    if (!reader.read(typeID)) {
                                        std::cout << "[ERROR] Failed to read component type ID!" << std::endl;
                                        break;
                                    }
                                    
                                    // Read component size (needed to skip unknown components)
                                    uint16_t componentSize = 0;
                                    if (!reader.read(componentSize)) {
                                        std::cout << "[ERROR] Failed to read component size!" << std::endl;
                                        break;
                                    }
                                    
                                    // Store current reader position (start of component data)
                                    // This is after typeID and size have been read
                                    size_t componentDataStart = reader.getOffset();
                                    
                                    // Debug: Log component type IDs (always log first 20 components)
                                    // static int debugComponentTypeCount = 0;
                                    // if (debugComponentTypeCount < 20) {
                                    //     std::cout << "[DEBUG VIEWER] Entity " << (int)i << " Component " << (int)j 
                                    //               << ": typeID=" << typeID 
                                    //               << " (Position=" << Position::getStaticTypeID()
                                    //               << ", Player=" << PlayerComponent::getStaticTypeID()
                                    //               << ", Input=" << InputComponent::getStaticTypeID()
                                    //               << "), size=" << componentSize << std::endl;
                                    //     debugComponentTypeCount++;
                                    // }
                                    
                                    // Deserialize based on component type
                                    bool deserialized = false;
                                    size_t expectedEndPos = componentDataStart + componentSize;
                                    
                                    if (typeID == Position::getStaticTypeID()) {
                                        Position pos;
                                        if (pos.deserialize(reader)) {
                                            view.x = pos.value.x;
                                            view.y = pos.value.y;
                                            view.z = pos.value.z;
                                            deserialized = true;
                                            // if (debugComponentTypeCount <= 20) {
                                            //     std::cout << "[DEBUG] Position deserialized: (" << view.x << ", " << view.y << ", " << view.z << ")" << std::endl;
                                            // }
                                        } else {
                                            // if (debugComponentTypeCount <= 20) {
                                            //     std::cout << "[ERROR] Position deserialization failed! Expected size=" << componentSize 
                                            //               << ", actual read=" << (reader.getOffset() - componentDataStart) << std::endl;
                                            // }
                                        }
                                    } else if (typeID == PlayerComponent::getStaticTypeID()) {
                                        PlayerComponent playerComp;
                                        if (playerComp.deserialize(reader)) {
                                            view.id = playerComp.playerID;
                                            deserialized = true;
                                            // if (debugComponentTypeCount <= 20) {
                                            //     std::cout << "[DEBUG] PlayerComponent deserialized: playerID=" << view.id << std::endl;
                                            // }
                                        } else {
                                            // if (debugComponentTypeCount <= 20) {
                                            //     std::cout << "[ERROR] PlayerComponent deserialization failed! Expected size=" << componentSize 
                                            //               << ", actual read=" << (reader.getOffset() - componentDataStart) << std::endl;
                                            // }
                                        }
                                    } else if (typeID == InputComponent::getStaticTypeID()) {
                                        InputComponent input;
                                        if (input.deserialize(reader)) {
                                            view.yaw = input.mouseYaw;
                                            view.inputFlags = input.flags;
                                            deserialized = true;
                                            // if (debugComponentTypeCount <= 20) {
                                            //     std::cout << "[DEBUG] InputComponent deserialized: yaw=" << view.yaw 
                                            //               << ", flags=" << view.inputFlags << std::endl;
                                            // }
                                        } else {
                                            // if (debugComponentTypeCount <= 20) {
                                            //     std::cout << "[ERROR] InputComponent deserialization failed! Expected size=" << componentSize 
                                            //               << ", actual read=" << (reader.getOffset() - componentDataStart) << std::endl;
                                            // }
                                        }
                                    } else if (typeID == ComponentType::Transform) {
                                        // Transform component - skip it (we use Position instead)
                                        // Skip the component data by moving reader forward
                                        reader.setPosition(componentDataStart + componentSize);
                                        deserialized = true; // Mark as handled
                                        // if (debugComponentTypeCount <= 20) {
                                        //     std::cout << "[DEBUG] Skipping Transform component (size=" << componentSize << " bytes)" << std::endl;
                                        // }
                                    } else if (typeID == ComponentType::Health) {
                                        // Health component - skip it (not needed for viewer)
                                        // Skip the component data by moving reader forward
                                        reader.setPosition(componentDataStart + componentSize);
                                        deserialized = true; // Mark as handled
                                        // if (debugComponentTypeCount <= 20) {
                                        //     std::cout << "[DEBUG] Skipping Health component (size=" << componentSize << " bytes)" << std::endl;
                                        // }
                                    } else if (typeID == ComponentType::Velocity) {
                                        // Velocity component - skip it (not needed for viewer)
                                        // Skip the component data by moving reader forward
                                        reader.setPosition(componentDataStart + componentSize);
                                        deserialized = true; // Mark as handled
                                        // if (debugComponentTypeCount <= 20) {
                                        //     std::cout << "[DEBUG] Skipping Velocity component (size=" << componentSize << " bytes)" << std::endl;
                                        // }
                                    } else if (typeID == ComponentType::CollisionComponent) {
                                        // CollisionComponent - skip it (not needed for viewer)
                                        // Skip the component data by moving reader forward
                                        reader.setPosition(componentDataStart + componentSize);
                                        deserialized = true; // Mark as handled
                                        // if (debugComponentTypeCount <= 20) {
                                        //     std::cout << "[DEBUG] Skipping CollisionComponent (size=" << componentSize << " bytes)" << std::endl;
                                        // }
                                    }
                                    
                                    // Always ensure reader is at the correct position after deserialization
                                    // This handles both successful and failed deserialization
                                    size_t currentPos = reader.getOffset();
                                    if (currentPos != expectedEndPos) {
                                        // if (debugComponentTypeCount <= 20) {
                                        //     std::cout << "[DEBUG] Reader position mismatch! Expected=" << expectedEndPos 
                                        //               << ", actual=" << currentPos << ", adjusting..." << std::endl;
                                        // }
                                        reader.setPosition(expectedEndPos);
                                    }
                                    
                                    // If component was not deserialized (unknown type), log it
                                    if (!deserialized) {
                                        // Debug: Log unknown component (only first few times)
                                        // static int debugUnknownComponentCount = 0;
                                        // if (debugUnknownComponentCount < 10) {
                                        //     std::cout << "[DEBUG] Skipped component type " << typeID 
                                        //               << " (size=" << componentSize << " bytes)" << std::endl;
                                        //     debugUnknownComponentCount++;
                                        // }
                                    }
                                }
                                
                                // Debug: Log view state after deserializing all components
                                // static int debugViewStateCount = 0;
                                // if (debugViewStateCount < 5) {
                                //     std::cout << "[DEBUG] Entity " << (int)i << " view: id=" << view.id 
                                //               << " (INVALID=" << INVALID_PLAYER << "), pos=(" 
                                //               << view.x << ", " << view.y << ", " << view.z << ")" << std::endl;
                                //     debugViewStateCount++;
                                // }
                                
                                // Only add if we have valid player ID
                                if (view.id != INVALID_PLAYER) {
                                    players.push_back(view);
                                }
                            }
                        } else {
                            std::cout << "[ERROR] Failed to read entity count from snapshot!" << std::endl;
                        }
                    } else if (recvHeader.type == PacketType::CONNECT) {
                        // Server acknowledged connection
                        // std::cout << "[DEBUG] Server acknowledged connection, Player ID: " << recvHeader.playerID << std::endl;
                    } else {
                        // Debug: Log other packet types
                        // static int debugOtherPacketCount = 0;
                        // if (debugOtherPacketCount < 3) {
                        //     std::cout << "[DEBUG] Received packet type: " << (int)recvHeader.type << std::endl;
                        //     debugOtherPacketCount++;
                        // }
                    }
                }
            }
        }
        
        // Render map every second (or immediately on first run)
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRender).count() >= 100) {
            // Clear screen to update map in place
            #ifdef _WIN32
            system("cls");
            #else
            system("clear");
            #endif
            
            std::cout << "========================================" << std::endl;
            std::cout << "=== MINI GAME - ASCII MAP VIEWER ===" << std::endl;
            std::cout << "Server: " << serverIP << ":" << serverPort << std::endl;
            std::cout << "Tick: " << lastServerTick << " | Players: " << players.size() << " | Snapshots received: " << snapshotCount << std::endl;
            std::cout << "========================================\n" << std::endl;
            
            if (players.empty()) {
                if (snapshotCount == 0) {
                    std::cout << "[INFO] Waiting for snapshot from server..." << std::endl;
                    // std::cout << "[DEBUG] Make sure TestClient.exe is running to see players" << std::endl;
                } else {
                    std::cout << "[INFO] No players in game yet..." << std::endl;
                }
                std::cout << "\nPress Ctrl+C to exit" << std::endl;
            } else {
                // Render ASCII map
                const int MAP_WIDTH = 40;
                const int MAP_HEIGHT = 20;
                const float CELL_SIZE = 2.0f;
                
                char map[MAP_HEIGHT][MAP_WIDTH];
                for (int y = 0; y < MAP_HEIGHT; y++) {
                    for (int x = 0; x < MAP_WIDTH; x++) {
                        map[y][x] = '.';
                    }
                }
                
                // Draw players
                for (const auto& player : players) {
                    int mapX = static_cast<int>((player.x / CELL_SIZE) + (MAP_WIDTH / 2));
                    int mapZ = static_cast<int>((player.z / CELL_SIZE) + (MAP_HEIGHT / 2));
                    
                    mapX = std::max(0, std::min(MAP_WIDTH - 1, mapX));
                    mapZ = std::max(0, std::min(MAP_HEIGHT - 1, mapZ));
                    
                    char playerChar = '0' + (player.id % 10);
                    map[mapZ][mapX] = playerChar;
                    
                    // Show player info
                    std::cout << "Player " << player.id 
                              << " @ (" << player.x << ", " << player.z << ")"
                              << " | Yaw: " << player.yaw << " deg";
                    
                    std::string inputStr = "";
                    if (player.inputFlags & 0x01) inputStr += "W";
                    if (player.inputFlags & 0x02) inputStr += "S";
                    if (player.inputFlags & 0x04) inputStr += "A";
                    if (player.inputFlags & 0x08) inputStr += "D";
                    if (player.inputFlags & 0x40) inputStr += "+";
                    if (!inputStr.empty()) {
                        std::cout << " | Input: " << inputStr;
                    }
                    std::cout << std::endl;
                }
                
                // Draw map
                std::cout << "\n   ";
                for (int x = 0; x < MAP_WIDTH; x++) {
                    std::cout << (x % 10);
                }
                std::cout << "\n";
                
                for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
                    std::cout << (y % 10) << " ";
                    for (int x = 0; x < MAP_WIDTH; x++) {
                        std::cout << map[y][x];
                    }
                    std::cout << " " << (y % 10) << "\n";
                }
                
                std::cout << "   ";
                for (int x = 0; x < MAP_WIDTH; x++) {
                    std::cout << (x % 10);
                }
                std::cout << "\n";
                
                std::cout << "\nLegend: . = Empty, 0-9 = Players" << std::endl;
            }
            
            std::cout << "========================================\n" << std::endl;
            lastRender = now;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    socket.close();
    return 0;
}
