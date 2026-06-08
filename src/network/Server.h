#pragma once
#include <SFML/Network.hpp>
#include <vector>
#include <memory>
#include <atomic>
#include "Packet.h"
#include "../world/World.h"

// Simple blocking TCP server for a voxel game.
// Accepts clients, relays player positions, and sends world data on join.
class Server {
public:
    Server();
    ~Server();

    // Start listening on the given port. Returns false on failure.
    bool start(unsigned short port = 53000);

    // Shut down the server and disconnect every client.
    void stop();

    bool isRunning() const { return m_running; }

    // Provide a pointer to the authoritative world so the server can
    // read / write blocks and send world data to newly-joined clients.
    void setWorld(World* world, int worldSize);

    // Drive the server from the game loop. Call once per frame.
    void update(float dt);

    int getClientCount() const { return static_cast<int>(m_clients.size()); }

private:
    // Represents one connected client.
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

    sf::TcpListener m_listener;
    std::vector<std::unique_ptr<Client>> m_clients;
    std::atomic<bool> m_running = false;
    int m_nextId = 1;
    float m_broadcastTimer = 0.0f;

    World* m_world = nullptr;
    int m_worldSize = 20;
};
