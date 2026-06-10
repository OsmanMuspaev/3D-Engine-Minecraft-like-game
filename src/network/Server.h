#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <memory>
#include <atomic>
#include "Packet.h"
#include "../world/World.h"

#ifndef _WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#endif

class Server {
public:
    Server();
    ~Server();

    bool start(unsigned short port = 0);
    void stop();
    bool isRunning() const { return m_running; }
    unsigned short getPort() const { return m_boundPort; }

    void setWorld(World* world, int worldSize);
    void setHostPosition(float x, float y, float z, float yaw, float pitch);
    void update(float dt);
    int getClientCount() const { return static_cast<int>(m_clients.size()); }
    std::vector<PlayerState> getClientStates() const;

private:
    struct Client {
        sf::TcpSocket socket;
        int id = 0;
        PlayerState state;
        bool connected = true;
    };

    void acceptClients();
    void handleClientPackets();
    void broadcastPositions();
    void sendWorldToClient(Client& client);
    void closeListener();

    int m_listenFd = -1;
    std::vector<std::unique_ptr<Client>> m_clients;
    std::atomic<bool> m_running = false;
    int m_nextId = 1;
    float m_broadcastTimer = 0.0f;
    unsigned short m_boundPort = 0;

    World* m_world = nullptr;
    int m_worldSize = 20;
    PlayerState m_hostState;
    bool m_hasHostState = false;
};
