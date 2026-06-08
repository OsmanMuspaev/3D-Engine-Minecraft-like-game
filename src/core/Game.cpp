#include "Game.h"
#include "math/Matrix4x4.h"
#include "world/WorldSave.h"
#include <iostream>
#include <filesystem>
#include <cstdio>
#include <array>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <cmath>

using CameraViewType = PlayerView::CameraViewType;

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

// Returns the macOS bundle Resources directory path, or empty string on other platforms.
static std::string getBundleResourcesPath() {
#ifdef __APPLE__
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (bundle) {
        CFURLRef url = CFBundleCopyResourcesDirectoryURL(bundle);
        if (url) {
            char path[1024];
            if (CFURLGetFileSystemRepresentation(url, true, reinterpret_cast<UInt8*>(path), sizeof(path))) {
                CFRelease(url);
                return std::string(path);
            }
            CFRelease(url);
        }
    }
#endif
    return "";
}

// Resolves an asset file path, checking bundle resources and common fallback locations.
static std::string getAssetPath(const std::string& filename) {
#ifdef __APPLE__
    std::string bundlePath = getBundleResourcesPath();
    if (!bundlePath.empty()) {
        std::string bp = bundlePath + "/assets/" + filename;
        if (std::filesystem::exists(bp)) {
            return bp;
        }
    }

    std::vector<std::string> paths = {
        "assets/" + filename,
        "../Resources/assets/" + filename,
        "3D-game.app/Contents/Resources/assets/" + filename,
        "/Users/osman-nyri/Проекты/Games/3D-Engine/assets/" + filename
    };

    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
#endif
    return "assets/" + filename;
}

// Creates the SFML window, loads the app icon, and sets up initial state.
Game::Game(unsigned int windowWidth, unsigned int windowHeight)
    : m_windowWidth(windowWidth)
    , m_windowHeight(windowHeight)
    , m_window(sf::VideoMode({windowWidth, windowHeight}), "Quadro")
    , m_world()
    , m_camera(Vector3(8.0f, 25.0f, 8.0f), Vector3(0, 0, 1), Vector3(0, 1, 0))
    , m_inventory()
    , m_invUI()
    , m_playerView()
    , m_crafting()
    , m_playerInteraction()
    , m_uiText(m_font)
{
    m_window.setFramerateLimit(120);

    sf::Image icon;
    std::string iconPath = getAssetPath("AppIcon.png");
    if (icon.loadFromFile(iconPath)) {
        m_window.setIcon(icon);
        std::cout << "App icon loaded: " << iconPath << "\n";
    }
}

Game::~Game() = default;

// Creates or recreates the renderer at the given scale factor.
void Game::initRenderer(float scale) {
    m_renderScale = scale;
    unsigned int rw = static_cast<unsigned int>(m_windowWidth * scale);
    unsigned int rh = static_cast<unsigned int>(m_windowHeight * scale);
    if (rw < 32) rw = 32;
    if (rh < 32) rh = 32;

    m_renderer = std::make_unique<Renderer>(rw, rh);
    std::cout << "Resolution changed: " << rw << "x" << rh << " (Scale: " << scale << ")\n";
}

// Loads a system font for HUD text rendering.
void Game::loadFont() {
    std::string fontPath = "/System/Library/Fonts/Helvetica.ttc";
    if (!std::filesystem::exists(fontPath)) {
        fontPath = "/System/Library/Fonts/Arial.ttf";
    }
    m_fontLoaded = m_font.openFromFile(fontPath);
    if (!m_fontLoaded) {
        std::cerr << "Font not found!\n";
        return;
    }

    m_uiText = sf::Text(m_font);
    m_uiText.setCharacterSize(16);
    m_uiText.setFillColor(sf::Color::White);
    m_uiText.setOutlineColor(sf::Color::Black);
    m_uiText.setOutlineThickness(1.0f);
    m_uiText.setPosition({10, 10});
}

// Adds the default set of building blocks to the inventory.
void Game::addDefaultInventory() {
    m_inventory.addItem(BlockType::STONE, 64);
    m_inventory.addItem(BlockType::DIRT, 64);
    m_inventory.addItem(BlockType::GRASS, 64);
    m_inventory.addItem(BlockType::OAK_LOG, 64);
    m_inventory.addItem(BlockType::OAK_PLANKS, 64);
    m_inventory.addItem(BlockType::COBBLESTONE, 64);
    m_inventory.addItem(BlockType::SAND, 32);
    m_inventory.addItem(BlockType::OAK_LEAVES, 32);
    m_inventory.addItem(BlockType::GLASS, 32);
    m_inventory.addItem(BlockType::IRON_BLOCK, 16);
    m_inventory.addItem(BlockType::DIAMOND_BLOCK, 8);
    m_inventory.addItem(BlockType::OBSIDIAN, 8);
    m_inventory.addItem(BlockType::TNT, 16);
    m_inventory.addItem(BlockType::CRAFTING_TABLE, 4);
    m_inventory.addItem(BlockType::FURNACE, 4);
    m_inventory.addItem(BlockType::CHEST, 4);
}

// Initializes textures, fonts, renderer, and menu system.
// Does NOT generate a world or populate inventory -- that happens when a world is started.
void Game::init() {
    initRenderer(m_renderScale);

    std::string assetsPath = getAssetPath("");
    if (!assetsPath.empty() && assetsPath.back() == '/') {
        assetsPath.pop_back();
    }
    if (!m_texMgr.loadFromDirectory(assetsPath, 16)) {
        std::cerr << "Failed to load textures from: " << assetsPath << "\n";
        return;
    }
    m_texMgr.loadItemTextures(assetsPath);

    BlockRegistry::instance().init();
    ItemRegistry::instance().init();

    m_invUI.loadTextures(assetsPath);
    m_playerView.loadTextures(assetsPath);
    loadFont();

    m_menu.init(this, m_windowWidth, m_windowHeight);
    m_menu.setState(MenuState::MainMenu);

    m_lastMouse = sf::Mouse::getPosition(m_window);
}

// Polls SFML events while in a menu state, forwarding them to the menu system.
void Game::handleMenuEvents() {
    while (auto event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            return;
        }

        if (const auto* resize = event->getIf<sf::Event::Resized>()) {
            m_windowWidth = resize->size.x;
            m_windowHeight = resize->size.y;
            m_menu.init(this, m_windowWidth, m_windowHeight);
        }

        m_menu.handleEvent(*event, m_window);
    }

    // Apply render scale changes from the settings menu.
    float menuScale = m_menu.getRenderScale();
    if (std::abs(menuScale - m_renderScale) > 0.001f) {
        initRenderer(menuScale);
    }

    // Apply window size changes from the settings menu.
    unsigned int menuW = static_cast<unsigned int>(m_menu.getWindowWidth());
    unsigned int menuH = static_cast<unsigned int>(m_menu.getWindowHeight());
    if (menuW != m_windowWidth || menuH != m_windowHeight) {
        m_windowWidth = menuW;
        m_windowHeight = menuH;
        m_menu.init(this, m_windowWidth, m_windowHeight);
        m_window.setSize({m_windowWidth, m_windowHeight});
        initRenderer(m_renderScale);
    }

    // Update server info screen with tunnel URL once available.
    if (m_menu.getState() == MenuState::ServerInfoScreen && !m_tunnelUrl.empty()) {
        m_menu.showServerInfo(m_tunnelUrl + "\n\nAnyone can connect using this address.\nInstall localtunnel: npm i -g localtunnel");
    }
}

// Processes SFML events: keyboard, mouse, and window close.
// Escape now opens the pause menu instead of closing the window.
void Game::handleEvents() {
    bool invLeftPressed = false;
    bool invLeftReleased = false;
    bool invRightPressed = false;
    bool invRightReleased = false;

    while (auto event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            m_window.close();

        if (const auto* focus = event->getIf<sf::Event::FocusLost>()) {
            (void)focus;
            m_window.setMouseCursorVisible(true);
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape)
                m_menu.setState(MenuState::PauseMenu);

            // Render resolution scaling.
            if (key->code == sf::Keyboard::Key::Equal) {
                float newScale = std::min(1.0f, m_renderScale + 0.05f);
                initRenderer(newScale);
            }
            if (key->code == sf::Keyboard::Key::Hyphen) {
                float newScale = std::max(0.1f, m_renderScale - 0.05f);
                initRenderer(newScale);
            }

            // Camera mode and view cycling.
            if (key->code == sf::Keyboard::Key::F1)
                m_camera.cycleMode();

            if (key->code == sf::Keyboard::Key::F5) {
                m_cameraView = static_cast<CameraView>((static_cast<int>(m_cameraView) + 1) % 3);
            }

            // Inventory toggle.
            if (key->code == sf::Keyboard::Key::E) {
                if (m_camera.getMode() != Camera::Mode::Spectator) {
                    m_invUI.toggleInventory();
                    m_lastMouse = sf::Mouse::getPosition(m_window);
                }
            }

            // Hotbar selection.
            if (key->scancode == sf::Keyboard::Scancode::Num1) m_inventory.setSelectedHotbarIndex(0);
            if (key->scancode == sf::Keyboard::Scancode::Num2) m_inventory.setSelectedHotbarIndex(1);
            if (key->scancode == sf::Keyboard::Scancode::Num3) m_inventory.setSelectedHotbarIndex(2);
            if (key->scancode == sf::Keyboard::Scancode::Num4) m_inventory.setSelectedHotbarIndex(3);
            if (key->scancode == sf::Keyboard::Scancode::Num5) m_inventory.setSelectedHotbarIndex(4);
            if (key->scancode == sf::Keyboard::Scancode::Num6) m_inventory.setSelectedHotbarIndex(5);
            if (key->scancode == sf::Keyboard::Scancode::Num7) m_inventory.setSelectedHotbarIndex(6);
            if (key->scancode == sf::Keyboard::Scancode::Num8) m_inventory.setSelectedHotbarIndex(7);
            if (key->scancode == sf::Keyboard::Scancode::Num9) m_inventory.setSelectedHotbarIndex(8);

            // Movement input.
            if (key->scancode == sf::Keyboard::Scancode::W) m_wPressed = true;
            if (key->scancode == sf::Keyboard::Scancode::S) m_sPressed = true;
            if (key->scancode == sf::Keyboard::Scancode::A) m_aPressed = true;
            if (key->scancode == sf::Keyboard::Scancode::D) m_dPressed = true;
            if (key->code == sf::Keyboard::Key::Space) m_spacePressed = true;
            if (key->scancode == sf::Keyboard::Scancode::LShift) m_shiftPressed = true;
        }

        if (const auto* key = event->getIf<sf::Event::KeyReleased>()) {
            if (key->scancode == sf::Keyboard::Scancode::W) m_wPressed = false;
            if (key->scancode == sf::Keyboard::Scancode::S) m_sPressed = false;
            if (key->scancode == sf::Keyboard::Scancode::A) m_aPressed = false;
            if (key->scancode == sf::Keyboard::Scancode::D) m_dPressed = false;
            if (key->code == sf::Keyboard::Key::Space) m_spacePressed = false;
            if (key->scancode == sf::Keyboard::Scancode::LShift) m_shiftPressed = false;
        }

        // Hotbar scroll.
        if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (!m_invUI.isInventoryOpen()) {
                m_inventory.cycleHotbar(scroll->delta > 0 ? -1 : 1);
            }
        }

        // Mouse button clicks for inventory interaction.
        if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mb->button == sf::Mouse::Button::Left) {
                if (m_invUI.isInventoryOpen()) {
                    invLeftPressed = true;
                }
            }
            if (mb->button == sf::Mouse::Button::Right) {
                if (m_invUI.isInventoryOpen()) {
                    invRightPressed = true;
                }
            }
        }

        if (const auto* mb = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mb->button == sf::Mouse::Button::Left && m_invUI.isInventoryOpen()) {
                invLeftReleased = true;
            }
            if (mb->button == sf::Mouse::Button::Right && m_invUI.isInventoryOpen()) {
                invRightReleased = true;
            }
        }
    }

    // Inventory mouse interaction.
    if (m_invUI.isInventoryOpen()) {
        m_window.setMouseCursorVisible(true);
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
        m_invUI.handleMouseInput(mousePos, invLeftPressed, invLeftReleased,
                                 invRightPressed, invRightReleased, m_inventory, m_crafting, m_windowWidth, m_windowHeight);
    } else if (m_camera.getMode() != Camera::Mode::Spectator) {
        // Free-look mouse rotation with edge wrapping.
        m_window.setMouseCursorVisible(false);

        if (m_pendingReset) {
            m_lastMouse = sf::Mouse::getPosition(m_window);
            m_pendingReset = false;
        }

        sf::Vector2i pos = sf::Mouse::getPosition(m_window);
        int dx = pos.x - m_lastMouse.x;
        int dy = pos.y - m_lastMouse.y;
        if (dx != 0 || dy != 0) {
            m_camera.rotate(-dx * 0.003f, dy * 0.003f);
        }

        const int margin = 5;
        sf::Vector2i windowSize(m_windowWidth, m_windowHeight);
        if (pos.x <= margin || pos.x >= static_cast<int>(windowSize.x) - margin ||
            pos.y <= margin || pos.y >= static_cast<int>(windowSize.y) - margin)
        {
            sf::Vector2i center(m_windowWidth / 2, m_windowHeight / 2);
            sf::Mouse::setPosition(center, m_window);
            m_pendingReset = true;
            m_lastMouse = pos;
        } else {
            m_lastMouse = pos;
        }
    } else {
        m_window.setMouseCursorVisible(true);
    }
}

// Updates movement, physics, mouse clicks, network sync, and menu flags.
void Game::update(float dt) {
    float baseSpeed = 4.5f;

    // Sprint: double-tap W on initial press only.
    if (m_wPressed && !m_wWasPressed) {
        float now = m_totalTime;
        if (now - m_lastWPressTime < 0.3f && m_lastWPressTime > 0.0f) {
            m_isSprinting = true;
        } else {
            m_isSprinting = false;
        }
        m_lastWPressTime = now;
    }
    m_wWasPressed = m_wPressed;
    if (!m_wPressed) m_isSprinting = false;

    float speed = baseSpeed * dt;
    if (m_isSprinting) speed *= 2.0f;

    // Horizontal movement.
    if (m_wPressed) m_camera.moveForward(speed, m_world);
    if (m_sPressed) m_camera.moveForward(-speed, m_world);
    if (m_aPressed) m_camera.moveRight(-speed, m_world);
    if (m_dPressed) m_camera.moveRight(speed, m_world);

    // Jump (single press).
    if (m_spacePressed && !m_spaceWasPressed) {
        m_camera.handleSpacePress(m_world, m_totalTime);
    }
    m_spaceWasPressed = m_spacePressed;

    // Fly up while holding space in creative mode.
    if (m_spacePressed && m_camera.getMode() == Camera::Mode::Creative && m_camera.isFlying()) {
        m_camera.moveUp(speed, m_world);
    }

    // Spectator vertical movement.
    if (m_camera.getMode() == Camera::Mode::Spectator) {
        if (m_spacePressed) m_camera.moveUp(speed, m_world);
    }

    // Descend in creative/spectator modes.
    if (m_shiftPressed) {
        if (m_camera.getMode() == Camera::Mode::Creative && m_camera.isFlying()) {
            m_camera.moveUp(-speed, m_world);
        } else if (m_camera.getMode() == Camera::Mode::Spectator) {
            m_camera.moveUp(-speed, m_world);
        }
    }

    m_camera.updatePhysics(dt, m_world);

    // Mouse click handling for block break/place.
    static bool lastLeftClick = false;
    static bool lastRightClick = false;
    static float breakAccumulator = 0.0f;
    static float placeAccumulator = 0.0f;

    bool currentLeftClick = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
    bool currentRightClick = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);

    if (!m_invUI.isInventoryOpen() && m_camera.getMode() != Camera::Mode::Spectator) {
        // Left click: break block (single action per press).
        if (currentLeftClick && !lastLeftClick) {
            m_playerInteraction.handleLeftClick(m_camera, m_world, m_inventory);
            m_playerView.onLeftClick();
            breakAccumulator = 0.0f;
        }

        // Right click: place block (single action per press).
        if (currentRightClick && !lastRightClick) {
            m_playerInteraction.handleRightClick(m_camera, m_world, m_inventory);
            m_playerView.onRightClick();
            placeAccumulator = 0.0f;
        }
    }

    lastLeftClick = currentLeftClick;
    lastRightClick = currentRightClick;

    // Network sync: send player position to server periodically.
    if (m_isClient && m_client.isConnected()) {
        m_networkSendTimer += dt;
        if (m_networkSendTimer >= 0.05f) {
            Vector3 pos = m_camera.getPosition();
            m_client.sendMove(pos.x, pos.y, pos.z, m_camera.getYaw(), m_camera.getPitch());
            m_networkSendTimer = 0.0f;
        }
        m_client.update(dt);
        auto blockUpdates = m_client.getBlockUpdates();
        for (auto& bu : blockUpdates) {
            m_world.setBlock(bu.x, bu.y, bu.z, static_cast<BlockType>(bu.type));
        }
    }

    if (m_isServer) {
        m_server.update(dt);
    }

    // Process menu actions.
    if (m_menu.wantsResume()) {
        m_menu.consumeResume();
    }
    if (m_menu.wantsLoadWorld()) {
        m_menu.consumeLoadWorld();
        loadExistingWorld(m_menu.getSelectedWorld());
    }
    if (m_menu.wantsCreateWorld()) {
        m_menu.consumeCreateWorld();
        startNewWorld(m_menu.getNewWorldName(), m_menu.getWorldSize());
    }
    if (m_menu.wantsConnect()) {
        m_menu.consumeConnect();
        connectToServer(m_menu.getServerAddress());
    }
    if (m_menu.wantsOpenToLAN()) {
        m_menu.consumeOpenToLAN();
        startServer(false);
    }
    if (m_menu.wantsOpenToTunnel()) {
        m_menu.consumeOpenToTunnel();
        startServer(true);
    }
    if (m_menu.wantsQuitToMenu()) {
        m_menu.consumeQuitToMenu();
        resetGame();
    }
}

// Finds a camera position along a ray that doesn't clip through blocks.
Vector3 Game::findSafeCameraPosition(const Vector3& headPos, const Vector3& desiredPos, const World& world) {
    Vector3 dir = (desiredPos - headPos).normalize();
    float maxDistance = (desiredPos - headPos).length();
    float step = 0.15f;

    float cameraRadius = 0.25f;

    for (float dist = 0.0f; dist <= maxDistance; dist += step) {
        Vector3 checkPos = headPos + dir * dist;

        bool collision = false;
        int bxMin = (int)std::floor(checkPos.x - cameraRadius);
        int bxMax = (int)std::floor(checkPos.x + cameraRadius);
        int byMin = (int)std::floor(checkPos.y - cameraRadius);
        int byMax = (int)std::floor(checkPos.y + cameraRadius);
        int bzMin = (int)std::floor(checkPos.z - cameraRadius);
        int bzMax = (int)std::floor(checkPos.z + cameraRadius);

        for (int bx = bxMin; bx <= bxMax; bx++) {
            for (int by = byMin; by <= byMax; by++) {
                for (int bz = bzMin; bz <= bzMax; bz++) {
                    if (world.isBlocking((float)bx, (float)by, (float)bz)) {
                        collision = true;
                        break;
                    }
                }
                if (collision) break;
            }
            if (collision) break;
        }

        if (collision) {
            float safeDist = std::max(0.5f, dist - step);
            return headPos + dir * safeDist;
        }
    }

    return desiredPos;
}

// Renders the 3D world, player model, other networked players, and 2D UI overlay.
void Game::render(float dt) {
    // Ensure cursor visibility is always correct.
    if (m_invUI.isInventoryOpen() || m_camera.getMode() == Camera::Mode::Spectator) {
        m_window.setMouseCursorVisible(true);
    } else {
        m_window.setMouseCursorVisible(false);
    }

    m_renderer->clear(sf::Color(100, 149, 237));

    float aspect = float(m_renderer->getWidth()) / float(m_renderer->getHeight());
    Matrix4x4 proj = Matrix4x4::perspective(90.0f, aspect, 0.1f, 120.0f);

    Vector3 eyePos = m_camera.getPosition();
    Vector3 feetPos = eyePos - Vector3(0, PLAYER_HEIGHT, 0);
    Vector3 camPos = eyePos;
    Vector3 lookTarget;

    // Calculate camera position and look target based on view mode.
    if (m_cameraView == CameraView::FirstPerson) {
        lookTarget = camPos + m_camera.getForward();
    } else if (m_cameraView == CameraView::ThirdPerson) {
        Vector3 fwd = m_camera.getForward();
        Vector3 back = Vector3(-fwd.x, 0, -fwd.z);
        if (back.length() > 0.001f) back = back.normalize();

        Vector3 headPos = feetPos + Vector3(0, 1.75f, 0);

        float pitch = m_camera.getPitch();
        float verticalOffset = std::tan(pitch) * 1.5f;
        verticalOffset = std::clamp(verticalOffset, -2.5f, 2.5f);

        camPos = headPos + back * 4.0f;
        camPos.y += verticalOffset;
        lookTarget = headPos;
    } else {
        Vector3 fwd = m_camera.getForward();
        Vector3 front = Vector3(fwd.x, 0, fwd.z);
        if (front.length() > 0.001f) front = front.normalize();

        Vector3 headPos = feetPos + Vector3(0, 1.75f, 0);

        float pitch = m_camera.getPitch();
        float verticalOffset = std::tan(pitch) * 1.5f;
        verticalOffset = std::clamp(verticalOffset, -2.5f, 2.5f);

        camPos = headPos + front * 4.0f;
        camPos.y += verticalOffset;
        lookTarget = headPos;
    }

    Matrix4x4 view = Matrix4x4::lookAt(camPos, lookTarget, Vector3(0, 1, 0));

    // Draw the voxel world.
    m_renderer->setLightDirection(Vector3(0.5f, -1.0f, 0.3f));
    m_world.draw(*m_renderer, m_texMgr, view, proj, camPos);

    // Draw the player model in third-person views.
    if (m_cameraView != CameraView::FirstPerson) {
        bool moving = m_wPressed || m_sPressed || m_aPressed || m_dPressed;
        CameraViewType viewType = (m_cameraView == CameraView::ThirdPerson)
            ? CameraViewType::ThirdPersonBack
            : CameraViewType::ThirdPersonFront;

        m_playerView.renderPlayer(*m_renderer, feetPos, m_camera.getForward(),
                                m_camera.getYaw(), m_camera.getPitch(),
                                view, proj, camPos, moving, dt, viewType);
    }

    // Draw other players (network).
    if (m_isClient) {
        auto others = m_client.getOtherPlayers();
        for (auto& other : others) {
            Vector3 otherPos(other.x, other.y - PLAYER_HEIGHT, other.z);
            m_playerView.renderPlayer(*m_renderer, otherPos, Vector3(0, 0, 1),
                other.yaw, other.pitch, view, proj, camPos, false, 0.0f,
                CameraViewType::ThirdPersonBack);
        }
    }

    // Draw the held item in first-person view.
    if (m_cameraView == CameraView::FirstPerson) {
        bool moving = m_wPressed || m_sPressed || m_aPressed || m_dPressed;
        m_playerView.renderHand(*m_renderer, eyePos, m_camera.getForward(),
                                m_inventory, m_texMgr, ItemRegistry::instance(),
                                view, proj, dt, moving);
    }

    // Blit the 3D framebuffer to the window.
    m_renderer->display(m_window);
    m_window.setView(m_window.getDefaultView());

    auto size = m_window.getSize();

    // 2D UI overlay: hotbar, crosshair, inventory.
    if (m_camera.getMode() != Camera::Mode::Spectator) {
        m_invUI.renderHotbar(m_window, m_inventory, m_texMgr, ItemRegistry::instance(), size.x, size.y);
    }

    if (!m_invUI.isInventoryOpen()) {
        m_invUI.renderCrosshair(m_window, size.x, size.y);
    }

    if (m_invUI.isInventoryOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
        m_invUI.renderInventory(m_window, m_inventory, m_crafting, m_texMgr, ItemRegistry::instance(),
                                size.x, size.y, mousePos, false, false);
    }

    // FPS counter.
    m_frames++;
    if (m_fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
        m_currentFps = static_cast<int>(m_frames / m_fpsClock.restart().asSeconds());
        m_frames = 0;
    }

    std::string modeStr;
    switch (m_camera.getMode()) {
        case Camera::Mode::Survival:  modeStr = "Survival"; break;
        case Camera::Mode::Creative:  modeStr = "Creative"; break;
        case Camera::Mode::Spectator: modeStr = "Spectator"; break;
    }

    std::string statusText =
        "FPS: " + std::to_string(m_currentFps) +
        "\nScale: " + std::to_string(m_renderScale).substr(0, 4) +
        "\nRes: " + std::to_string(m_renderer->getWidth()) + "x" +
        std::to_string(m_renderer->getHeight()) +
        "\nMode: " + modeStr;

    if (m_isServer) {
        statusText += "\nServer: " + std::to_string(m_server.getClientCount()) + " clients";
    }

    m_uiText.setString(statusText);
    m_window.draw(m_uiText);
    m_window.display();
}

// Renders the menu screen and displays the frame.
void Game::renderMenu() {
    m_menu.update(1.0f / 60.0f, m_window);
    m_menu.draw(m_window);
    m_window.display();
}

// Generates a new world, populates the inventory, and enters the Playing state.
void Game::startNewWorld(const std::string& name, int worldSize) {
    m_currentWorldName = name;
    m_world.generate(worldSize, worldSize);
    addDefaultInventory();
    m_menu.setState(MenuState::Playing);
    saveCurrentWorld();
    std::cout << "Created world: " << name << " (size " << worldSize << ")\n";
}

// Loads a saved world from disk, populates the inventory, and enters the Playing state.
void Game::loadExistingWorld(const std::string& name) {
    int worldSize = 20;
    std::string seed;
    if (WorldSave::loadWorld(name, m_world, worldSize, seed)) {
        m_currentWorldName = name;
        addDefaultInventory();
        m_menu.setState(MenuState::Playing);
        std::cout << "Loaded world: " << name << "\n";
    } else {
        std::cerr << "Failed to load world: " << name << "\n";
    }
}

// Parses a host:port address, connects to the server, and enters the Playing state.
void Game::connectToServer(const std::string& address) {
    std::string host = address;
    unsigned short port = 53000;

    auto colonPos = address.rfind(':');
    if (colonPos != std::string::npos) {
        host = address.substr(0, colonPos);
        try {
            port = static_cast<unsigned short>(std::stoi(address.substr(colonPos + 1)));
        } catch (...) {
            std::cerr << "Invalid port in address: " << address << "\n";
            return;
        }
    }

    if (m_client.connect(host, port)) {
        m_isClient = true;
        m_menu.setState(MenuState::Playing);
        std::cout << "Connected to " << host << ":" << port << "\n";
    } else {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
    }
}

// Starts the TCP server on port 53000 and enters the Playing state.
void Game::startServer(bool useTunnel) {
    // If server is already running, just show the info screen.
    if (m_isServer) {
        if (useTunnel) {
            launchLocalTunnel(53000);
        } else {
            std::string ip = getLocalIP();
            m_menu.showServerInfo("Local IP: " + ip + ":53000\n\nShare this address with other\nplayers on your local network.");
        }
        return;
    }

    if (m_server.start(53000)) {
        m_isServer = true;
        m_server.setWorld(&m_world, m_menu.getWorldSize());

        if (useTunnel) {
            launchLocalTunnel(53000);
        } else {
            std::string ip = getLocalIP();
            m_menu.showServerInfo("Local IP: " + ip + ":53000\n\nShare this address with other\nplayers on your local network.");
        }
    } else {
        m_menu.showServerInfo("Failed to start server on port 53000.\nPort may be in use by another program.");
    }
}

// ---------------------------------------------------------------------------
// Get the local LAN IP address (first non-loopback IPv4)
// ---------------------------------------------------------------------------

std::string Game::getLocalIP() {
    struct ifaddrs* ifas = nullptr;
    if (getifaddrs(&ifas) != 0)
        return "127.0.0.1";

    std::string result = "127.0.0.1";
    for (auto* ifa = ifas; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;
        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;
        if (ifa->ifa_flags & IFF_LOOPBACK)
            continue;

        auto* sa = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
        char buf[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf))) {
            result = buf;
            break;
        }
    }
    freeifaddrs(ifas);
    return result;
}

// ---------------------------------------------------------------------------
// Launch localtunnel in a background thread, capture the URL
// ---------------------------------------------------------------------------

void Game::launchLocalTunnel(unsigned short port) {
    if (m_tunnelRunning)
        return;

    m_tunnelRunning = true;
    m_tunnelUrl.clear();

    m_tunnelThread = std::thread([this, port]() {
        std::string cmd = "lt --port " + std::to_string(port) + " 2>&1";
        FILE* pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            m_tunnelUrl = "Error: failed to launch localtunnel.\nMake sure 'lt' is installed:\n  npm install -g localtunnel";
            m_tunnelRunning = false;
            return;
        }

        char buf[512];
        while (fgets(buf, sizeof(buf), pipe)) {
            std::string line(buf);
            // lt outputs: "your url is: https://xxx.loca.lt"
            if (line.find("loca.lt") != std::string::npos ||
                line.find("https://") != std::string::npos) {
                // Trim trailing newline
                while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
                    line.pop_back();
                m_tunnelUrl = line;
                break;
            }
        }

        if (m_tunnelUrl.empty()) {
            m_tunnelUrl = "Error: localtunnel exited without providing a URL.\nMake sure 'lt' is installed:\n  npm install -g localtunnel";
        }

        pclose(pipe);
        m_tunnelRunning = false;
    });
    m_tunnelThread.detach();

    // Show initial info immediately — URL will be available shortly
    m_menu.showServerInfo("Starting LocalTunnel...\nWaiting for tunnel URL...");
}

// Saves the current world to disk using WorldSave.
void Game::saveCurrentWorld() {
    if (!m_currentWorldName.empty()) {
        if (WorldSave::saveWorld(m_currentWorldName, m_world, m_menu.getWorldSize(), m_menu.getWorldSeed())) {
            std::cout << "World saved: " << m_currentWorldName << "\n";
        } else {
            std::cerr << "Failed to save world: " << m_currentWorldName << "\n";
        }
    }
}

// Stops networking, clears the world and inventory, and returns to the main menu.
void Game::resetGame() {
    if (m_isServer) {
        m_server.stop();
        m_isServer = false;
    }
    if (m_isClient) {
        m_client.disconnect();
        m_isClient = false;
    }
    if (m_tunnelRunning) {
        m_tunnelRunning = false;
        if (m_tunnelThread.joinable())
            m_tunnelThread.detach();
    }
    m_tunnelUrl.clear();

    saveCurrentWorld();
    m_world = World();
    m_inventory.clear();
    m_currentWorldName.clear();
    m_menu.setState(MenuState::MainMenu);
}

// Main game loop: init, then handle events / update / render each frame.
void Game::run() {
    init();
    while (m_window.isOpen()) {
        float dt = m_deltaClock.restart().asSeconds();
        m_totalTime += dt;

        if (m_menu.getState() != MenuState::Playing) {
            handleMenuEvents();
            renderMenu();
        } else {
            handleEvents();
            update(dt);
            render(dt);
        }
    }
    cleanup();
}

// Stops server and client on shutdown.
void Game::cleanup() {
    if (m_isServer) {
        m_server.stop();
        m_isServer = false;
    }
    if (m_isClient) {
        m_client.disconnect();
        m_isClient = false;
    }
    m_tunnelRunning = false;
}
