#include "MenuWidget.h"
#include <algorithm>

Button::Button(const std::string& text, sf::Font& font, float x, float y, float w, float h)
    : label(font)
{
    shape.setPosition({x, y});
    shape.setSize({w, h});
    shape.setFillColor(normalColor);
    shape.setOutlineColor(sf::Color(100, 100, 100));
    shape.setOutlineThickness(2.0f);

    label.setString(text);
    label.setCharacterSize(static_cast<unsigned int>(h * 0.4f));
    label.setFillColor(sf::Color::White);

    auto lb = label.getLocalBounds();
    label.setOrigin({lb.position.x + lb.size.x / 2.0f, lb.position.y + lb.size.y / 2.0f});
    label.setPosition({x + w / 2.0f, y + h / 2.0f});
}

void Button::update(const sf::Vector2f& mousePos) {
    isHovered = shape.getGlobalBounds().contains(mousePos);
    shape.setFillColor(isHovered ? hoverColor : normalColor);
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(shape);
    window.draw(label);
}

bool Button::contains(const sf::Vector2f& point) const {
    return shape.getGlobalBounds().contains(point);
}

TextInput::TextInput(const std::string& labelText, sf::Font& font, float x, float y, float w)
    : label(font), value(font)
{
    box.setPosition({x, y + 25.0f});
    box.setSize({w, 35.0f});
    box.setFillColor(sf::Color(40, 40, 40));
    box.setOutlineColor(sf::Color(100, 100, 100));
    box.setOutlineThickness(2.0f);

    label.setString(labelText);
    label.setCharacterSize(14);
    label.setFillColor(sf::Color(200, 200, 200));
    label.setPosition({x, y});

    value.setCharacterSize(16);
    value.setFillColor(sf::Color::White);
    value.setPosition({x + 8.0f, y + 32.0f});

    cursor.setSize({2.0f, 20.0f});
    cursor.setFillColor(sf::Color::White);
    cursor.setPosition({x + 8.0f, y + 35.0f});
}

void TextInput::update(float dt, const sf::Vector2f& mousePos, bool clicked) {
    if (clicked) {
        if (box.getGlobalBounds().contains(mousePos)) {
            focused = true;
        } else {
            focused = false;
        }
    }

    if (focused) {
        cursorTimer += dt;
        if (cursorTimer >= 1.0f) cursorTimer -= 1.0f;
    }
}

void TextInput::draw(sf::RenderWindow& window) {
    window.draw(box);
    window.draw(label);
    window.draw(value);
    if (focused) {
        float cursorX = value.findCharacterPos(static_cast<unsigned int>(text.size())).x;
        cursor.setPosition({cursorX + 2.0f, box.getPosition().y + 7.0f});
        if (static_cast<int>(cursorTimer * 2.0f) % 2 == 0) {
            window.draw(cursor);
        }
    }
}

void TextInput::handleEvent(const sf::Event& event) {
    if (!focused) return;

    if (const auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
        if (textEvent->unicode == 8) {
            if (!text.empty()) text.pop_back();
        } else if (textEvent->unicode == 13) {
            focused = false;
        } else if (textEvent->unicode >= 32 && textEvent->unicode < 128) {
            text += static_cast<char>(textEvent->unicode);
        }
        value.setString(text);
    }
}

void TextInput::setFocus(bool f) {
    focused = f;
    cursorTimer = 0.0f;
}

Label::Label(const std::string& str, sf::Font& font, float x, float y, unsigned int size)
    : text(font)
{
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(sf::Color::White);
    text.setPosition({x, y});
}

void Label::draw(sf::RenderWindow& window) {
    window.draw(text);
}
