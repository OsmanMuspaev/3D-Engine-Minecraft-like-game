#include "Client.h"
#include <iostream>
#include <algorithm>

Client::Client() = default;

Client::~Client() {
    disconnect();
}

// Connects to the given host and port, spawning a background receive thread.
bool Client::connect(const std::string& host, unsigned short port) {
    if (m_connected)
        return false;

    m_socket.setBlocking(true);

    auto address = sf::IpAddress::resolve(host);
    if (!address || m_socket.connect(*address, port) != sf::Socket::Status::Done) {
        std::cerr << "[Client] Failed to connect to " << host
                  << ":" << port << "\n";
        return false;
    }

    m_connected = true;
    std::cout << "[Client] Connected to " << host << ":" << port << "\n";

    m_receiveThread = std::thread(&Client::receiveLoop, this);

    return true;
}

// Disconnects from the server and clears all buffered data.
void Client::disconnect() {
    if (!m_connected)
        return;

    m_connected = false;
    m_socket.disconnect();

    if (m_receiveThread.joinable())
        m_receiveThread.join();

    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_otherPlayers.clear();
        m_blockUpdates.clear();
        m_chatMessages.clear();
        m_hasWorld = false;
    }

    std::cout << "[Client] Disconnected.\n";
}

// Sends the local player's position and orientation to the server.
void Client::sendMove(float x, float y, float z, float yaw, float pitch) {
    if (!m_connected)
        return;

    sf::Packet pkt;
    PlayerState state;
    state.id = m_localId;
    state.x = x;
    state.y = y;
    state.z = z;
    state.yaw = yaw;
    state.pitch = pitch;

    pkt << PacketType::PlayerMove << state;
    (void)m_socket.send(pkt);
}

// Sends a block break request to the server.
void Client::sendBlockBreak(int x, int y, int z) {
    if (!m_connected)
        return;

    sf::Packet pkt;
    pkt << PacketType::BlockBreak << x << y << z;
    (void)m_socket.send(pkt);
}

// Sends a block place request to the server.
void Client::sendBlockPlace(int x, int y, int z, unsigned int blockType) {
    if (!m_connected)
        return;

    sf::Packet pkt;
    pkt << PacketType::BlockPlace << x << y << z << blockType;
    (void)m_socket.send(pkt);
}

// Sends a chat message to the server.
void Client::sendChat(const std::string& message) {
    if (!m_connected)
        return;

    sf::Packet pkt;
    pkt << PacketType::ChatMessage << message;
    (void)m_socket.send(pkt);
}

// Swaps out buffered events so the caller can process them without holding the lock.
void Client::update(float dt) {
    if (!m_connected)
        return;

    m_sendTimer += dt;
    if (m_sendTimer >= 0.05f) {
        m_sendTimer = 0.0f;
    }
}

// Returns a copy of the other players list under the mutex lock.
std::vector<PlayerState> Client::getOtherPlayers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_otherPlayers;
}

// Moves block update events out so the caller owns them.
std::vector<Client::BlockUpdateEvent> Client::getBlockUpdates() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto out = std::move(m_blockUpdates);
    m_blockUpdates.clear();
    return out;
}

// Moves chat messages out so the caller owns them.
std::vector<Client::ChatEvent> Client::getChatMessages() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto out = std::move(m_chatMessages);
    m_chatMessages.clear();
    return out;
}

// Background receive loop that runs on its own thread until disconnection.
void Client::receiveLoop() {
    while (m_connected) {
        sf::Packet packet;
        sf::Socket::Status status = m_socket.receive(packet);

        if (status == sf::Socket::Status::Done) {
            PacketType type;
            packet >> type;

            switch (type) {
                case PacketType::JoinAccepted: {
                    packet >> m_localId;
                    std::cout << "[Client] Join accepted. Local id = "
                              << m_localId << "\n";
                    break;
                }
                case PacketType::JoinRejected: {
                    std::cout << "[Client] Join rejected by server.\n";
                    m_connected = false;
                    return;
                }
                case PacketType::WorldData: {
                    int worldSize;
                    packet >> worldSize;
                    std::lock_guard<std::mutex> lock(m_mutex);
                    while (!packet.endOfPacket()) {
                        BlockUpdateEvent ev;
                        packet >> ev.x >> ev.y >> ev.z >> ev.type;
                        m_blockUpdates.push_back(ev);
                    }
                    m_hasWorld = true;
                    std::cout << "[Client] Received world data (" << m_blockUpdates.size() << " blocks)\n";
                    break;
                }
                case PacketType::BlockUpdate: {
                    BlockUpdateEvent ev;
                    packet >> ev.x >> ev.y >> ev.z >> ev.type;
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_blockUpdates.push_back(ev);
                    break;
                }
                case PacketType::PlayerSpawn: {
                    PlayerState ps;
                    packet >> ps;
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_otherPlayers.push_back(ps);
                    break;
                }
                case PacketType::PlayerDespawn: {
                    int id;
                    packet >> id;
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_otherPlayers.erase(
                        std::remove_if(m_otherPlayers.begin(),
                                       m_otherPlayers.end(),
                                       [id](const PlayerState& p) {
                                           return p.id == id;
                                       }),
                        m_otherPlayers.end());
                    break;
                }
                case PacketType::PlayerPosition: {
                    PlayerState ps;
                    packet >> ps;
                    std::lock_guard<std::mutex> lock(m_mutex);
                    bool found = false;
                    for (auto& existing : m_otherPlayers) {
                        if (existing.id == ps.id) {
                            existing = ps;
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                        m_otherPlayers.push_back(ps);
                    break;
                }
                case PacketType::ChatBroadcast: {
                    ChatEvent ev;
                    packet >> ev.sender >> ev.message;
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_chatMessages.push_back(std::move(ev));
                    break;
                }
                default:
                    break;
            }
        } else if (status == sf::Socket::Status::Disconnected) {
            std::cout << "[Client] Server closed the connection.\n";
            m_connected = false;
            return;
        }
    }
}
