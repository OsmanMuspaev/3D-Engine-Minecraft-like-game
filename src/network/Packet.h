#pragma once
#include <SFML/Network.hpp>
#include <cstdint>
#include <string>
#include <vector>

enum class PacketType : uint8_t {
    PlayerMove    = 1,
    BlockBreak    = 2,
    BlockPlace    = 3,
    ChatMessage   = 4,
    JoinRequest   = 5,

    WorldData      = 10,
    BlockUpdate    = 11,
    PlayerSpawn    = 12,
    PlayerDespawn  = 13,
    PlayerPosition = 14,
    ChatBroadcast  = 15,
    JoinAccepted   = 16,
    JoinRejected   = 17,
};

struct PlayerState {
    int id = 0;
    float x = 0, y = 0, z = 0;
    float yaw = 0, pitch = 0;
    std::string name;
};

// Serialises a PacketType through sf::Packet.
sf::Packet& operator<<(sf::Packet& packet, PacketType type);
// Deserialises a PacketType from sf::Packet.
sf::Packet& operator>>(sf::Packet& packet, PacketType& type);

// Serialises a PlayerState through sf::Packet.
sf::Packet& operator<<(sf::Packet& packet, const PlayerState& state);
// Deserialises a PlayerState from sf::Packet.
sf::Packet& operator>>(sf::Packet& packet, PlayerState& state);
