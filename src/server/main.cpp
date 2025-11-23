#include "GameServer.hpp"
#include "ServerConfig.hpp"
#include <iostream>
#include <csignal>

namespace {
    game::server::GameServer* g_server = nullptr;
    
    void signalHandler(int signal) {
        if (g_server) {
            std::cout << "\nShutting down server..." << std::endl;
            g_server->stop();
        }
    }
}

int main() {
    std::cout << "=== Game Server ===" << std::endl;
    
    // Create server
    game::server::GameServer server;
    g_server = &server;
    
    // Setup signal handlers
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    // Configure server
    game::server::ServerConfig config;
    config.port = 7777;
    config.tickRate = 60;
    config.snapshotRate = 20;
    config.maxPlayers = 128;
    
    // Initialize server
    if (!server.initialize(config)) {
        std::cerr << "Failed to initialize server" << std::endl;
        return 1;
    }
    
    // Run server (blocks until shutdown)
    server.run();
    
    // Shutdown
    server.shutdown();
    
    std::cout << "Server stopped" << std::endl;
    return 0;
}

