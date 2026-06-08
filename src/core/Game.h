#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <memory>
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

// Main game class that owns the window, world, and all subsystems.
class Game {
public:
    enum class CameraView {
        FirstPerson,
        ThirdPerson,
        FrontView
    };

    Game(unsigned int windowWidth, unsigned int windowHeight);
    ~Game();

    // Main loop entry point.
    void run();

private:
    void init();
    void handleEvents();
    void update(float dt);
    void render(float dt);
    void cleanup();

    // Window dimensions and render scale.
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

    // HUD text rendering.
    sf::Font m_font;
    sf::Text m_uiText;
    bool m_fontLoaded = false;

    // Timing and FPS tracking.
    sf::Clock m_deltaClock;
    sf::Clock m_fpsClock;
    int m_frames = 0;
    int m_currentFps = 0;
    float m_totalTime = 0.0f;

    // Movement input state.
    bool m_wPressed = false, m_sPressed = false, m_aPressed = false, m_dPressed = false;
    bool m_spacePressed = false, m_shiftPressed = false;
    bool m_spaceWasPressed = false;
    bool m_wWasPressed = false;
    bool m_isSprinting = false;
    float m_lastWPressTime = -1.0f;

    // Mouse state for camera rotation.
    sf::Vector2i m_lastMouse;
    bool m_pendingReset = false;

    // Camera view mode and helper.
    CameraView m_cameraView = CameraView::FirstPerson;
    Vector3 findSafeCameraPosition(const Vector3& headPos, const Vector3& desiredPos, const World& world);

    // Initialization helpers.
    void initRenderer(float scale);
    void loadFont();
};
