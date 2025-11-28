#include <iostream>
#include <chrono>
#include <SFML/Graphics.hpp>
#include <LDtkLoader/Project.hpp>

#include "game/GameModel.hpp"
#include "game/GameView.hpp"
#include "game/GameController.hpp"
#include "game/GameConstants.hpp"
#include "network/Packet.hpp"
#include "network/PacketTypes.hpp"

using namespace game::client;

int main() {
    // Load LDtk project
    ldtk::Project project;
    std::string ldtk_filename = "assets/maps/map.ldtk";
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
    try {
        model.init(project);
    }
    catch (const std::exception& ex) {
        std::cerr << "ERROR: Failed to initialize game model: " << ex.what() << std::endl;
        std::cerr << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return 1;
    }

    // Create window
    sf::RenderWindow window;
    window.create(sf::VideoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT), "LDtkLoader - SFML");
    window.setFramerateLimit(Constants::WINDOW_FPS_LIMIT);

    // Network heartbeat timer
    auto lastHeartbeat = std::chrono::steady_clock::now();
    const float heartbeatInterval = Constants::HEARTBEAT_INTERVAL;

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

        // Update game state (only process input if window has focus)
        GameController::update(model, window);
        
        // Update camera
        GameView::updateCamera(model);

        // Render game
        try {
            window.clear();
            GameView::render(window, model);
            window.display();
        }
        catch (const std::exception& ex) {
            std::cerr << "ERROR during rendering: " << ex.what() << std::endl;
            std::cerr << "Press Enter to exit..." << std::endl;
            std::cin.get();
            window.close();
        }
    }
    
    // Disconnect from server
    if (model.connectedToServer) {
        model.networkClient.disconnect();
        model.networkClient.shutdown();
    }
    
    return 0;
}
