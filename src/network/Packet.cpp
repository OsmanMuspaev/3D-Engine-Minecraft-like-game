#include "Packet.h"

sf::Packet& operator<<(sf::Packet& packet, PacketType type) {
    return packet << static_cast<uint8_t>(type);
}

sf::Packet& operator>>(sf::Packet& packet, PacketType& type) {
    uint8_t raw;
    packet >> raw;
    type = static_cast<PacketType>(raw);
    return packet;
}

sf::Packet& operator<<(sf::Packet& packet, const PlayerState& state) {
    packet << state.id
           << state.x << state.y << state.z
           << state.yaw << state.pitch
           << state.name;
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, PlayerState& state) {
    packet >> state.id
           >> state.x >> state.y >> state.z
           >> state.yaw >> state.pitch
           >> state.name;
    return packet;
}
