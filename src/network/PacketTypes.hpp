#pragma once

#include <cstdint>

namespace game::network {

/**
 * Packet Types
 * 
 * Network packet type definitions
 */
enum class PacketType : uint8_t {
    CONNECT = 0,        // Client → Server: Bağlantı isteği
    CONNECT_ACK = 1,    // Server → Client: Bağlantı onayı (entity ID gönderir)
    DISCONNECT = 2,     // Client → Server veya Server → Client: Bağlantı kesme
    HEARTBEAT = 3,      // Client ↔ Server: Bağlantı canlı tutma
    INPUT = 4,          // Client → Server: Oyuncu input'u
    SNAPSHOT = 5,       // Server → Client: Oyun durumu snapshot'ı
    INVALID = 255
};

/**
 * Packet Header
 * 
 * Her packet'in başında bulunan header
 */
struct PacketHeader {
    PacketType type;
    uint32_t sequenceNumber;  // Packet sequence (reliability için)
    uint32_t timestamp;        // Timestamp (milliseconds)
    
    static constexpr size_t SIZE = sizeof(PacketType) + sizeof(uint32_t) * 2;
};

/**
 * Packet Size Limits
 */
constexpr size_t MAX_PACKET_SIZE = 1400;  // MTU-safe
constexpr size_t MAX_PAYLOAD_SIZE = MAX_PACKET_SIZE - PacketHeader::SIZE;

} // namespace game::network

