#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class TextureAtlas {
public:
    TextureAtlas() = default;
    
    bool loadFromFile(const std::string& path, unsigned int tileSize);
    void loadFromTexture(const sf::Texture& texture, unsigned int tileSize);  // ← ДОЛЖНА БЫТЬ ЭТА СТРОКА
    
    std::vector<sf::Vector2f> getUV(unsigned int tileIndex) const;
    
    const sf::Texture& getTexture() const { return m_texture; }
    bool isLoaded() const { return m_loaded; }
    
private:
    sf::Texture m_texture;
    unsigned int m_tileSize = 0;
    unsigned int m_tilesPerRow = 0;
    bool m_loaded = false;
};