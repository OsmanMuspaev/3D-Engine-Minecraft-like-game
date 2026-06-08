#include "Client.h"
#include <iostream>

Client::Client() = default;

Client::~Client() {
    disconnect();
}

// ---------------------------------------------------------------------------
// Connection
// ---------------------------------------------------------------------------

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

    // Spawn the background receive thread.
    m_receiveThread = std::thread(&Client::receiveLoop, this);

    return true;
}

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

// ---------------------------------------------------------------------------
// Sending
// ---------------------------------------------------------------------------

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

void Client::sendBlockBreak(int x, int y, int z) {
    if (!m_connected)
        return;

    sf::Packet pkt;
    pkt << PacketType::BlockBreak << x << y << z;
    (void)m_socket.send(pkt);
}

void Client::sendBlockPlace(int x, int y, int z, unsigned int blockType) {
    if (!m_connected)
        return;

    sf::Packet pkt;
    pkt << PacketType::BlockPlace << x << y << z << blockType;
    (void)m_socket.send(pkt);
}

void Client::sendChat(const std::string& message) {
    if (!m_connected)
        return;

    sf::Packet pkt;
    pkt << PacketType::ChatMessage << message;
    (void)m_socket.send(pkt);
}

// ---------------------------------------------------------------------------
// Game-loop update – swap out the buffered events so the caller can process
// them without holding the lock for the rest of the frame.
// ---------------------------------------------------------------------------

void Client::update(float dt) {
    if (!m_connected)
        return;

    // Send our own position periodically (every ~50 ms ≈ 20 Hz).
    m_sendTimer += dt;
    if (m_sendTimer >= 0.05f) {
        // The game code should call sendMove() directly with the current
        // camera position; this is just a fallback tick.
        m_sendTimer = 0.0f;
    }

    // Events are consumed by the game code through the get*() accessors.
    // Nothing extra to do here – the receive thread fills the buffers.
}

// ---------------------------------------------------------------------------
// Thread-safe accessors (copy under lock)
// ---------------------------------------------------------------------------

std::vector<PlayerState> Client::getOtherPlayers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_otherPlayers;
}

std::vector<Client::BlockUpdateEvent> Client::getBlockUpdates() {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Move the events out so the caller owns them and we don't accumulate.
    auto out = std::move(m_blockUpdates);
    m_blockUpdates.clear();
    return out;
}

std::vector<Client::ChatEvent> Client::getChatMessages() {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto out = std::move(m_chatMessages);
    m_chatMessages.clear();
    return out;
}

// ---------------------------------------------------------------------------
// Background receive loop – runs on its own thread until disconnection.
// ---------------------------------------------------------------------------

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
                    // Block data follows (x, y, z, blockType) for every
                    // non-air block. The game code should apply these once
                    // the flag hasWorldData becomes true.
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_hasWorld = true;
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
                    // Update existing player or add new one.
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
        // Error or NotReady – just retry next iteration.
    }
}
