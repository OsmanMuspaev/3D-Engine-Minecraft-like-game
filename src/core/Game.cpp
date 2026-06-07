#include "Game.h"
#include "math/Matrix4x4.h"
#include <iostream>
#include <filesystem>
#include <vector>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

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

Game::Game(unsigned int windowWidth, unsigned int windowHeight)
    : m_windowWidth(windowWidth)
    , m_windowHeight(windowHeight)
    , m_window(sf::VideoMode({windowWidth, windowHeight}), "Quadro")
    , m_world()
    , m_camera(Vector3(8.0f, 25.0f, 8.0f), Vector3(0, 0, 1), Vector3(0, 1, 0))
    , m_inventory()
    , m_invUI()
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

void Game::initRenderer(float scale) {
    m_renderScale = scale;
    unsigned int rw = static_cast<unsigned int>(m_windowWidth * scale);
    unsigned int rh = static_cast<unsigned int>(m_windowHeight * scale);
    if (rw < 32) rw = 32;
    if (rh < 32) rh = 32;

    m_renderer = std::make_unique<Renderer>(rw, rh);
    std::cout << "Resolution changed: " << rw << "x" << rh << " (Scale: " << scale << ")\n";
}

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

    BlockRegistry::instance().init();
    ItemRegistry::instance().init();

    m_world.generate(20, 20);

    m_inventory.addItem(BlockType::STONE, 64);
    m_inventory.addItem(BlockType::DIRT, 64);
    m_inventory.addItem(BlockType::GRASS, 64);
    m_inventory.addItem(BlockType::WOOD, 64);
    m_inventory.addItem(BlockType::SAND, 64);
    m_inventory.addItem(BlockType::SANDSTONE, 64);
    m_inventory.addItem(BlockType::SNOW_BLOCK, 64);
    m_inventory.addItem(BlockType::LEAVES, 64);

    m_invUI.loadTextures(assetsPath);
    loadFont();

    m_lastMouse = sf::Mouse::getPosition(m_window);
}

void Game::handleEvents() {
    bool invMousePressed = false;
    bool invMouseReleased = false;
    bool leftClickBlock = false;
    bool rightClickBlock = false;

    while (auto event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            m_window.close();

        if (const auto* focus = event->getIf<sf::Event::FocusLost>()) {
            (void)focus;
            m_window.setMouseCursorVisible(true);
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Escape)
                m_window.close();

            if (key->code == sf::Keyboard::Key::Equal) {
                float newScale = std::min(1.0f, m_renderScale + 0.05f);
                initRenderer(newScale);
            }
            if (key->code == sf::Keyboard::Key::Hyphen) {
                float newScale = std::max(0.1f, m_renderScale - 0.05f);
                initRenderer(newScale);
            }

            if (key->code == sf::Keyboard::Key::F1)
                m_camera.cycleMode();

            if (key->code == sf::Keyboard::Key::E) {
                if (m_camera.getMode() != Camera::Mode::Spectator) {
                    m_invUI.toggleInventory();
                    m_lastMouse = sf::Mouse::getPosition(m_window);
                }
            }

            if (key->scancode == sf::Keyboard::Scancode::Num1) m_inventory.setSelectedHotbarIndex(0);
            if (key->scancode == sf::Keyboard::Scancode::Num2) m_inventory.setSelectedHotbarIndex(1);
            if (key->scancode == sf::Keyboard::Scancode::Num3) m_inventory.setSelectedHotbarIndex(2);
            if (key->scancode == sf::Keyboard::Scancode::Num4) m_inventory.setSelectedHotbarIndex(3);
            if (key->scancode == sf::Keyboard::Scancode::Num5) m_inventory.setSelectedHotbarIndex(4);
            if (key->scancode == sf::Keyboard::Scancode::Num6) m_inventory.setSelectedHotbarIndex(5);
            if (key->scancode == sf::Keyboard::Scancode::Num7) m_inventory.setSelectedHotbarIndex(6);
            if (key->scancode == sf::Keyboard::Scancode::Num8) m_inventory.setSelectedHotbarIndex(7);
            if (key->scancode == sf::Keyboard::Scancode::Num9) m_inventory.setSelectedHotbarIndex(8);

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

        if (const auto* scroll = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (!m_invUI.isInventoryOpen()) {
                m_inventory.cycleHotbar(scroll->delta > 0 ? -1 : 1);
            }
        }

        if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mb->button == sf::Mouse::Button::Left) {
                if (m_invUI.isInventoryOpen()) {
                    invMousePressed = true;
                } else if (m_camera.getMode() != Camera::Mode::Spectator) {
                    leftClickBlock = true;
                }
            }
        }

        if (const auto* mb = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mb->button == sf::Mouse::Button::Left && m_invUI.isInventoryOpen()) {
                invMouseReleased = true;
            }
        }

        if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mb->button == sf::Mouse::Button::Right && !m_invUI.isInventoryOpen()
                && m_camera.getMode() != Camera::Mode::Spectator) {
                rightClickBlock = true;
            }
        }
    }

    if (m_invUI.isInventoryOpen()) {
        m_window.setMouseCursorVisible(true);
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
        m_invUI.handleMouseInput(mousePos, invMousePressed, invMouseReleased, m_inventory, m_windowWidth, m_windowHeight);
    } else if (m_camera.getMode() != Camera::Mode::Spectator) {
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

    if (leftClickBlock) {
        m_playerInteraction.handleLeftClick(m_camera, m_world, m_inventory);
    }
    if (rightClickBlock) {
        m_playerInteraction.handleRightClick(m_camera, m_world, m_inventory);
    }
}

void Game::update(float dt) {
    float speed = 12.0f * dt;

    if (m_wPressed) m_camera.moveForward(speed, m_world);
    if (m_sPressed) m_camera.moveForward(-speed, m_world);
    if (m_aPressed) m_camera.moveRight(-speed, m_world);
    if (m_dPressed) m_camera.moveRight(speed, m_world);

    if (m_spacePressed && !m_spaceWasPressed) {
        m_camera.handleSpacePress(m_world, m_totalTime);
    }
    m_spaceWasPressed = m_spacePressed;

    if (m_spacePressed && m_camera.getMode() == Camera::Mode::Creative && m_camera.isFlying()) {
        m_camera.moveUp(speed, m_world);
    }

    if (m_camera.getMode() == Camera::Mode::Spectator) {
        if (m_spacePressed) m_camera.moveUp(speed, m_world);
    }

    if (m_shiftPressed) {
        if (m_camera.getMode() == Camera::Mode::Creative && m_camera.isFlying()) {
            m_camera.moveUp(-speed, m_world);
        } else if (m_camera.getMode() == Camera::Mode::Spectator) {
            m_camera.moveUp(-speed, m_world);
        }
    }

    m_camera.updatePhysics(dt, m_world);
}

void Game::render() {
    m_renderer->clear(sf::Color(100, 149, 237));

    float aspect = float(m_renderer->getWidth()) / float(m_renderer->getHeight());
    Matrix4x4 proj = Matrix4x4::perspective(90.0f, aspect, 0.1f, 120.0f);
    Matrix4x4 view = m_camera.getViewMatrix();

    m_renderer->setLightDirection(Vector3(0.5f, -1.0f, 0.3f));
    m_world.draw(*m_renderer, m_texMgr, view, proj, m_camera.getPosition());

    m_renderer->display(m_window);

    m_window.setView(m_window.getDefaultView());

    auto size = m_window.getSize();

    if (m_camera.getMode() != Camera::Mode::Spectator) {
        m_invUI.renderHotbar(
            m_window,
            m_inventory,
            m_texMgr,
            ItemRegistry::instance(),
            size.x,
            size.y
        );
    }

    if (!m_invUI.isInventoryOpen()) {
        m_invUI.renderCrosshair(
            m_window,
            size.x,
            size.y
        );
    }

    if (m_invUI.isInventoryOpen()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);

        m_invUI.renderInventory(
            m_window,
            m_inventory,
            m_texMgr,
            ItemRegistry::instance(),
            size.x,
            size.y,
            mousePos,
            false,
            false
        );
    }

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

    m_uiText.setString(
        "FPS: " + std::to_string(m_currentFps) +
        "\nScale: " + std::to_string(m_renderScale).substr(0, 4) +
        "\nRes: " + std::to_string(m_renderer->getWidth()) + "x" +
        std::to_string(m_renderer->getHeight()) +
        "\nMode: " + modeStr
    );

    m_window.draw(m_uiText);

    m_window.display();
}

void Game::run() {
    init();
    while (m_window.isOpen()) {
        handleEvents();
        float dt = m_deltaClock.restart().asSeconds();
        m_totalTime += dt;
        update(dt);
        render();
    }
    cleanup();
}

void Game::cleanup() {
}
