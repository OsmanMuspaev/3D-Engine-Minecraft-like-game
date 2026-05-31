#include <SFML/Graphics.hpp>

#include <SFML/Window.hpp>

#include <cmath>

#include <iostream>



#include "core/Camera.h"

#include "math/Vector3.h"

#include "math/Matrix4x4.h"

#include "renderer/Renderer.h"

#include "world/TextureAtlas.h"

#include "world/World.h"



int main() {

// Уменьшаем разрешение в 4 раза для оптимизации на CPU

const unsigned int WINDOW_W = 1680;

const unsigned int WINDOW_H = 1050;

const unsigned int RENDER_W = WINDOW_W / 2;

const unsigned int RENDER_H = WINDOW_H / 2;



sf::RenderWindow window(sf::VideoMode({WINDOW_W, WINDOW_H}), "Voxel Engine CPU");

window.setFramerateLimit(120);



// Инициализируем рендерер маленьким разрешением

Renderer renderer(RENDER_W, RENDER_H);



TextureAtlas atlas;

if (!atlas.loadFromFile("assets/atlas.png", 16)) {

std::cerr << "Failed to load atlas!\n";

return -1;

}



// Создаем и генерируем мир

World world(128, 256, 128);

world.generate(); // Наш новый метод с холмами

world.buildMesh(atlas);



// ---------------- CAMERA ----------------

float fov = 90.0f;

float aspect = float(RENDER_W) / float(RENDER_H);

Matrix4x4 proj = Matrix4x4::perspective(fov, aspect, 0.1f, 100.0f);



// Спавнимся чуть выше, чтобы не застрять в холмах

Camera camera(Vector3(16.0f, 16.0f, 16.0f), Vector3(0, 0, 1), Vector3(0, 1, 0));



// ---------------- UI ----------------

sf::Font font;

// Путь для macOS, если не найдет — замени на свой путь к .ttf

if (!font.openFromFile("/System/Library/Fonts/Helvetica.ttc")) {

std::cerr << "Font not found, UI might not render.\n";

}



sf::Text fpsText(font);

fpsText.setCharacterSize(18);

fpsText.setFillColor(sf::Color::White);

fpsText.setPosition({10, 10});



sf::Clock deltaClock;

sf::Clock fpsClock;

int frames = 0;



bool rotating = false;

sf::Vector2i lastMouse;



// Скрываем курсор для удобства управления

window.setMouseCursorVisible(true);



// ---------------- LOOP ----------------

while (window.isOpen()) {

while (auto event = window.pollEvent()) {

if (event->is<sf::Event::Closed>())

window.close();



if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {

if (key->code == sf::Keyboard::Key::Escape)

window.close();

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



// ---------------- LOOK ----------------

if (rotating) {

auto pos = sf::Mouse::getPosition(window);

float dx = (pos.x - lastMouse.x) * 0.003f;

float dy = (pos.y - lastMouse.y) * 0.003f;

camera.rotate(-dx, dy);

lastMouse = pos;

}



// ---------------- MOVE ----------------

float speed = 5.0f * dt;

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



// ---------------- PHYSICS ----------------

camera.updatePhysics(dt, world);



// ---------------- RENDER ----------------

renderer.clear(sf::Color(100, 149, 237)); // Красивое небо CornflowerBlue



Matrix4x4 view = camera.getViewMatrix();

// Направление света (чуть сверху и сбоку)

renderer.setLightDirection(Vector3(0.5f, -1.0f, 0.3f));



// Рисуем мир

world.draw(renderer, atlas, view, proj, camera.getPosition());



// Выводим результат рендеринга на экран

renderer.display(window);



// ---------------- UI & OVERLAY ----------------

frames++;

if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {

float actualFps = frames / fpsClock.restart().asSeconds();

fpsText.setString("FPS: " + std::to_string(int(actualFps)) +

" | Res: " + std::to_string(RENDER_W) + "x" + std::to_string(RENDER_H));

frames = 0;

}



window.draw(fpsText);

window.display(); // Финальный вывод SFML

}



return 0;

}