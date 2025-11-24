#include <iostream>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <LDtkLoader/Project.hpp>

#include "game/GameModel.hpp"
#include "game/GameView.hpp"
#include "game/GameController.hpp"
#include "network/Packet.hpp"
#include "network/PacketTypes.hpp"

using namespace game::client;

int main() {
    // Load LDtk project
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

    // Initialize game model
    GameModel model;
    model.init(project);

    // Create window
    sf::RenderWindow window;
    window.create(sf::VideoMode(800, 500), "LDtkLoader - SFML");
    window.setFramerateLimit(60);

    // Network heartbeat timer
    auto lastHeartbeat = std::chrono::steady_clock::now();
    const float heartbeatInterval = 1.0f;  // 1 second
    
    // Debug log timer
    model.lastDebugLogTime = std::chrono::steady_clock::now();

    // Main game loop
    while(window.isOpen()) {
        // Handle events
        sf::Event event{};
        while(window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::F1) {
                    model.show_colliders = !model.show_colliders;
                }
                else if (event.key.code == sf::Keyboard::F5) {
                    // Reload the LDtk project and reinitialize the game
                    project.loadFromFile(ldtk_filename);
                    model.init(project, true);
                    std::cout << "Reloaded project " << project.getFilePath() << std::endl;
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }

        // Process network packets
        GameController::processNetwork(model);
        
        // Send heartbeat periodically
        if (model.connectedToServer) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration<float>(now - lastHeartbeat).count();
            
            if (elapsed >= heartbeatInterval && model.networkClient.isConnected()) {
                game::network::Packet heartbeat(game::network::PacketType::HEARTBEAT);
                heartbeat.setSequence(1);
                heartbeat.setTimestamp(static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()
                ).count()));
                
                model.networkClient.sendPacket(heartbeat);
                lastHeartbeat = now;
            }
        }

        // Update game state
        GameController::update(model);
        
        // Update camera
        GameView::updateCamera(model);

        // Debug log: Print positions every 5 seconds
        auto now = std::chrono::steady_clock::now();
        auto debugLogElapsed = std::chrono::duration<float>(
            now - model.lastDebugLogTime
        ).count();
        
        if (debugLogElapsed >= 5.0f) {
            std::cout << "\n=== CLIENT DEBUG LOG (Player Positions) ===" << std::endl;
            std::cout << "  Local Player Position: (" << model.player.getPosition().x 
                      << ", " << model.player.getPosition().y << ")" << std::endl;
            std::cout << "  My Entity ID: " << model.networkClient.myEntityID << std::endl;
            std::cout << "  Connected to Server: " << (model.connectedToServer ? "Yes" : "No") << std::endl;
            
            if (model.connectedToServer && model.networkClient.myEntityID != game::INVALID_ENTITY) {
                auto it = model.networkClient.remoteEntities.find(model.networkClient.myEntityID);
                if (it != model.networkClient.remoteEntities.end()) {
                    std::cout << "  Server Position (from snapshot): (" 
                              << it->second.position.x << ", " << it->second.position.y << ")" << std::endl;
                    std::cout << "  Server Entity Size: (" 
                              << it->second.size.x << ", " << it->second.size.y << ")" << std::endl;
                } else {
                    std::cout << "  Server Position: NOT FOUND in snapshot!" << std::endl;
                }
            }
            
            std::cout << "  Total Remote Entities: " << model.networkClient.remoteEntities.size() << std::endl;
            for (const auto& [entityID, remoteEntity] : model.networkClient.remoteEntities) {
                std::cout << "    Entity ID: " << entityID 
                          << " | Position: (" << remoteEntity.position.x 
                          << ", " << remoteEntity.position.y << ")" << std::endl;
            }
            std::cout << "=== END CLIENT DEBUG LOG ===\n" << std::endl;
            model.lastDebugLogTime = now;
        }

        // Render game
        window.clear();
        GameView::render(window, model);
        window.display();
    }
    
    // Disconnect from server
    if (model.connectedToServer) {
        model.networkClient.disconnect();
        model.networkClient.shutdown();
    }
    
    return 0;
}
