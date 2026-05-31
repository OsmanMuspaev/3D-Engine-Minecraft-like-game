#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <string>

class TextureAtlas {
public:
    // Загрузка атласа из файла
    bool loadFromFile(const std::string& path, unsigned int tileSize);
    
    // Загрузка атласа из существующей текстуры
    void loadFromTexture(const sf::Texture& texture, unsigned int tileSize);

    // Возвращает массив из 4 UV-координат в ПИКСЕЛЯХ
    std::array<sf::Vector2f, 4> getUV(unsigned int tileIndex) const;

    const sf::Texture& getTexture() const { return m_texture; }
    const sf::Image&   getImage()   const { return m_image; }

private:
    sf::Texture  m_texture;
    sf::Image    m_image;       
    unsigned int m_tileSize    = 0;
    unsigned int m_tilesPerRow = 0;
    bool         m_loaded      = false;
};