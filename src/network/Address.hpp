#pragma once

#include <string>
#include <cstdint>
#include <SFML/Network/IpAddress.hpp>

namespace game::network {

/**
 * Network Address
 * 
 * IP address + Port wrapper for SFML
 */
class Address {
public:
    Address() : port(0) {}
    Address(const std::string& ip, uint16_t p) : ipAddress(ip), port(p) {}
    Address(const sf::IpAddress& ip, uint16_t p) : ipAddress(ip), port(p) {}
    
    const sf::IpAddress& getIpAddress() const { return ipAddress; }
    uint16_t getPort() const { return port; }
    
    std::string toString() const {
        return ipAddress.toString() + ":" + std::to_string(port);
    }
    
    bool operator==(const Address& other) const {
        return ipAddress == other.ipAddress && port == other.port;
    }
    
    bool operator!=(const Address& other) const {
        return !(*this == other);
    }
    
    // Hash support for use in containers
    struct Hash {
        std::size_t operator()(const Address& addr) const {
            return std::hash<std::string>{}(addr.toString());
        }
    };
    
private:
    sf::IpAddress ipAddress;
    uint16_t port;
};

} // namespace game::network

