#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>

#include "core/Camera.h"
#include "math/Vector3.h"
#include "math/Matrix4x4.h"
#include "renderer/Renderer.h"
#include "world/TextureAtlas.h"
#include "world/Cube.h"

int main() {
    const unsigned int WIDTH = 800;
    const unsigned int HEIGHT = 600;
    
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "3D Renderer");
    window.setFramerateLimit(300);
    
    Renderer renderer(WIDTH, HEIGHT);
    
    TextureAtlas atlas;
    if (!atlas.loadFromFile("iron_block.png", 64)) {
        std::cerr << "Failed to load atlas!" << std::endl;
        return -1;
    }
    
    std::cout << "Atlas loaded: " << atlas.getTexture().getSize().x << "x"
              << atlas.getTexture().getSize().y << std::endl;
    
    Cube cube;
    cube.setAllFaces(0);
    Cube cube2;
    cube2.setAllFaces(0);
    Cube cube3;
    cube3.setAllFaces(0);
    
    float fov = 90.0f;
    float aspect = static_cast<float>(WIDTH) / HEIGHT;
    Matrix4x4 proj = Matrix4x4::perspective(fov, aspect, 0.1f, 100.0f);
    
    Camera camera(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 1, 0));
    Matrix4x4 view;
    
    sf::Clock clock;
    sf::Clock deltaClock;
    sf::Clock fpsClock;
    int frameCount = 0;
    float fps = 0.0f;
    sf::Font font;
    if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {
        std::cerr << "Failed to load font!" << std::endl;
    }
    sf::Text fpsText(font);
    fpsText.setCharacterSize(20);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({10.0f, 10.0f});

    bool rightMouseDown = false;
    sf::Vector2i lastMousePos;

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) window.close();
            }
            if (const auto* mb = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mb->button == sf::Mouse::Button::Right) {
                    rightMouseDown = true;
                    lastMousePos = sf::Mouse::getPosition(window);
                }
            }
            if (const auto* mb = event->getIf<sf::Event::MouseButtonReleased>()) {
                if (mb->button == sf::Mouse::Button::Right) rightMouseDown = false;
            }
        }
        
        float deltaTime = deltaClock.restart().asSeconds();
        if (rightMouseDown) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            float dx = static_cast<float>(mousePos.x - lastMousePos.x) * 0.003f;
            float dy = static_cast<float>(mousePos.y - lastMousePos.y) * 0.003f;
            camera.rotate(-dx, dy);
            lastMousePos = mousePos;
        }

        float speed = 2.0f * deltaTime;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) camera.moveForward(speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) camera.moveForward(-speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) camera.moveRight(-speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) camera.moveRight(speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) camera.moveUp(speed);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) camera.moveUp(-speed);
        view = camera.getViewMatrix();

        renderer.setLightDirection(Vector3(0.0f, 0.0f, -1.0f));
        renderer.clear(sf::Color(30, 30, 30));
        
        float time = clock.getElapsedTime().asSeconds();
        
        Matrix4x4 model1 = Matrix4x4::rotationY(time * 0.6f);
        cube.draw(renderer, atlas, model1, view, proj, camera.getPosition());

        Matrix4x4 model2 = Matrix4x4::translation(2, 0, 0) * Matrix4x4::rotationX(time * 0.4f);
        cube2.draw(renderer, atlas, model2, view, proj, camera.getPosition());
        
        Matrix4x4 model3 = Matrix4x4::translation(-2, 0, 0) * Matrix4x4::rotationY(-time * 0.5f);
        cube3.draw(renderer, atlas, model3, view, proj, camera.getPosition());
        
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
