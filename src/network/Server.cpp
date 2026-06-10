#include "Server.h"
#include <iostream>
#include <algorithm>

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#endif

class RawSocket : public sf::TcpSocket {
public:
    using sf::TcpSocket::create;
};

Server::Server() = default;

Server::~Server() {
    stop();
}

void Server::closeListener() {
    if (m_listenFd >= 0) {
        struct linger sl{};
        sl.l_onoff = 1;
        sl.l_linger = 0;
        setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl));
        ::close(m_listenFd);
        m_listenFd = -1;
    }
}

bool Server::start(unsigned short port) {
    if (m_running)
        return false;

    closeListener();

    m_listenFd = static_cast<int>(::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (m_listenFd < 0) {
        std::cerr << "[Server] Failed to create socket\n";
        return false;
    }

    int opt = 1;
    setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(m_listenFd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        int err = errno;
        std::cerr << "[Server] Failed to bind to port " << port << " (errno " << err << ")\n";
        closeListener();
        return false;
    }

    socklen_t addrLen = sizeof(addr);
    getsockname(m_listenFd, (struct sockaddr*)&addr, &addrLen);
    m_boundPort = ntohs(addr.sin_port);

    if (::listen(m_listenFd, 8) < 0) {
        std::cerr << "[Server] Failed to listen on port " << m_boundPort << "\n";
        closeListener();
        return false;
    }

#ifndef _WIN32
    fcntl(m_listenFd, F_SETFL, O_NONBLOCK);
#else
    u_long nonblock = 1;
    ioctlsocket(m_listenFd, FIONBIO, &nonblock);
#endif

    m_running = true;
    std::cout << "[Server] Listening on port " << m_boundPort << "\n";
    return true;
}

void Server::stop() {
    if (!m_running && m_listenFd < 0)
        return;

    m_running = false;

    for (auto& client : m_clients) {
        client->socket.disconnect();
    }
    m_clients.clear();

    closeListener();

    m_nextId = 1;
    std::cout << "[Server] Stopped.\n";
}

void Server::setWorld(World* world, int worldSize) {
    m_world = world;
    m_worldSize = worldSize;
}

void Server::update(float /*dt*/) {
    if (!m_running)
        return;

    acceptClients();
    handleClientPackets();

    m_broadcastTimer += 1.0f / 60.0f;
    if (m_broadcastTimer >= 0.05f) {
        broadcastPositions();
        m_broadcastTimer = 0.0f;
    }
}

void Server::acceptClients() {
    struct sockaddr_in clientAddr{};
    socklen_t addrLen = sizeof(clientAddr);

    int clientFd = static_cast<int>(::accept(m_listenFd, (struct sockaddr*)&clientAddr, &addrLen));
    if (clientFd < 0)
        return;

    auto client = std::make_unique<Client>();
    client->socket.setBlocking(false);

    auto handle = static_cast<sf::SocketHandle>(clientFd);
    static_cast<RawSocket&>(client->socket).create(handle);

    client->id = m_nextId++;
    client->state.id = client->id;
    client->state.name = "Player" + std::to_string(client->id);

    std::cout << "[Server] Client " << client->id << " connected.\n";

    sf::Packet pkt;
    pkt << PacketType::JoinAccepted << client->id;
    (void)client->socket.send(pkt);

    sendWorldToClient(*client);

    m_clients.push_back(std::move(client));
}

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
                    client->state.id = client->id;
                    break;
                }
                case PacketType::BlockBreak: {
                    int bx, by, bz;
                    packet >> bx >> by >> bz;
                    if (m_world) {
                        m_world->setBlock(bx, by, bz, BlockType::AIR);
                        sf::Packet bc;
                        bc << PacketType::BlockUpdate << bx << by << bz
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
                        m_world->setBlock(bx, by, bz, static_cast<BlockType>(bt));
                        sf::Packet bc;
                        bc << PacketType::BlockUpdate << bx << by << bz << bt;
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
                    bc << PacketType::ChatBroadcast << client->state.name << msg;
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
            std::cout << "[Server] Client " << client->id << " disconnected.\n";
            sf::Packet bc;
            bc << PacketType::PlayerDespawn << client->id;
            for (auto& other : m_clients) {
                if (other->connected)
                    (void)other->socket.send(bc);
            }
        }
    }

    m_clients.erase(
        std::remove_if(m_clients.begin(), m_clients.end(),
                        [](const std::unique_ptr<Client>& c) { return !c->connected; }),
        m_clients.end());
}

void Server::broadcastPositions() {
    for (auto& client : m_clients) {
        if (!client->connected)
            continue;
        for (auto& other : m_clients) {
            if (other->id == client->id || !other->connected)
                continue;
            sf::Packet pkt;
            pkt << PacketType::PlayerPosition << other->state;
            (void)client->socket.send(pkt);
        }
    }
}

void Server::sendWorldToClient(Client& client) {
    if (!m_world)
        return;

    int half = m_worldSize / 2;

    sf::Packet pkt;
    pkt << PacketType::WorldData << m_worldSize;

    constexpr int WORLD_HEIGHT = 256;

    for (int x = -half; x < half; ++x) {
        for (int z = -half; z < half; ++z) {
            for (int y = 0; y < WORLD_HEIGHT; ++y) {
                Block block = m_world->getBlock(x, y, z);
                if (block.type == BlockType::AIR)
                    continue;
                pkt << x << y << z << static_cast<unsigned int>(block.type);
            }
        }
    }

    (void)client.socket.send(pkt);
    std::cout << "[Server] Sent world data to client " << client.id << ".\n";
}
