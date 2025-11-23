#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include "PacketTypes.hpp"

namespace game::network {

/**
 * Network Packet
 * 
 * Binary packet serialization/deserialization
 */
class Packet {
public:
    Packet() : readPos(0) {
        buffer.reserve(MAX_PACKET_SIZE);
    }
    
    Packet(PacketType type) : readPos(0) {
        buffer.reserve(MAX_PACKET_SIZE);
        writeHeader(type);
    }
    
    // Write operations
    void writeHeader(PacketType type) {
        writePos = 0;
        write(type);
        write(static_cast<uint32_t>(0));  // sequence (set later)
        write(static_cast<uint32_t>(0));  // timestamp (set later)
    }
    
    void setSequence(uint32_t seq) {
        *reinterpret_cast<uint32_t*>(&buffer[sizeof(PacketType)]) = seq;
    }
    
    void setTimestamp(uint32_t ts) {
        *reinterpret_cast<uint32_t*>(&buffer[sizeof(PacketType) + sizeof(uint32_t)]) = ts;
    }
    
    template<typename T>
    void write(const T& value) {
        const size_t size = sizeof(T);
        if (writePos + size > MAX_PACKET_SIZE) {
            return;  // Buffer overflow protection
        }
        
        if (writePos + size > buffer.size()) {
            buffer.resize(writePos + size);
        }
        
        std::memcpy(&buffer[writePos], &value, size);
        writePos += size;
    }
    
    void writeString(const std::string& str) {
        uint16_t len = static_cast<uint16_t>(str.length());
        write(len);
        for (char c : str) {
            write(c);
        }
    }
    
    // Read operations
    void resetRead() {
        readPos = PacketHeader::SIZE;  // Skip header
    }
    
    PacketType getType() const {
        if (buffer.size() < sizeof(PacketType)) {
            return PacketType::INVALID;
        }
        return *reinterpret_cast<const PacketType*>(&buffer[0]);
    }
    
    uint32_t getSequence() const {
        if (buffer.size() < sizeof(PacketType) + sizeof(uint32_t)) {
            return 0;
        }
        return *reinterpret_cast<const uint32_t*>(&buffer[sizeof(PacketType)]);
    }
    
    uint32_t getTimestamp() const {
        if (buffer.size() < sizeof(PacketType) + sizeof(uint32_t) * 2) {
            return 0;
        }
        return *reinterpret_cast<const uint32_t*>(&buffer[sizeof(PacketType) + sizeof(uint32_t)]);
    }
    
    template<typename T>
    bool read(T& value) {
        const size_t size = sizeof(T);
        if (readPos + size > buffer.size()) {
            return false;  // Not enough data
        }
        
        std::memcpy(&value, &buffer[readPos], size);
        readPos += size;
        return true;
    }
    
    bool readString(std::string& str) {
        uint16_t len;
        if (!read(len)) {
            return false;
        }
        
        str.clear();
        str.reserve(len);
        for (uint16_t i = 0; i < len; ++i) {
            char c;
            if (!read(c)) {
                return false;
            }
            str += c;
        }
        return true;
    }
    
    // Buffer access
    const uint8_t* getData() const { return buffer.data(); }
    uint8_t* getData() { return buffer.data(); }  // Non-const version for receiving
    size_t getSize() const { return writePos; }
    size_t getCapacity() const { return buffer.capacity(); }
    
    /**
     * Set packet data from external buffer (for receiving)
     */
    void setData(const uint8_t* data, size_t size) {
        buffer.assign(data, data + size);
        writePos = size;
        readPos = 0;
    }
    
    void clear() {
        buffer.clear();
        writePos = 0;
        readPos = 0;
    }
    
private:
    std::vector<uint8_t> buffer;
    size_t writePos = 0;
    size_t readPos = 0;
};

} // namespace game::network

