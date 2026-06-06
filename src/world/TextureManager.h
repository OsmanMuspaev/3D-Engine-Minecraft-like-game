#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <array>

enum FaceSide {
    FACE_PX = 0,
    FACE_NX = 1,
    FACE_PY = 2,
    FACE_NY = 3,
    FACE_PZ = 4,
    FACE_NZ = 5
};

class TextureManager {
public:
    TextureManager() = default;

    bool loadFromDirectory(const std::string& assetsPath, unsigned int tileSize = 16);

    unsigned int getTileIndex(const std::string& textureName) const;

    std::array<sf::Vector2f, 4> getUV(unsigned int tileIndex) const;

    const sf::Image& getImage() const { return m_atlasImage; }
    unsigned int getTileSize() const { return m_tileSize; }
    unsigned int getTilesPerRow() const { return m_tilesPerRow; }

private:
    sf::Image m_atlasImage;
    unsigned int m_tileSize = 16;
    unsigned int m_tilesPerRow = 0;
    bool m_loaded = false;

    std::unordered_map<std::string, unsigned int> m_nameToIndex;

    std::string m_blockTexturePath;

    bool buildAtlas(const std::vector<std::pair<std::string, sf::Image>>& images);
};
