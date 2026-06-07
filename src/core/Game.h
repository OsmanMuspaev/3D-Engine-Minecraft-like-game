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
#include "renderer/Renderer.h"
#include "world/BlockRegistry.h"

class Game {
public:
    Game(unsigned int windowWidth, unsigned int windowHeight);
    ~Game();

    void run();

private:
    void init();
    void handleEvents();
    void update(float dt);
    void render();
    void cleanup();

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
    PlayerInteraction m_playerInteraction;

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

    sf::Vector2i m_lastMouse;
    bool m_pendingReset = false;

    void initRenderer(float scale);
    void loadFont();
};
