#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <iostream>
#include <memory>

#include "core/Camera.h"
#include "math/Vector3.h"
#include "math/Matrix4x4.h"
#include "renderer/Renderer.h"
#include "world/TextureAtlas.h"
#include "world/World.h"

int main() {
    // Константы окна
    const unsigned int WINDOW_W = 1680;
    const unsigned int WINDOW_H = 1050;

    // Начальный масштаб рендеринга (0.25 = 1/4 от размера окна)
    float renderScale = 0.25f;

    sf::RenderWindow window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Voxel Engine CPU - Dynamic Resolution");
    window.setFramerateLimit(120);

    // Используем указатель, чтобы пересоздавать рендерер при изменении разрешения
    std::unique_ptr<Renderer> renderer;

    // Функция для инициализации рендерера под конкретный масштаб
    auto initRenderer = [&](float scale) {
        unsigned int rw = static_cast<unsigned int>(WINDOW_W * scale);
        unsigned int rh = static_cast<unsigned int>(WINDOW_H * scale);
        if (rw < 32) rw = 32; // Защита от слишком маленького разрешения
        if (rh < 32) rh = 32;
        
        renderer = std::make_unique<Renderer>(rw, rh);
        std::cout << "Resolution changed: " << rw << "x" << rh << " (Scale: " << scale << ")\n";
    };

    // Первая инициализация
    initRenderer(renderScale);

    TextureAtlas atlas;
    if (!atlas.loadFromFile("assets/atlas.png", 16)) {
        std::cerr << "Failed to load atlas!\n";
        return -1;
    }

    // Генерация мира (радиус 10 чанков = 21x21 чанк)
    World world;
    world.generate(10, 10); 

    // Настройка камеры
    Camera camera(Vector3(8.0f, 25.0f, 8.0f), Vector3(0, 0, 1), Vector3(0, 1, 0));

    // UI для отображения инфы
    sf::Font font;
    // Путь для macOS, на Windows/Linux замени на стандартный путь к шрифту
    if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        std::cerr << "Font not found!\n";
    }

    sf::Text uiText(font);
    uiText.setCharacterSize(16);
    uiText.setFillColor(sf::Color::White);
    uiText.setOutlineColor(sf::Color::Black);
    uiText.setOutlineThickness(1.0f);
    uiText.setPosition({10, 10});

    sf::Clock deltaClock;
    sf::Clock fpsClock;
    int frames = 0;
    int currentFps = 0;

    bool rotating = false;
    sf::Vector2i lastMouse;

    // Основной цикл
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape)
                    window.close();
                
                // ГОРЯЧИЕ КЛАВИШИ ДЛЯ РАЗРЕШЕНИЯ
                if (key->code == sf::Keyboard::Key::Equal) { // Клавиша '+'
                    renderScale = std::min(1.0f, renderScale + 0.05f);
                    initRenderer(renderScale);
                }
                if (key->code == sf::Keyboard::Key::Hyphen) { // Клавиша '-'
                    renderScale = std::max(0.1f, renderScale - 0.05f);
                    initRenderer(renderScale);
                }
                
                if (key->code == sf::Keyboard::Key::F1)
                    camera.cycleMode();
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

        // Поворот камеры
        if (rotating) {
            auto pos = sf::Mouse::getPosition(window);
            float dx = (pos.x - lastMouse.x) * 0.003f;
            float dy = (pos.y - lastMouse.y) * 0.003f;
            camera.rotate(-dx, dy);
            lastMouse = pos;
        }

        // Движение
        float speed = 12.0f * dt;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W)) camera.moveForward(speed, world);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S)) camera.moveForward(-speed, world);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::A)) camera.moveRight(-speed, world);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::D)) camera.moveRight(speed, world);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            if (camera.getMode() == Camera::Mode::Survival) {
                if (camera.isOnGround(world)) camera.jump();
            } else {
                camera.moveUp(speed);
            }
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            if (camera.getMode() != Camera::Mode::Survival) camera.moveUp(-speed);
        }

        // Физика
        camera.updatePhysics(dt, world);

        // --- РЕНДЕРИНГ ---
        renderer->clear(sf::Color(100, 149, 237)); 

        // Пересчитываем матрицы на основе текущего разрешения рендерера
        float aspect = float(renderer->getWidth()) / float(renderer->getHeight());
        Matrix4x4 proj = Matrix4x4::perspective(90.0f, aspect, 0.1f, 120.0f);
        Matrix4x4 view = camera.getViewMatrix();

        renderer->setLightDirection(Vector3(0.5f, -1.0f, 0.3f));
        
        // Рисуем мир
        world.draw(*renderer, atlas, view, proj, camera.getPosition());

        // Вывод буфера на экран (масштабируется под окно автоматически)
        renderer->display(window);

        // Обновление FPS и текста
        frames++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            currentFps = (int)(frames / fpsClock.restart().asSeconds());
            frames = 0;
        }

        uiText.setString("FPS: " + std::to_string(currentFps) + 
                         "\nScale: " + std::to_string(renderScale).substr(0, 4) +
                         "\nRes: " + std::to_string(renderer->getWidth()) + "x" + std::to_string(renderer->getHeight()) +
                         "\nControls: [+/-] Change Res | [F1] Mode");

        window.draw(uiText);
        window.display(); 
    }

    return 0;
}