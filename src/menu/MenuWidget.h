#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

struct MenuButton {
    std::string text;
    sf::Vector2f position;
    sf::Vector2f size;
    int actionId;
};

struct Button {
    sf::RectangleShape shape;
    sf::Text label;
    sf::Color normalColor = sf::Color(60, 60, 60);
    sf::Color hoverColor = sf::Color(80, 80, 80);
    sf::Color clickColor = sf::Color(50, 50, 50);
    bool isHovered = false;
    std::function<void()> onClick;

    Button() = delete;

    Button(const std::string& text, sf::Font& font, float x, float y, float w, float h);

    void update(const sf::Vector2f& mousePos);
    void draw(sf::RenderWindow& window);
    bool contains(const sf::Vector2f& point) const;
};

struct TextInput {
    sf::RectangleShape box;
    sf::Text label;
    sf::Text value;
    sf::RectangleShape cursor;
    std::string text;
    bool focused = false;
    float cursorTimer = 0.0f;

    TextInput() = delete;

    TextInput(const std::string& labelText, sf::Font& font, float x, float y, float w);

    void update(float dt, const sf::Vector2f& mousePos, bool clicked);
    void draw(sf::RenderWindow& window);
    void handleEvent(const sf::Event& event);
    void setFocus(bool f);
};

struct Label {
    sf::Text text;

    Label() = delete;
    Label(const std::string& str, sf::Font& font, float x, float y, unsigned int size = 16);
    void draw(sf::RenderWindow& window);
};
