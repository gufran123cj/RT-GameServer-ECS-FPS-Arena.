#include "../net/Socket.hpp"
#include "../net/Packet.hpp"
#include "../include/common/types.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace game;
using namespace game::net;

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
        std::cout << "[DEBUG] CONNECT packet sent to server" << std::endl;
    } else {
        std::cout << "[ERROR] Failed to send CONNECT packet!" << std::endl;
    }
    std::cout << "Connected to server " << serverIP << ":" << serverPort << std::endl;
    std::cout << "Viewing game map (updates every 1 second)...\n" << std::endl;
    
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
                static int debugHeartbeatCount = 0;
                if (debugHeartbeatCount < 3) {
                    std::cout << "[DEBUG] HEARTBEAT sent to server (seq=" << heartbeatSequence - 1 << ")" << std::endl;
                    debugHeartbeatCount++;
                }
            } else {
                std::cout << "[ERROR] Failed to send HEARTBEAT!" << std::endl;
            }
        }
        
        // Check for packets from server (longer timeout to catch snapshots)
        Packet packet;
        while (socket.receive(packet, 100)) {
            if (packet.size >= sizeof(PacketHeader)) {
                PacketReader reader(packet.data.data(), packet.size);
                PacketHeader recvHeader;
                if (reader.read(recvHeader)) {
                    if (recvHeader.type == PacketType::SNAPSHOT) {
                        lastServerTick = recvHeader.serverTick;
                        snapshotCount++;
                        
                        // Debug: Log snapshot reception (only first few times)
                        static int debugSnapshotReceiveCount = 0;
                        if (debugSnapshotReceiveCount < 3) {
                            std::cout << "[DEBUG] SNAPSHOT received! Tick=" << recvHeader.serverTick 
                                      << ", Size=" << packet.size << " bytes" << std::endl;
                            debugSnapshotReceiveCount++;
                        }
                        
                        // Read snapshot
                        uint8_t playerCount = 0;
                        if (reader.read(playerCount)) {
                            if (debugSnapshotReceiveCount <= 3) {
                                std::cout << "[DEBUG] Snapshot contains " << (int)playerCount << " players" << std::endl;
                            }
                            
                            players.clear();
                            players.reserve(playerCount);
                            
                            for (uint8_t i = 0; i < playerCount; i++) {
                                SnapshotPlayerData playerData;
                                if (reader.read(playerData)) {
                                    PlayerView view;
                                    view.id = playerData.playerID;
                                    view.x = playerData.x;
                                    view.y = playerData.y;
                                    view.z = playerData.z;
                                    view.yaw = playerData.yaw;
                                    view.inputFlags = playerData.inputFlags;
                                    players.push_back(view);
                                }
                            }
                        } else {
                            std::cout << "[ERROR] Failed to read player count from snapshot!" << std::endl;
                        }
                    } else if (recvHeader.type == PacketType::CONNECT) {
                        // Server acknowledged connection
                        std::cout << "[DEBUG] Server acknowledged connection, Player ID: " << recvHeader.playerID << std::endl;
                    } else {
                        // Debug: Log other packet types
                        static int debugOtherPacketCount = 0;
                        if (debugOtherPacketCount < 3) {
                            std::cout << "[DEBUG] Received packet type: " << (int)recvHeader.type << std::endl;
                            debugOtherPacketCount++;
                        }
                    }
                }
            }
        }
        
        // Render map every second (or immediately on first run)
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastRender).count() >= 1000) {
            // Clear screen and render
            #ifdef _WIN32
            system("cls");
            #endif
            
            std::cout << "========================================" << std::endl;
            std::cout << "=== MINI GAME - ASCII MAP VIEWER ===" << std::endl;
            std::cout << "Server: " << serverIP << ":" << serverPort << std::endl;
            std::cout << "Tick: " << lastServerTick << " | Players: " << players.size() << " | Snapshots received: " << snapshotCount << std::endl;
            std::cout << "========================================\n" << std::endl;
            
            if (players.empty()) {
                if (snapshotCount == 0) {
                    std::cout << "[INFO] Waiting for snapshot from server..." << std::endl;
                    std::cout << "[DEBUG] Make sure TestClient.exe is running to see players" << std::endl;
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
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    socket.close();
    return 0;
}
