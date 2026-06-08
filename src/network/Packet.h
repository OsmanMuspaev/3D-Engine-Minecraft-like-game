#pragma once
#include <SFML/Network.hpp>
#include <cstdint>
#include <string>
#include <vector>

// Identifies the type of each network packet exchanged between client and server.
enum class PacketType : uint8_t {
    // Client -> Server
    PlayerMove    = 1,
    BlockBreak    = 2,
    BlockPlace    = 3,
    ChatMessage   = 4,
    JoinRequest   = 5,

    // Server -> Client
    WorldData      = 10,
    BlockUpdate    = 11,
    PlayerSpawn    = 12,
    PlayerDespawn  = 13,
    PlayerPosition = 14,
    ChatBroadcast  = 15,
    JoinAccepted   = 16,
    JoinRejected   = 17,
};

// Shared representation of a remote player's state.
struct PlayerState {
    int id = 0;
    float x = 0, y = 0, z = 0;
    float yaw = 0, pitch = 0;
    std::string name;
};

// Serialise / deserialise PacketType through sf::Packet.
sf::Packet& operator<<(sf::Packet& packet, PacketType type);
sf::Packet& operator>>(sf::Packet& packet, PacketType& type);

// Serialise / deserialise PlayerState through sf::Packet.
sf::Packet& operator<<(sf::Packet& packet, const PlayerState& state);
sf::Packet& operator>>(sf::Packet& packet, PlayerState& state);
