#pragma once
#include <SFML/Network.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <string>
#include "Packet.h"

// TCP client for connecting to a voxel-game server.
// Runs a background thread that continuously receives packets.
class Client {
public:
    Client();
    ~Client();

    // Connect to the given host and port. Returns false on failure.
    bool connect(const std::string& host, unsigned short port = 53000);

    // Disconnect from the server.
    void disconnect();

    bool isConnected() const { return m_connected; }
    int getLocalId() const { return m_localId; }

    // --- Sending helpers ---------------------------------------------------

    void sendMove(float x, float y, float z, float yaw, float pitch);
    void sendBlockBreak(int x, int y, int z);
    void sendBlockPlace(int x, int y, int z, unsigned int blockType);
    void sendChat(const std::string& message);

    // --- Receiving (call from game loop) -----------------------------------

    // Consume buffered events produced by the receive thread.
    void update(float dt);

    // Event types produced by the receive thread.
    struct BlockUpdateEvent { int x, y, z; unsigned int type; };
    struct ChatEvent        { std::string sender; std::string message; };

    // Thread-safe accessors – copy the data out under the lock.
    std::vector<PlayerState>     getOtherPlayers();
    std::vector<BlockUpdateEvent> getBlockUpdates();
    std::vector<ChatEvent>        getChatMessages();
    bool hasWorldData() const { return m_hasWorld; }

private:
    void receiveLoop();

    sf::TcpSocket m_socket;
    std::atomic<bool> m_connected = false;
    std::thread m_receiveThread;
    int m_localId = 0;

    // Buffered data shared between the receive thread and the game thread.
    std::mutex m_mutex;
    std::vector<PlayerState>      m_otherPlayers;
    std::vector<BlockUpdateEvent> m_blockUpdates;
    std::vector<ChatEvent>        m_chatMessages;
    bool m_hasWorld = false;

    float m_sendTimer = 0.0f;
};
