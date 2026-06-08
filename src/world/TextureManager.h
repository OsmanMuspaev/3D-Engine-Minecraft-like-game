#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>
#include <array>

// Face side indices matching the texture order in BlockDef
enum FaceSide {
    FACE_PX = 0,
    FACE_NX = 1,
    FACE_PY = 2,
    FACE_NY = 3,
    FACE_PZ = 4,
    FACE_NZ = 5
};

// Manages block and item texture atlases built from individual PNG files
class TextureManager {
public:
    TextureManager() = default;

    // Loads all block textures from a directory and builds an atlas
    bool loadFromDirectory(const std::string& assetsPath, unsigned int tileSize = 16);
    // Loads all item textures from a directory and builds an item atlas
    bool loadItemTextures(const std::string& assetsPath);

    // Returns the tile index for a named block texture in the atlas
    unsigned int getTileIndex(const std::string& textureName) const;

    // Returns UV coordinates for a given tile index in the block atlas
    std::array<sf::Vector2f, 4> getUV(unsigned int tileIndex) const;

    const sf::Image& getImage() const { return m_atlasImage; }
    unsigned int getTileSize() const { return m_tileSize; }
    unsigned int getTilesPerRow() const { return m_tilesPerRow; }

    // Returns the tile index for a named item texture in the item atlas
    unsigned int getItemTileIndex(const std::string& textureName) const;
    // Returns UV coordinates for a given tile index in the item atlas
    std::array<sf::Vector2f, 4> getItemUV(unsigned int tileIndex) const;
    const sf::Image& getItemImage() const { return m_itemAtlasImage; }

private:
    sf::Image m_atlasImage;
    sf::Image m_itemAtlasImage;
    unsigned int m_tileSize = 16;
    unsigned int m_tilesPerRow = 0;
    unsigned int m_itemTilesPerRow = 0;
    bool m_loaded = false;

    std::unordered_map<std::string, unsigned int> m_nameToIndex;
    std::unordered_map<std::string, unsigned int> m_itemNameToIndex;

    std::string m_blockTexturePath;

    // Assembles individual images into a single block texture atlas
    bool buildAtlas(const std::vector<std::pair<std::string, sf::Image>>& images);
    // Assembles individual images into a single item texture atlas
    bool buildItemAtlas(const std::vector<std::pair<std::string, sf::Image>>& images);
};
