#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cmath>
#include <vector>

#include "core/Camera.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix4x4.h"
#include "renderer/Renderer.h"

int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;
    
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Software 3D Renderer");
    // window.setFramerateLimit(60);
    
    Renderer renderer(WIDTH, HEIGHT);
    
    // ========================================
    // КУБ: 8 вершин, 12 треугольников
    // ========================================
    float s = 0.5f; // половина стороны куба
    
    std::vector<Vector3> vertices = {
        // Передняя грань
        Vector3(-s, -s,  s), // 0: лево-низ-близко
        Vector3( s, -s,  s), // 1: право-низ-близко
        Vector3( s,  s,  s), // 2: право-верх-близко
        Vector3(-s,  s,  s), // 3: лево-верх-близко
        
        // Задняя грань
        Vector3(-s, -s, -s), // 4: лево-низ-далеко
        Vector3( s, -s, -s), // 5: право-низ-далеко
        Vector3( s,  s, -s), // 6: право-верх-далеко
        Vector3(-s,  s, -s), // 7: лево-верх-далеко
    };
    
    // Индексы: каждые 3 числа = один треугольник
    std::vector<unsigned int> indices = {
        // Передняя грань (z = +s)
        0, 1, 2,  0, 2, 3,
        // Задняя грань (z = -s)
        5, 4, 7,  5, 7, 6,
        // Правая грань (x = +s)
        1, 5, 6,  1, 6, 2,
        // Левая грань (x = -s)
        4, 0, 3,  4, 3, 7,
        // Верхняя грань (y = +s)
        3, 2, 6,  3, 6, 7,
        // Нижняя грань (y = -s)
        4, 5, 1,  4, 1, 0,
    };
    
    // ========================================
    // Матрица проекции (один раз)
    // ========================================
    float fov = 90.0f;  // градусы
    float aspect = static_cast<float>(WIDTH) / static_cast<float>(HEIGHT);
    float near = 0.1f;
    float far = 100.0f;
    
    Matrix4x4 proj = Matrix4x4::perspective(fov, aspect, near, far);
    
    // ========================================
    // Камера
    // ========================================
    Camera camera(Vector3(0, 0, 3), Vector3(0, 0, 0), Vector3(0, 1, 0));
    Matrix4x4 view = camera.getViewMatrix();
    
    // ========================================
    // Часы для анимации
    // ========================================
    sf::Clock clock;
    sf::Clock deltaClock;

    // ФПС
    sf::Clock fpsClock;
    int frameCount = 0;
    float fps = 0.0f;
    sf::Font font;
    font.openFromFile("/System/Library/Fonts/Helvetica.ttc");
    sf::Text fpsText(font);
    fpsText.setFont(font);
    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({10.0f, 10.0f});




    bool cursorGrabbed = true;
    window.setMouseCursorGrabbed(true);
    sf::Vector2i lastMousePos;

    // ========================================
    // Главный цикл
    // ========================================
    while (window.isOpen()) {
        // Обработка событий
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            // Выход по Escape
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Tab) {
                    cursorGrabbed = !cursorGrabbed;
                    window.setMouseCursorGrabbed(cursorGrabbed);
                }
            }
        }

        // Свет
        renderer.setLightDirection(Vector3(0.0f, 1.0f, 0.0f));
        
        float deltaTime = deltaClock.restart().asSeconds();

        if (cursorGrabbed) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            // Чтобы не было рывка при первом захвате
            if (lastMousePos.x == 0 && lastMousePos.y == 0)
                lastMousePos = mousePos;
            
            float dx = static_cast<float>(mousePos.x - lastMousePos.x) * 0.003f;
            float dy = static_cast<float>(mousePos.y - lastMousePos.y) * 0.003f;
            camera.rotate(-dx, dy);
            lastMousePos = mousePos;
        } else {
            lastMousePos = {0, 0};  // сброс для следующего захвата
        }

        float speed = 2.0f * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) camera.moveForward(speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) camera.moveForward(-speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) camera.moveRight(-speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) camera.moveRight(speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) camera.moveUp(speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) camera.moveUp(-speed);

        view = camera.getViewMatrix();

        // Очистка
        renderer.clear(sf::Color(30, 30, 30));
        
        // Рисуем куб
        renderer.drawMesh(vertices, indices, Matrix4x4() , view, proj, sf::Color(255, 100, 0), camera.getPosition());
        
        // Показываем на экране
        renderer.display(window);

        frameCount++;
        if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
            fps = static_cast<float>(frameCount) / fpsClock.restart().asSeconds();
            frameCount = 0;
            fpsText.setString("FPS: " + std::to_string(static_cast<int>(fps)));
        }
        window.draw(fpsText);

        window.display();
    }
    
    return 0;
}