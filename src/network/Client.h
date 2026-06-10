#pragma once
#include <SFML/Network.hpp>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <string>
#include "Packet.h"

class Client {
public:
    Client();
    ~Client();

    // Connects to the given host and port. Returns false on failure.
    bool connect(const std::string& host, unsigned short port = 53000);

    // Disconnects from the server and clears all buffered data.
    void disconnect();

    bool isConnected() const { return m_connected; }
    int getLocalId() const { return m_localId; }

    void sendMove(float x, float y, float z, float yaw, float pitch);
    void sendBlockBreak(int x, int y, int z);
    void sendBlockPlace(int x, int y, int z, unsigned int blockType);
    void sendChat(const std::string& message);

    // Swaps out buffered events so the caller can process them.
    void update(float dt);

    struct BlockUpdateEvent { int x, y, z; unsigned int type; };
    struct ChatEvent        { std::string sender; std::string message; };

    // Returns a copy of the other players list under the mutex lock.
    std::vector<PlayerState>     getOtherPlayers();
    // Moves block update events out so the caller owns them.
    std::vector<BlockUpdateEvent> getBlockUpdates();
    // Moves chat messages out so the caller owns them.
    std::vector<ChatEvent>        getChatMessages();
    bool hasWorldData() const { return m_hasWorld; }

private:
    // Background receive loop that runs on its own thread until disconnection.
    void receiveLoop();

    sf::TcpSocket m_socket;
    std::atomic<bool> m_connected = false;
    std::thread m_receiveThread;
    int m_localId = 0;

    std::mutex m_mutex;
    std::vector<PlayerState>      m_otherPlayers;
    std::vector<BlockUpdateEvent> m_blockUpdates;
    std::vector<ChatEvent>        m_chatMessages;
    bool m_hasWorld = false;

    float m_sendTimer = 0.0f;
};
