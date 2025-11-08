#include "Server.hpp"
#include "../net/Packet.hpp"
#include "../components/Components.hpp"
#include "../systems/MovementSystem.hpp"
#include <iostream>
#include <thread>
#include <algorithm>

namespace game {

GameServer::GameServer(const std::string& bindIP, uint16_t port, int tickRate)
    : nextPlayerID(0), nextRoomID(0), serverTick(0), serverTickRate(tickRate),
      accumulatedTime(0.0f) {
    serverAddress = net::Address(bindIP, port);
    socket = std::make_unique<net::UDPSocket>();
}

GameServer::~GameServer() {
    shutdown();
}

bool GameServer::initialize() {
    if (!socket->bind(serverAddress)) {
        std::cerr << "Failed to bind server socket to " 
                  << serverAddress.ip << ":" << serverAddress.port << std::endl;
        return false;
    }
    
    lastTickTime = std::chrono::steady_clock::now();
    std::cout << "Game Server initialized on " 
              << serverAddress.ip << ":" << serverAddress.port 
              << " (Tick Rate: " << serverTickRate << ")" << std::endl;
    
    return true;
}

void GameServer::run() {
    const float fixedDeltaTime = 1.0f / serverTickRate;
    
    while (true) {
        auto currentTime = std::chrono::steady_clock::now();
        auto frameTime = std::chrono::duration<float>(currentTime - lastTickTime).count();
        lastTickTime = currentTime;
        
        // Clamp frame time
        frameTime = std::min(frameTime, MAX_DELTA_TIME);
        accumulatedTime += frameTime;
        
        // Process network packets
        processPackets();
        
        // Fixed timestep update
        while (accumulatedTime >= fixedDeltaTime) {
            updateRooms(fixedDeltaTime);
            serverTick++;
            accumulatedTime -= fixedDeltaTime;
        }
        
        // Send snapshots
        sendSnapshots();
        
        // Sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void GameServer::processPackets() {
    net::Packet packet;
    while (socket->receive(packet, 0)) {
        if (packet.size < sizeof(net::PacketHeader)) {
            continue;
        }
        
        net::PacketReader reader(packet.data.data(), packet.size);
        net::PacketHeader header;
        if (!reader.read(header)) {
            continue;
        }
        
        // Debug: Log all received packets (only first few times)
        static int debugPacketCount = 0;
        if (debugPacketCount < 5) {
            std::cout << "[DEBUG] Received packet type=" << (int)header.type 
                      << " from " << packet.from.ip << ":" << packet.from.port << std::endl;
            debugPacketCount++;
        }
        
        // Find or create player
        Player* player = nullptr;
        for (auto& [id, p] : players) {
            if (p->address == packet.from) {
                player = p.get();
                break;
            }
        }
        
        if (!player && header.type == net::PacketType::CONNECT) {
            PlayerID newID = addPlayer(packet.from);
            player = players[newID].get();
            
            // Create default room and add player entity
            Room* defaultRoom = getOrCreateRoom(0);
            createPlayerEntity(defaultRoom, newID);
            player->currentRoom = defaultRoom->id;
            defaultRoom->players.push_back(newID);
            
            std::cout << "Player " << newID << " connected from " 
                      << packet.from.ip << ":" << packet.from.port << std::endl;
        }
        
        // Handle packets from unknown players (might be HEARTBEAT from viewer)
        if (!player && header.type == net::PacketType::HEARTBEAT) {
            // Viewer might send HEARTBEAT before CONNECT is processed
            // Create player for this address
            PlayerID newID = addPlayer(packet.from);
            player = players[newID].get();
            
            Room* defaultRoom = getOrCreateRoom(0);
            player->currentRoom = defaultRoom->id;
            
            std::cout << "[DEBUG] Created player " << newID << " from HEARTBEAT (viewer?) from " 
                      << packet.from.ip << ":" << packet.from.port << std::endl;
        }
        
        if (player) {
            player->lastSeenTick = serverTick;
            
            // Handle different packet types
            switch (header.type) {
                case net::PacketType::INPUT:
                    processInputPacket(player, reader, header.sequence);
                    break;
                case net::PacketType::HEARTBEAT:
                    // Heartbeat received - snapshot will be sent in sendSnapshots()
                    // Also handle viewer connections (players not in room)
                    if (player->currentRoom == INVALID_ROOM) {
                        // This might be a viewer - add to default room for snapshot access
                        Room* defaultRoom = getOrCreateRoom(0);
                        player->currentRoom = defaultRoom->id;
                        std::cout << "[DEBUG] HEARTBEAT from Player " << player->id 
                                  << " - added to room " << defaultRoom->id << std::endl;
                    } else {
                        // Debug: Log HEARTBEAT from known player
                        static int debugHeartbeatKnownCount = 0;
                        if (debugHeartbeatKnownCount < 3) {
                            std::cout << "[DEBUG] HEARTBEAT from Player " << player->id 
                                      << " (room=" << player->currentRoom << ")" << std::endl;
                            debugHeartbeatKnownCount++;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void GameServer::updateRooms(float deltaTime) {
    for (auto& [roomID, room] : rooms) {
        if (room->isActive) {
            room->world.update(deltaTime);
            room->currentTick++;
            
            // Mini Game map rendering moved to MiniGameViewer.exe
            // (Will use snapshot system in Phase 4)
        }
    }
}

void GameServer::sendSnapshots() {
    // Simple snapshot sending (Phase 3 - will be replaced in Phase 4)
    // Send snapshot to all connected players/viewers from default room (room 0)
    Room* defaultRoom = nullptr;
    auto roomIt = rooms.find(0);
    if (roomIt != rooms.end()) {
        defaultRoom = roomIt->second.get();
    }
    
    if (!defaultRoom || !defaultRoom->isActive) {
        return; // No default room yet
    }
    
    // Get all player entities in default room
    auto playerEntities = defaultRoom->world.queryEntities<components::PlayerComponent>();
    
    // Debug: Log room state (only first few times)
    static int debugRoomStateCount = 0;
    if (debugRoomStateCount < 3) {
        std::cout << "[DEBUG] sendSnapshots: Room 0 active, " << playerEntities.size() 
                  << " entities, " << players.size() << " connected players" << std::endl;
        debugRoomStateCount++;
    }
    
    // Send snapshot to all connected players/viewers (every 10 ticks = ~6 times per second at 60fps)
    static Tick lastSnapshotTick = 0;
    if (serverTick > 0 && serverTick - lastSnapshotTick < 10) {
        return; // Throttle snapshot sending (but send on first tick)
    }
    lastSnapshotTick = serverTick;
    
    // Debug: Log snapshot attempt
    static int debugSnapshotAttemptCount = 0;
    if (debugSnapshotAttemptCount < 5) {
        std::cout << "[DEBUG] sendSnapshots called at tick " << serverTick 
                  << ", players.size()=" << players.size() << std::endl;
        debugSnapshotAttemptCount++;
    }
    
    if (players.empty()) {
        static int debugEmptyPlayersCount = 0;
        if (debugEmptyPlayersCount < 2) {
            std::cout << "[DEBUG] sendSnapshots: No players connected yet" << std::endl;
            debugEmptyPlayersCount++;
        }
        return;
    }
    
    for (auto& [playerID, player] : players) {
        if (!player->connected) {
            static int debugDisconnectedCount = 0;
            if (debugDisconnectedCount < 2) {
                std::cout << "[DEBUG] Player " << playerID << " is not connected" << std::endl;
                debugDisconnectedCount++;
            }
            continue;
        }
        
        // Build snapshot packet
        net::PacketWriter writer;
        net::PacketHeader header;
        header.type = net::PacketType::SNAPSHOT;
        header.sequence = 0;
        header.serverTick = serverTick;
        header.playerID = playerID;
        
        writer.write(header);
        
        // Write player count
        uint8_t playerCount = static_cast<uint8_t>(playerEntities.size());
        writer.write(playerCount);
        
        // Write each player's data
        for (EntityID entityID : playerEntities) {
            auto* pos = defaultRoom->world.getComponent<components::Position>(entityID);
            auto* playerComp = defaultRoom->world.getComponent<components::PlayerComponent>(entityID);
            auto* input = defaultRoom->world.getComponent<components::InputComponent>(entityID);
            
            if (pos && playerComp) {
                net::SnapshotPlayerData playerData;
                playerData.playerID = playerComp->playerID;
                playerData.x = pos->value.x;
                playerData.y = pos->value.y;
                playerData.z = pos->value.z;
                playerData.yaw = input ? input->mouseYaw : 0.0f;
                playerData.inputFlags = input ? input->flags : 0;
                
                writer.write(playerData);
            }
        }
        
        // Send snapshot (even if empty - viewer needs to know there are no players)
        if (writer.getSize() > sizeof(net::PacketHeader)) {
            if (socket->send(player->address, writer.getData(), writer.getSize())) {
                // Debug: Log snapshot sending (only first few times)
                static int debugSnapshotCount = 0;
                if (debugSnapshotCount < 3) {
                    std::cout << "[DEBUG] Snapshot sent to Player " << playerID 
                              << " at " << player->address.ip << ":" << player->address.port
                              << " (" << (int)playerCount << " players, " << writer.getSize() << " bytes)" << std::endl;
                    debugSnapshotCount++;
                }
            } else {
                std::cout << "[ERROR] Failed to send snapshot to Player " << playerID << std::endl;
            }
        } else {
            static int debugEmptySnapshotCount = 0;
            if (debugEmptySnapshotCount < 2) {
                std::cout << "[DEBUG] Snapshot too small to send (only header)" << std::endl;
                debugEmptySnapshotCount++;
            }
        }
    }
}

Room* GameServer::getOrCreateRoom(RoomID roomID) {
    auto it = rooms.find(roomID);
    if (it != rooms.end()) {
        return it->second.get();
    }
    
    RoomID newRoomID = createRoom();
    return rooms[newRoomID].get();
}

PlayerID GameServer::addPlayer(const net::Address& address) {
    PlayerID id = nextPlayerID++;
    players[id] = std::make_unique<Player>(id, address);
    return id;
}

void GameServer::removePlayer(PlayerID playerID) {
    auto it = players.find(playerID);
    if (it != players.end()) {
        // Remove from room
        RoomID roomID = it->second->currentRoom;
        if (roomID != INVALID_ROOM) {
            auto roomIt = rooms.find(roomID);
            if (roomIt != rooms.end()) {
                auto& roomPlayers = roomIt->second->players;
                roomPlayers.erase(
                    std::remove(roomPlayers.begin(), roomPlayers.end(), playerID),
                    roomPlayers.end()
                );
            }
        }
        players.erase(it);
        std::cout << "Player " << playerID << " disconnected" << std::endl;
    }
}

RoomID GameServer::createRoom(int tickRate) {
    RoomID id = nextRoomID++;
    rooms[id] = std::make_unique<Room>(id, tickRate);
    
    // Add Movement System to room's world
    auto movementSystem = std::make_unique<systems::MovementSystem>();
    rooms[id]->world.addSystem(std::move(movementSystem));
    
    std::cout << "Room " << id << " created (Tick Rate: " << tickRate << ") - Movement System added" << std::endl;
    std::cout << "Mini Game ASCII Map will render every 1 second..." << std::endl;
    return id;
}

EntityID GameServer::createPlayerEntity(Room* room, PlayerID playerID) {
    if (!room) return INVALID_ENTITY;
    
    // Create entity
    EntityID entityID = room->world.createEntity();
    
    // Add components
    auto position = std::make_unique<components::Position>(0.0f, 0.0f, 0.0f);
    auto velocity = std::make_unique<components::Velocity>(0.0f, 0.0f, 0.0f);
    auto health = std::make_unique<components::Health>(100.0f);
    auto playerComp = std::make_unique<components::PlayerComponent>(playerID);
    auto transform = std::make_unique<components::Transform>();
    
    room->world.addComponent<components::Position>(entityID, std::move(position));
    room->world.addComponent<components::Velocity>(entityID, std::move(velocity));
    room->world.addComponent<components::Health>(entityID, std::move(health));
    room->world.addComponent<components::PlayerComponent>(entityID, std::move(playerComp));
    room->world.addComponent<components::Transform>(entityID, std::move(transform));
    
    // Add InputComponent for player input handling
    auto input = std::make_unique<components::InputComponent>();
    room->world.addComponent<components::InputComponent>(entityID, std::move(input));
    
    // Verify components were added correctly (PHASE 1 TEST)
    auto* pos = room->world.getComponent<components::Position>(entityID);
    auto* vel = room->world.getComponent<components::Velocity>(entityID);
    auto* hp = room->world.getComponent<components::Health>(entityID);
    auto* player = room->world.getComponent<components::PlayerComponent>(entityID);
    auto* trans = room->world.getComponent<components::Transform>(entityID);
    
    std::cout << "\n=== PHASE 1 TEST: Player Entity Created ===" << std::endl;
    std::cout << "Entity ID: " << entityID << " | Player ID: " << playerID << std::endl;
    
    if (pos) {
        std::cout << "  [OK] Position: (" << pos->value.x << ", " << pos->value.y << ", " << pos->value.z << ")" << std::endl;
    } else {
        std::cout << "  [FAIL] Position: FAILED" << std::endl;
    }
    
    if (vel) {
        std::cout << "  [OK] Velocity: (" << vel->value.x << ", " << vel->value.y << ", " << vel->value.z << ")" << std::endl;
    } else {
        std::cout << "  [FAIL] Velocity: FAILED" << std::endl;
    }
    
    if (hp) {
        std::cout << "  [OK] Health: " << hp->current << "/" << hp->maximum << " (" << (hp->isAlive ? "Alive" : "Dead") << ")" << std::endl;
    } else {
        std::cout << "  [FAIL] Health: FAILED" << std::endl;
    }
    
    if (player) {
        std::cout << "  [OK] PlayerComponent: ID=" << player->playerID << ", Rating=" << player->rating << std::endl;
    } else {
        std::cout << "  [FAIL] PlayerComponent: FAILED" << std::endl;
    }
    
    if (trans) {
        std::cout << "  [OK] Transform: Pos(" << trans->position.x << ", " << trans->position.y << ", " << trans->position.z << ")" << std::endl;
    } else {
        std::cout << "  [FAIL] Transform: FAILED" << std::endl;
    }
    
    // Test ECS query system
    auto playerEntities = room->world.queryEntities<components::PlayerComponent>();
    std::cout << "  [OK] ECS Query: Found " << playerEntities.size() << " player entity/entities in world" << std::endl;
    
    std::cout << "==========================================\n" << std::endl;
    
    return entityID;
}

EntityID GameServer::getPlayerEntity(Room* room, PlayerID playerID) {
    if (!room) return INVALID_ENTITY;
    
    // Find entity with matching PlayerComponent
    auto playerEntities = room->world.queryEntities<components::PlayerComponent>();
    for (EntityID entityID : playerEntities) {
        auto* playerComp = room->world.getComponent<components::PlayerComponent>(entityID);
        if (playerComp && playerComp->playerID == playerID) {
            return entityID;
        }
    }
    
    return INVALID_ENTITY;
}

void GameServer::processInputPacket(Player* player, net::PacketReader& reader, SequenceNumber sequence) {
    if (!player || player->currentRoom == INVALID_ROOM) return;
    
    // Find player's room
    auto roomIt = rooms.find(player->currentRoom);
    if (roomIt == rooms.end()) return;
    
    Room* room = roomIt->second.get();
    
    // Find player's entity
    EntityID entityID = getPlayerEntity(room, player->id);
    if (entityID == INVALID_ENTITY) return;
    
    // Read input packet
    net::InputPacket inputPacket;
    if (!reader.read(inputPacket)) {
        return; // Invalid packet
    }
    
    // Get or create InputComponent
    auto* inputComp = room->world.getComponent<components::InputComponent>(entityID);
    if (!inputComp) {
        // Create InputComponent if it doesn't exist
        auto input = std::make_unique<components::InputComponent>();
        inputComp = room->world.addComponent<components::InputComponent>(entityID, std::move(input));
    }
    
    if (inputComp) {
        // Update input component
        inputComp->flags = inputPacket.flags;
        inputComp->mouseYaw = inputPacket.mouseYaw;
        inputComp->mousePitch = inputPacket.mousePitch;
        inputComp->sequence = sequence;
        inputComp->inputTick = serverTick;
        
        // Debug output - her 10 pakette bir göster (daha sık)
        static Tick lastDebugTick = 0;
        static int packetCount = 0;
        packetCount++;
        
        if (serverTick - lastDebugTick >= 10 || packetCount % 10 == 0) { // Her 10 paket veya 10 tick'te bir
            std::cout << "[Player " << player->id << "] Input received: flags=" << inputPacket.flags 
                      << " yaw=" << inputPacket.mouseYaw << " pitch=" << inputPacket.mousePitch 
                      << " (seq=" << sequence << ", tick=" << serverTick << ")" << std::endl;
            lastDebugTick = serverTick;
        }
    } else {
        std::cout << "[WARNING] Player " << player->id << " - InputComponent not found!" << std::endl;
    }
}

void GameServer::shutdown() {
    if (socket && socket->isOpen()) {
        socket->close();
    }
}

} // namespace game

