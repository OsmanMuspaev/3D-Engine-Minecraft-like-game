#include "TextureAtlas.h"

bool TextureAtlas::loadFromFile(const std::string& path, unsigned int tileSize) {
    if (!m_texture.loadFromFile(path)) return false;
    m_texture.setSmooth(false);
    m_tileSize = tileSize;
    m_tilesPerRow = m_texture.getSize().x / tileSize;
    m_loaded = true;
    return true;
}

std::vector<sf::Vector2f> TextureAtlas::getUV(unsigned int tileIndex) const {
    if (!m_loaded) return {{0,1}, {1,1}, {1,0}, {0,0}};
    
    unsigned int col = tileIndex % m_tilesPerRow;
    unsigned int row = tileIndex / m_tilesPerRow;
    
    float texW = static_cast<float>(m_texture.getSize().x);
    float texH = static_cast<float>(m_texture.getSize().y);
    float ts = static_cast<float>(m_tileSize);
    
    float u0 = col * ts;
    float v0 = row * ts;

    float u1 = (col + 1) * ts;
    float v1 = (row + 1) * ts;

    return {
        {u0, v1},  // лево-низ
        {u1, v1},  // право-низ
        {u1, v0},  // право-верх
        {u0, v0},  // лево-верх
    };
}

void TextureAtlas::loadFromTexture(const sf::Texture& texture, unsigned int tileSize) {
    m_texture = texture;
    m_texture.setSmooth(false);
    m_tileSize = tileSize;
    m_tilesPerRow = m_texture.getSize().x / tileSize;
    m_loaded = true;
}