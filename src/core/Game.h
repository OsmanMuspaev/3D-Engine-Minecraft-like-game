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

    // Main game loop: init, then handle events, update, and render each frame.
    void run();

private:
    // Initializes textures, fonts, renderer, and menu system.
    void init();
    // Processes SFML events: keyboard, mouse, and window close.
    void handleEvents();
    // Polls SFML events while in a menu state.
    void handleMenuEvents();
    // Updates movement, physics, mouse clicks, network sync, and menu flags.
    void update(float dt);
    // Renders the 3D world, player model, other networked players, and 2D UI overlay.
    void render(float dt);
    // Renders the menu screen and displays the frame.
    void renderMenu();
    // Stops server and client on shutdown.
    void cleanup();

    // Adds the default set of building blocks to the inventory.
    void addDefaultInventory();

    // Generates a new world, populates the inventory, and enters the Playing state.
    void startNewWorld(const std::string& name, int worldSize);
    // Loads a saved world from disk, populates the inventory, and enters the Playing state.
    void loadExistingWorld(const std::string& name);
    // Parses a host:port address, connects to the server, and enters the Playing state.
    void connectToServer(const std::string& address);
    // Starts the TCP server on port 53000 and enters the Playing state.
    void startServer(bool useTunnel);
    // Saves the current world to disk using WorldSave.
    void saveCurrentWorld();
    // Stops networking, clears the world and inventory, and returns to the main menu.
    void resetGame();

    // Returns the local LAN IP address (first non-loopback IPv4).
    static std::string getLocalIP();
    // Launches localtunnel in a background thread and captures the URL.
    void launchLocalTunnel(unsigned short port);

    unsigned int m_windowWidth;
    unsigned int m_windowHeight;
    float m_renderScale = 0.25f;

    sf::RenderWindow m_window;
    std::unique_ptr<Renderer> m_renderer;

    TextureManager m_texMgr;
    World m_world;
    Camera m_camera;
    Inventory m_inventory;
    InventoryUI m_invUI;
    PlayerView m_playerView;
    CraftingSystem m_crafting;
    PlayerInteraction m_playerInteraction;

    MenuManager m_menu;

    Server m_server;
    Client m_client;
    bool m_isServer = false;
    bool m_isClient = false;
    float m_networkSendTimer = 0.0f;
    std::string m_currentWorldName;

    std::thread m_tunnelThread;
    std::atomic<bool> m_tunnelRunning{false};
    std::string m_tunnelUrl;

    sf::Font m_font;
    sf::Text m_uiText;
    bool m_fontLoaded = false;

    sf::Clock m_deltaClock;
    sf::Clock m_fpsClock;
    int m_frames = 0;
    int m_currentFps = 0;
    float m_totalTime = 0.0f;

    bool m_wPressed = false, m_sPressed = false, m_aPressed = false, m_dPressed = false;
    bool m_spacePressed = false, m_shiftPressed = false;
    bool m_spaceWasPressed = false;
    bool m_wWasPressed = false;
    bool m_isSprinting = false;
    float m_lastWPressTime = -1.0f;

    sf::Vector2i m_lastMouse;
    bool m_pendingReset = false;

    CameraView m_cameraView = CameraView::FirstPerson;
    // Finds a camera position along a ray that doesn't clip through blocks.
    Vector3 findSafeCameraPosition(const Vector3& headPos, const Vector3& desiredPos, const World& world);

    // Creates or recreates the renderer at the given scale factor.
    void initRenderer(float scale);
    // Loads a system font for HUD text rendering.
    void loadFont();
};
