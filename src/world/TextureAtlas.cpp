#include "TextureAtlas.h"

bool TextureAtlas::loadFromFile(const std::string& path, unsigned int tileSize) {
    if (!m_texture.loadFromFile(path)) return false;
    
    m_texture.setSmooth(false); 
    m_image = m_texture.copyToImage(); 
    m_tileSize    = tileSize;
    m_tilesPerRow = m_texture.getSize().x / tileSize;
    m_loaded      = true;
    return true;
}

void TextureAtlas::loadFromTexture(const sf::Texture& texture, unsigned int tileSize) {
    m_texture = texture;
    m_texture.setSmooth(false);
    m_image = m_texture.copyToImage(); 
    m_tileSize    = tileSize;
    m_tilesPerRow = m_texture.getSize().x / tileSize;
    m_loaded      = true;
}

std::array<sf::Vector2f, 4> TextureAtlas::getUV(unsigned int tileIndex) const {
    // Если атлас не загружен, отдаем дефолтные пиксельные координаты (0-1, просто чтобы не упало)
    if (!m_loaded) {
        return { 
            sf::Vector2f(0.0f, 1.0f), 
            sf::Vector2f(1.0f, 1.0f), 
            sf::Vector2f(1.0f, 0.0f), 
            sf::Vector2f(0.0f, 0.0f) 
        };
    }

    unsigned int col = tileIndex % m_tilesPerRow;
    unsigned int row = tileIndex / m_tilesPerRow;

    float ts = static_cast<float>(m_tileSize);

    // Считаем строго в пикселях
    float u0 = col * ts;
    float v0 = row * ts;
    float u1 = u0 + ts;
    float v1 = v0 + ts;

    // Порядок: лево-низ, право-низ, право-верх, лево-верх
    return { 
        sf::Vector2f(u0, v1), 
        sf::Vector2f(u1, v1), 
        sf::Vector2f(u1, v0), 
        sf::Vector2f(u0, v0) 
    };
}