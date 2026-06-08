#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include "Camera.h"
#include "world/World.h"
#include "world/TextureManager.h"
#include "world/Inventory.h"
#include "world/InventoryUI.h"
#include "world/PlayerInteraction.h"
#include "world/Item.h"
#include "world/PlayerView.h"
#include "world/CraftingSystem.h"
#include "renderer/Renderer.h"
#include "world/BlockRegistry.h"
#include "menu/MenuManager.h"
#include "network/Server.h"
#include "network/Client.h"

class Game {
public:
    enum class CameraView {
        FirstPerson,
        ThirdPerson,
        FrontView
    };

    Game(unsigned int windowWidth, unsigned int windowHeight);
    ~Game();

    void run();

private:
    void init();
    void handleEvents();
    void handleMenuEvents();
    void update(float dt);
    void render(float dt);
    void renderMenu();
    void cleanup();

    // Inventory helpers.
    void addDefaultInventory();

    // Game state management.
    void startNewWorld(const std::string& name, int worldSize);
    void loadExistingWorld(const std::string& name);
    void connectToServer(const std::string& address);
    void startServer(bool useTunnel);
    void saveCurrentWorld();
    void resetGame();

    // Networking helpers.
    static std::string getLocalIP();
    void launchLocalTunnel(unsigned short port);

    unsigned int m_windowWidth;
    unsigned int m_windowHeight;
    float m_renderScale = 0.25f;

    sf::RenderWindow m_window;
    std::unique_ptr<Renderer> m_renderer;

    // Core subsystems.
    TextureManager m_texMgr;
    World m_world;
    Camera m_camera;
    Inventory m_inventory;
    InventoryUI m_invUI;
    PlayerView m_playerView;
    CraftingSystem m_crafting;
    PlayerInteraction m_playerInteraction;

    // Menu system.
    MenuManager m_menu;

    // Networking.
    Server m_server;
    Client m_client;
    bool m_isServer = false;
    bool m_isClient = false;
    float m_networkSendTimer = 0.0f;
    std::string m_currentWorldName;

    // LocalTunnel process.
    std::thread m_tunnelThread;
    std::atomic<bool> m_tunnelRunning{false};
    std::string m_tunnelUrl;

    // HUD text.
    sf::Font m_font;
    sf::Text m_uiText;
    bool m_fontLoaded = false;

    // Timing.
    sf::Clock m_deltaClock;
    sf::Clock m_fpsClock;
    int m_frames = 0;
    int m_currentFps = 0;
    float m_totalTime = 0.0f;

    // Movement input.
    bool m_wPressed = false, m_sPressed = false, m_aPressed = false, m_dPressed = false;
    bool m_spacePressed = false, m_shiftPressed = false;
    bool m_spaceWasPressed = false;
    bool m_wWasPressed = false;
    bool m_isSprinting = false;
    float m_lastWPressTime = -1.0f;

    // Mouse state.
    sf::Vector2i m_lastMouse;
    bool m_pendingReset = false;

    // Camera view mode.
    CameraView m_cameraView = CameraView::FirstPerson;
    Vector3 findSafeCameraPosition(const Vector3& headPos, const Vector3& desiredPos, const World& world);

    void initRenderer(float scale);
    void loadFont();
};
