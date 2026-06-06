#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>
#include <memory>
#include <filesystem>
#include <vector>

#include "core/Camera.h"
#include "math/Vector3.h"
#include "math/Matrix4x4.h"
#include "renderer/Renderer.h"
#include "world/TextureManager.h"
#include "world/BlockRegistry.h"
#include "world/World.h"

std::string getAssetPath(const std::string& filename) {
#ifdef __APPLE__
    std::vector<std::string> paths = {
        "assets/" + filename,
        "../Resources/assets/" + filename,
        "3D-game.app/Contents/Resources/assets/" + filename,
        "/Users/osman-nyri/Desktop/Проекты/Games/3D-Engine/assets/" + filename
    };

    for (const auto& path : paths) {
        if (std::filesystem::exists(path)) {
            std::cout << "Found asset: " << path << std::endl;
            return path;
        }
    }
#endif
    return "assets/" + filename;
}

int main() {
    const unsigned int WINDOW_W = 1680;
    const unsigned int WINDOW_H = 1050;
    float renderScale = 0.25f;

    sf::RenderWindow window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Quadro");
    window.setFramerateLimit(120);

    // Установка иконки приложения
    {
        sf::Image icon;
        std::string iconPath = getAssetPath("AppIcon.png");
        if (icon.loadFromFile(iconPath)) {
            window.setIcon(icon);
            std::cout << "App icon loaded: " << iconPath << "\n";
        }
    }

    std::unique_ptr<Renderer> renderer;

    auto initRenderer = [&](float scale) {
        unsigned int rw = static_cast<unsigned int>(WINDOW_W * scale);
        unsigned int rh = static_cast<unsigned int>(WINDOW_H * scale);
        if (rw < 32) rw = 32;
        if (rh < 32) rh = 32;

        renderer = std::make_unique<Renderer>(rw, rh);
        std::cout << "Resolution changed: " << rw << "x" << rh << " (Scale: " << scale << ")\n";
    };

    initRenderer(renderScale);

    TextureManager texMgr;
    std::string assetsPath = getAssetPath("");
    // Убираем завершающий слэш если есть
    if (!assetsPath.empty() && assetsPath.back() == '/') {
        assetsPath.pop_back();
    }
    if (!texMgr.loadFromDirectory(assetsPath, 16)) {
        std::cerr << "Failed to load textures from: " << assetsPath << "\n";
        return -1;
    }

    BlockRegistry::instance().init();

    World world;
    world.generate(10, 10);

    Camera camera(Vector3(8.0f, 25.0f, 8.0f), Vector3(0, 0, 1), Vector3(0, 1, 0));

    sf::Font font;
    std::string fontPath = "/System/Library/Fonts/Helvetica.ttc";
    if (!std::filesystem::exists(fontPath)) {
        fontPath = "/System/Library/Fonts/Arial.ttf";
    }
    if (!font.openFromFile(fontPath)) {
        std::cerr << "Font not found!\n";
    }

    sf::Text uiText(font);
    uiText.setCharacterSize(16);
    uiText.setFillColor(sf::Color::White);
    uiText.setOutlineColor(sf::Color::Black);
    uiText.setOutlineThickness(1.0f);
    uiText.setPosition({10, 10});

    sf::Clock deltaClock, fpsClock;
    int frames = 0, currentFps = 0;
    bool rotating = false;
    sf::Vector2i lastMouse;

    bool wPressed = false, sPressed = false, aPressed = false, dPressed = false;
    bool spacePressed = false, shiftPressed = false;
    bool spaceWasPressed = false;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();

                if (key->code == sf::Keyboard::Key::Equal) {
                    renderScale = std::min(1.0f, renderScale + 0.05f);
                    initRenderer(renderScale);
                }
                if (key->code == sf::Keyboard::Key::Hyphen) {
                    renderScale = std::max(0.1f, renderScale - 0.05f);
                    initRenderer(renderScale);
                }

                if (key->code == sf::Keyboard::Key::F1)
                    camera.cycleMode();

                if (key->scancode == sf::Keyboard::Scancode::W) wPressed = true;
                if (key->scancode == sf::Keyboard::Scancode::S) sPressed = true;
                if (key->scancode == sf::Keyboard::Scancode::A) aPressed = true;
                if (key->scancode == sf::Keyboard::Scancode::D) dPressed = true;
                if (key->code == sf::Keyboard::Key::Space) spacePressed = true;
                if (key->scancode == sf::Keyboard::Scancode::LShift) shiftPressed = true;
            }

            if (const auto* key = event->getIf<sf::Event::KeyReleased>()) {
                if (key->scancode == sf::Keyboard::Scancode::W) wPressed = false;
                if (key->scancode == sf::Keyboard::Scancode::S) sPressed = false;
                if (key->scancode == sf::Keyboard::Scancode::A) aPressed = false;
                if (key->scancode == sf::Keyboard::Scancode::D) dPressed = false;
                if (key->code == sf::Keyboard::Key::Space) spacePressed = false;
                if (key->scancode == sf::Keyboard::Scancode::LShift) shiftPressed = false;
            }

            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Right) {
                    rotating = true;
                    lastMouse = sf::Mouse::getPosition(window);
                }
            }

            if (const auto* mb = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mb->button == sf::Mouse::Button::Right)
                    rotating = false;
            }
        }

        float dt = deltaClock.restart().asSeconds();

        if (rotating) {
            auto pos = sf::Mouse::getPosition(window);
            float dx = (pos.x - lastMouse.x) * 0.003f;
            float dy = (pos.y - lastMouse.y) * 0.003f;
            camera.rotate(-dx, dy);
            lastMouse = pos;
        }

        float speed = 12.0f * dt;

        if (wPressed) camera.moveForward(speed, world);
        if (sPressed) camera.moveForward(-speed, world);
        if (aPressed) camera.moveRight(-speed, world);
        if (dPressed) camera.moveRight(speed, world);

        if (spacePressed && !spaceWasPressed) {
            if (camera.getMode() == Camera::Mode::Survival) {
                if (camera.isOnGround(world)) camera.jump();
            } else {
                camera.moveUp(speed);
            }
        }
        spaceWasPressed = spacePressed;

        if (shiftPressed) {
            if (camera.getMode() != Camera::Mode::Survival) camera.moveUp(-speed);
        }

        camera.updatePhysics(dt, world);

        renderer->clear(sf::Color(100, 149, 237));

        float aspect = float(renderer->getWidth()) / float(renderer->getHeight());
        Matrix4x4 proj = Matrix4x4::perspective(90.0f, aspect, 0.1f, 120.0f);
        Matrix4x4 view = camera.getViewMatrix();

        renderer->setLightDirection(Vector3(0.5f, -1.0f, 0.3f));
        world.draw(*renderer, texMgr, view, proj, camera.getPosition());
        renderer->display(window);

        frames++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            currentFps = (int)(frames / fpsClock.restart().asSeconds());
            frames = 0;
        }

        std::string modeStr = (camera.getMode() == Camera::Mode::Survival) ? "Survival" : "Free";
        uiText.setString("FPS: " + std::to_string(currentFps) +
                         "\nScale: " + std::to_string(renderScale).substr(0, 4) +
                         "\nRes: " + std::to_string(renderer->getWidth()) + "x" + std::to_string(renderer->getHeight()) +
                         "\nMode: " + modeStr +
                         "\n[+/-] Res | [F1] Mode");

        window.draw(uiText);
        window.display();
    }

    return 0;
}
