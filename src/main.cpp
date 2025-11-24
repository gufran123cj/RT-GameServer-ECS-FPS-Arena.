#include <iostream>
#include <chrono>
#include <thread>
#include <map>

#include <SFML/Graphics.hpp>
#include <LDtkLoader/Project.hpp>

#include "TileMap.hpp"
#include "client/ClientNetworkManager.hpp"
#include "network/Packet.hpp"
#include "network/PacketTypes.hpp"
#include "core/components/PositionComponent.hpp"
#include "core/components/SpriteComponent.hpp"


auto getPlayerCollider(sf::Shape& player) -> sf::FloatRect {
    auto bounds = player.getGlobalBounds();
    sf::FloatRect rect;
    rect.left = bounds.left;
    rect.width = bounds.width;
    rect.top = bounds.top + bounds.height / 2;
    rect.height = bounds.height / 2;
    return rect;
}

auto getColliderShape(const sf::FloatRect& rect) -> sf::RectangleShape {
    sf::RectangleShape r;
    r.setSize({rect.width, rect.height});
    r.setPosition(rect.left, rect.top);
    r.setFillColor({200, 0, 0, 95});
    return r;
}

// Network client for Game
class GameClient : public game::client::ClientNetworkManager {
public:
    void onConnectAck(game::core::Entity::ID entityID) override {
        std::cout << "✅ Connected to server! Entity ID: " << entityID << std::endl;
        myEntityID = entityID;
        // Also update base class entityID for consistency
        this->entityID = entityID;
    }
    
    void onSnapshot(game::network::Packet& packet) override {
        packet.resetRead();
        
        uint32_t entityCount;
        if (!packet.read(entityCount)) {
            return;
        }
        
        // Clear old entities
        remoteEntities.clear();
        
        // Read entities from snapshot
        for (uint32_t i = 0; i < entityCount; ++i) {
            game::core::Entity::ID entityID;
            if (!packet.read(entityID)) break;
            
            float posX, posY;
            if (!packet.read(posX) || !packet.read(posY)) break;
            
            float sizeX, sizeY;
            if (!packet.read(sizeX) || !packet.read(sizeY)) break;
            
            uint8_t r, g, b, a;
            if (!packet.read(r) || !packet.read(g) || !packet.read(b) || !packet.read(a)) break;
            
            RemoteEntity entity;
            entity.position = sf::Vector2f(posX, posY);
            entity.size = sf::Vector2f(sizeX, sizeY);
            entity.color = sf::Color(r, g, b, a);
            
            remoteEntities[entityID] = entity;
        }
    }
    
    void onDisconnect() override {
        std::cout << "Disconnected from server" << std::endl;
        remoteEntities.clear();
        myEntityID = 0;
    }
    
    game::core::Entity::ID myEntityID = 0;
    
    struct RemoteEntity {
        sf::Vector2f position;
        sf::Vector2f size;
        sf::Color color;
    };
    
    std::map<game::core::Entity::ID, RemoteEntity> remoteEntities;
};

struct Game {
    TileMap tilemap;
    sf::RectangleShape player;

    std::vector<sf::FloatRect> colliders;
    bool show_colliders = false;

    sf::View camera;
    sf::FloatRect camera_bounds;
    
    // Network
    GameClient networkClient;
    bool connectedToServer = false;
    std::string serverIp = "127.0.0.1";
    uint16_t serverPort = 7777;
    sf::Vector2f initialPlayerPosition;  // LDtk'den yüklenen initial pozisyon
    
    // Debug timing
    std::chrono::steady_clock::time_point lastDebugLogTime;

    void init(const ldtk::Project& ldtk, bool reloading = false) {
        // get the world from the project
        auto& world = ldtk.getWorld();

        // get first level from the world
        auto& ldtk_level0 = world.getLevel("Level_0");

        // load the TileMap from the level
        TileMap::path = ldtk.getFilePath().directory();
        tilemap.load(ldtk_level0);
        
        // Initialize network client
        if (!reloading) {
            if (networkClient.initialize()) {
                // Connect with initial player position from LDtk
                if (networkClient.connect(serverIp, serverPort, initialPlayerPosition)) {
                    connectedToServer = true;
                    std::cout << "Connecting to server " << serverIp << ":" << serverPort 
                              << " with initial position (" << initialPlayerPosition.x 
                              << ", " << initialPlayerPosition.y << ")..." << std::endl;
                } else {
                    std::cerr << "Failed to connect to server" << std::endl;
                }
            } else {
                std::cerr << "Failed to initialize network client" << std::endl;
            }
        }

        // get Entities layer from level_0
        auto& entities_layer = ldtk_level0.getLayer("Entities");

        // retrieve collider entities from entities layer and store them in the colliders vector
        colliders.clear();
        for (ldtk::Entity& col : entities_layer.getEntitiesByName("Collider")) {
            colliders.emplace_back(
                (float)col.getPosition().x, (float)col.getPosition().y,
                (float)col.getSize().x, (float)col.getSize().y
            );
        }

        // get the Player entity, and its 'color' field
        auto& player_ent = entities_layer.getEntitiesByName("Player")[0].get();
        auto& player_color = player_ent.getField<ldtk::Color>("color").value();

        // initialize player shape
        player.setSize({8, 16});
        player.setOrigin(4, 16);
        if (!reloading) {
            initialPlayerPosition = sf::Vector2f(
                static_cast<float>(player_ent.getPosition().x + 8),
                static_cast<float>(player_ent.getPosition().y + 16)
            );
            player.setPosition(initialPlayerPosition);
        }
        player.setFillColor({player_color.r, player_color.g, player_color.b});

        // create camera view
        camera.setSize({400, 250});
        camera.zoom(0.55f);
        camera.setCenter(player.getPosition());
        camera_bounds.left = 0;
        camera_bounds.top = 0;
        camera_bounds.width = static_cast<float>(ldtk_level0.size.x);
        camera_bounds.height = static_cast<float>(ldtk_level0.size.y);
    }

    void update() {
        // Calculate input velocity from keyboard
        float velX = 0.0f, velY = 0.0f;
        const float moveSpeed = 60.0f;  // pixels per second (server will use this in fixed timestep)
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            velY = -moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            velY = moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            velX = -moveSpeed;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            velX = moveSpeed;
        
        // Send input to server if connected
        if (connectedToServer && networkClient.isConnected()) {
            game::network::Packet inputPacket(game::network::PacketType::INPUT);
            inputPacket.setSequence(1);
            inputPacket.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()));
            inputPacket.write(velX);
            inputPacket.write(velY);
            networkClient.sendPacket(inputPacket);
        }
        
        // Update player position from server snapshot (if connected)
        if (connectedToServer && networkClient.myEntityID != 0) {
            auto it = networkClient.remoteEntities.find(networkClient.myEntityID);
            if (it != networkClient.remoteEntities.end()) {
                // Server'dan gelen pozisyonu kullan
                player.setPosition(it->second.position);
            }
            // Eğer snapshot henüz gelmemişse, player pozisyonu değişmez (local pozisyon kalır)
        }

        // update camera (follow player position from server)
        camera.move((player.getPosition() - camera.getCenter())/5.f);

        auto cam_size = camera.getSize();

        {
            auto cam_pos = camera.getCenter();
            // check for camera X limit
            if (cam_pos.x - cam_size.x / 2 < camera_bounds.left) {
                camera.setCenter(camera_bounds.left + cam_size.x / 2, cam_pos.y);
            }
            else if (cam_pos.x + cam_size.x / 2 > camera_bounds.left + camera_bounds.width) {
                camera.setCenter(camera_bounds.left + camera_bounds.width - cam_size.x / 2, cam_pos.y);
            }
        }

        {
            auto cam_pos = camera.getCenter();
            // check for camera Y limit
            if (cam_pos.y - cam_size.y / 2 < camera_bounds.top) {
                camera.setCenter(cam_pos.x, camera_bounds.top + cam_size.y / 2);
            }
            else if (cam_pos.y + cam_size.y / 2 > camera_bounds.top + camera_bounds.height) {
                camera.setCenter(cam_pos.x, camera_bounds.top + camera_bounds.height - cam_size.y / 2);
            }
        }
    }

    void render(sf::RenderTarget& target) {
        target.setView(camera);

        // draw map background layers
        target.draw(tilemap.getLayer("Ground"));
        target.draw(tilemap.getLayer("Trees"));
        if (show_colliders) {
            // draw map colliders
            for (auto& rect : colliders)
                target.draw(getColliderShape(rect));
        }

        // Draw local player (always visible, position updated from server if connected)
        target.draw(player);
        
        // Draw other players from server snapshot (excluding ourselves)
        if (connectedToServer && !networkClient.remoteEntities.empty()) {
            for (const auto& [entityID, remoteEntity] : networkClient.remoteEntities) {
                // Skip our own entity (already drawn above)
                if (entityID == networkClient.myEntityID && networkClient.myEntityID != 0) {
                    continue;
                }
                
                // Draw other players (remote entities)
                sf::RectangleShape remotePlayer;
                remotePlayer.setSize(remoteEntity.size);
                remotePlayer.setPosition(remoteEntity.position);
                remotePlayer.setFillColor(remoteEntity.color);
                // Set origin to match local player (bottom-center)
                remotePlayer.setOrigin(remoteEntity.size.x * 0.5f, remoteEntity.size.y);
                target.draw(remotePlayer);
            }
        }
        if (show_colliders) {
            // draw player collider
            target.draw(getColliderShape(getPlayerCollider(player)));
        }

        // draw map top layer
        target.draw(tilemap.getLayer("Trees_top"));
    }
};

int main() {
    ldtk::Project project;
    std::string ldtk_filename = "assets/maps/world1.ldtk";
    try {
        project.loadFromFile(ldtk_filename);
        std::cout << "LDtk World \"" << project.getFilePath() << "\" was loaded successfully." << std::endl;
    }
    catch (std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }

    // initialize the game from the LDtk project data
    Game game;
    game.init(project);

    // create the window
    sf::RenderWindow window;
    window.create(sf::VideoMode(800, 500), "LDtkLoader - SFML");
    window.setFramerateLimit(60);

    // Network heartbeat timer
    auto lastHeartbeat = std::chrono::steady_clock::now();
    const float heartbeatInterval = 1.0f;  // 1 second
    
    // Debug log timer
    game.lastDebugLogTime = std::chrono::steady_clock::now();

    // start game loop
    while(window.isOpen()) {

        // EVENTS
        sf::Event event{};
        while(window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::F1)
                    game.show_colliders = !game.show_colliders;
                else if (event.key.code == sf::Keyboard::F5) {
                    // reload the LDtk project and reinitialize the game
                    project.loadFromFile(ldtk_filename);
                    game.init(project, true);
                    std::cout << "Reloaded project " << project.getFilePath() << std::endl;
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // Process network packets
        if (game.connectedToServer) {
            game.networkClient.processPackets();
            
            // Send heartbeat periodically
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration<float>(now - lastHeartbeat).count();
            
            if (elapsed >= heartbeatInterval && game.networkClient.isConnected()) {
                game::network::Packet heartbeat(game::network::PacketType::HEARTBEAT);
                heartbeat.setSequence(1);
                heartbeat.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()
                ).count()));
                
                game.networkClient.sendPacket(heartbeat);
                lastHeartbeat = now;
            }
        }

        // UPDATE
        game.update();

        // Debug log: Print positions every 5 seconds
        auto now = std::chrono::steady_clock::now();
        auto debugLogElapsed = std::chrono::duration<float>(
            now - game.lastDebugLogTime
        ).count();
        
        if (debugLogElapsed >= 5.0f) {
            std::cout << "\n=== CLIENT DEBUG LOG (Player Positions) ===" << std::endl;
            std::cout << "  Local Player Position: (" << game.player.getPosition().x 
                      << ", " << game.player.getPosition().y << ")" << std::endl;
            std::cout << "  My Entity ID: " << game.networkClient.myEntityID << std::endl;
            std::cout << "  Connected to Server: " << (game.connectedToServer ? "Yes" : "No") << std::endl;
            
            if (game.connectedToServer && game.networkClient.myEntityID != 0) {
                auto it = game.networkClient.remoteEntities.find(game.networkClient.myEntityID);
                if (it != game.networkClient.remoteEntities.end()) {
                    std::cout << "  Server Position (from snapshot): (" 
                              << it->second.position.x << ", " << it->second.position.y << ")" << std::endl;
                    std::cout << "  Server Entity Size: (" 
                              << it->second.size.x << ", " << it->second.size.y << ")" << std::endl;
                } else {
                    std::cout << "  Server Position: NOT FOUND in snapshot!" << std::endl;
                }
            }
            
            std::cout << "  Total Remote Entities: " << game.networkClient.remoteEntities.size() << std::endl;
            for (const auto& [entityID, remoteEntity] : game.networkClient.remoteEntities) {
                std::cout << "    Entity ID: " << entityID 
                          << " | Position: (" << remoteEntity.position.x 
                          << ", " << remoteEntity.position.y << ")" << std::endl;
            }
            std::cout << "=== END CLIENT DEBUG LOG ===\n" << std::endl;
            game.lastDebugLogTime = now;
        }

        // RENDER
        window.clear();
        game.render(window);
        window.display();

    }
    
    // Disconnect from server
    if (game.connectedToServer) {
        game.networkClient.disconnect();
        game.networkClient.shutdown();
    }
    return 0;
}
