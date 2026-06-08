#include "Server.h"
#include <iostream>

Server::Server() = default;

Server::~Server() {
    stop();
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool Server::start(unsigned short port) {
    if (m_running)
        return false;

    if (m_listener.listen(port) != sf::Socket::Status::Done) {
        std::cerr << "[Server] Failed to listen on port " << port << "\n";
        return false;
    }

    m_listener.setBlocking(false);
    m_running = true;
    std::cout << "[Server] Listening on port " << port << "\n";
    return true;
}

void Server::stop() {
    if (!m_running)
        return;

    m_running = false;

    for (auto& client : m_clients) {
        client->socket.disconnect();
    }
    m_clients.clear();
    m_listener.close();
    m_nextId = 1;

    std::cout << "[Server] Stopped.\n";
}

void Server::setWorld(World* world, int worldSize) {
    m_world = world;
    m_worldSize = worldSize;
}

// ---------------------------------------------------------------------------
// Per-frame update
// ---------------------------------------------------------------------------

void Server::update(float /*dt*/) {
    if (!m_running)
        return;

    acceptClients();
    handleClientPackets();

    // Broadcast all player positions roughly 20 times per second.
    m_broadcastTimer += 1.0f / 60.0f; // assume ~60 fps caller
    if (m_broadcastTimer >= 0.05f) {
        broadcastPositions();
        m_broadcastTimer = 0.0f;
    }
}

// ---------------------------------------------------------------------------
// Accept new TCP connections
// ---------------------------------------------------------------------------

void Server::acceptClients() {
    // Try to accept one new client per frame (non-blocking).
    auto newClient = std::make_unique<Client>();
    newClient->socket.setBlocking(false);

    if (m_listener.accept(newClient->socket) == sf::Socket::Status::Done) {
        newClient->id = m_nextId++;
        newClient->state.id = newClient->id;
        newClient->state.name = "Player" + std::to_string(newClient->id);

        std::cout << "[Server] Client " << newClient->id << " connected.\n";

        // Send join-accepted with assigned id.
        sf::Packet pkt;
        pkt << PacketType::JoinAccepted << newClient->id;
        (void)newClient->socket.send(pkt);

        // Send world data to the new client.
        sendWorldToClient(*newClient);

        m_clients.push_back(std::move(newClient));
    }
}

// ---------------------------------------------------------------------------
// Process incoming packets from all clients
// ---------------------------------------------------------------------------

void Server::handleClientPackets() {
    for (auto& client : m_clients) {
        if (!client->connected)
            continue;

        sf::Packet packet;
        sf::Socket::Status status = client->socket.receive(packet);

        if (status == sf::Socket::Status::Done) {
            PacketType type;
            packet >> type;

            switch (type) {
                case PacketType::PlayerMove: {
                    packet >> client->state;
                    client->state.id = client->id; // enforce id
                    break;
                }
                case PacketType::BlockBreak: {
                    int bx, by, bz;
                    packet >> bx >> by >> bz;
                    if (m_world) {
                        m_world->setBlock(bx, by, bz, BlockType::AIR);

                        // Broadcast the change to all clients.
                        sf::Packet bc;
                        bc << PacketType::BlockUpdate
                           << bx << by << bz
                           << static_cast<unsigned int>(BlockType::AIR);
                        for (auto& other : m_clients) {
                            if (other->connected)
                                (void)other->socket.send(bc);
                        }
                    }
                    break;
                }
                case PacketType::BlockPlace: {
                    int bx, by, bz;
                    unsigned int bt;
                    packet >> bx >> by >> bz >> bt;
                    if (m_world) {
                        m_world->setBlock(bx, by, bz,
                                           static_cast<BlockType>(bt));

                        sf::Packet bc;
                        bc << PacketType::BlockUpdate
                           << bx << by << bz << bt;
                        for (auto& other : m_clients) {
                            if (other->connected)
                                (void)other->socket.send(bc);
                        }
                    }
                    break;
                }
                case PacketType::ChatMessage: {
                    std::string msg;
                    packet >> msg;

                    sf::Packet bc;
                    bc << PacketType::ChatBroadcast
                       << client->state.name << msg;
                    for (auto& other : m_clients) {
                        if (other->connected)
                            (void)other->socket.send(bc);
                    }
                    break;
                }
                default:
                    break;
            }
        } else if (status == sf::Socket::Status::Disconnected) {
            client->connected = false;
            std::cout << "[Server] Client " << client->id
                      << " disconnected.\n";

            // Notify everyone.
            sf::Packet bc;
            bc << PacketType::PlayerDespawn << client->id;
            for (auto& other : m_clients) {
                if (other->connected)
                    (void)other->socket.send(bc);
            }
        }
        // Disconnected sockets are cleaned up in the next pruning pass.
    }

    // Remove disconnected clients.
    m_clients.erase(
        std::remove_if(m_clients.begin(), m_clients.end(),
                        [](const std::unique_ptr<Client>& c) {
                            return !c->connected;
                        }),
        m_clients.end());
}

// ---------------------------------------------------------------------------
// Broadcast every connected player's position to every other client
// ---------------------------------------------------------------------------

void Server::broadcastPositions() {
    for (auto& client : m_clients) {
        if (!client->connected)
            continue;

        // Tell this client about every *other* player.
        for (auto& other : m_clients) {
            if (other->id == client->id || !other->connected)
                continue;

            sf::Packet pkt;
            pkt << PacketType::PlayerPosition << other->state;
            (void)client->socket.send(pkt);
        }
    }
}

// ---------------------------------------------------------------------------
// Send the full world block data to a newly-connected client
// ---------------------------------------------------------------------------

void Server::sendWorldToClient(Client& client) {
    if (!m_world)
        return;

    int half = m_worldSize / 2;

    // WorldData packet layout:
    //   PacketType, int worldSize
    //   then worldSize^2 columns * worldHeight blocks
    //   Each block: int x, int y, int z, unsigned int blockType

    sf::Packet pkt;
    pkt << PacketType::WorldData << m_worldSize;

    constexpr int WORLD_HEIGHT = 256;

    for (int x = -half; x < half; ++x) {
        for (int z = -half; z < half; ++z) {
            for (int y = 0; y < WORLD_HEIGHT; ++y) {
                Block block = m_world->getBlock(x, y, z);
                if (block.type == BlockType::AIR)
                    continue;

                pkt << x << y << z
                    << static_cast<unsigned int>(block.type);
            }
        }
    }

    (void)client.socket.send(pkt);
    std::cout << "[Server] Sent world data to client " << client.id << ".\n";
}
