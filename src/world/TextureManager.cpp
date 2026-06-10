#include "TextureManager.h"
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;

bool TextureManager::loadFromDirectory(const std::string& assetsPath, unsigned int tileSize) {
    m_tileSize = tileSize;
    m_blockTexturePath = assetsPath + "/minecraft/textures/block/";

    if (!fs::exists(m_blockTexturePath) || !fs::is_directory(m_blockTexturePath)) {
        std::cerr << "Block texture directory not found: " << m_blockTexturePath << "\n";
        return false;
    }

    std::vector<std::pair<std::string, fs::path>> textureFiles;
    for (const auto& entry : fs::directory_iterator(m_blockTexturePath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            std::string name = entry.path().stem().string();
            textureFiles.push_back({name, entry.path()});
        }
    }

    std::sort(textureFiles.begin(), textureFiles.end());
    std::cout << "Found " << textureFiles.size() << " block textures\n";

    std::vector<std::pair<std::string, sf::Image>> images;
    for (const auto& [name, path] : textureFiles) {
        sf::Image img;
        if (!img.loadFromFile(path.string())) {
            std::cerr << "Failed to load texture: " << path << "\n";
            continue;
        }
        images.push_back({name, std::move(img)});
    }

    return buildAtlas(images);
}

bool TextureManager::loadItemTextures(const std::string& assetsPath) {
    std::string itemTexturePath = assetsPath + "/minecraft/textures/item/";

    if (!fs::exists(itemTexturePath) || !fs::is_directory(itemTexturePath)) {
        std::cerr << "Item texture directory not found: " << itemTexturePath << "\n";
        return false;
    }

    std::vector<std::pair<std::string, fs::path>> textureFiles;
    for (const auto& entry : fs::directory_iterator(itemTexturePath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
            std::string name = entry.path().stem().string();
            textureFiles.push_back({name, entry.path()});
        }
    }

    std::sort(textureFiles.begin(), textureFiles.end());
    std::cout << "Found " << textureFiles.size() << " item textures\n";

    std::vector<std::pair<std::string, sf::Image>> images;
    for (const auto& [name, path] : textureFiles) {
        sf::Image img;
        if (!img.loadFromFile(path.string())) {
            std::cerr << "Failed to load item texture: " << path << "\n";
            continue;
        }
        images.push_back({name, std::move(img)});
    }

    return buildItemAtlas(images);
}

bool TextureManager::buildItemAtlas(const std::vector<std::pair<std::string, sf::Image>>& images) {
    if (images.empty()) return false;

    m_itemNameToIndex.clear();

    unsigned int tilesPerRow = 16;
    m_itemTilesPerRow = tilesPerRow;

    unsigned int numTiles = static_cast<unsigned int>(images.size());
    unsigned int numRows = (numTiles + tilesPerRow - 1) / tilesPerRow;

    unsigned int atlasW = tilesPerRow * m_tileSize;
    unsigned int atlasH = numRows * m_tileSize;

    m_itemAtlasImage.resize({atlasW, atlasH}, sf::Color::Black);

    for (unsigned int i = 0; i < numTiles; i++) {
        const auto& [name, img] = images[i];

        unsigned int col = i % tilesPerRow;
        unsigned int row = i / tilesPerRow;

        unsigned int dstX = col * m_tileSize;
        unsigned int dstY = row * m_tileSize;

        sf::Vector2u srcSize = img.getSize();
        for (unsigned int y = 0; y < m_tileSize && y < srcSize.y; y++) {
            for (unsigned int x = 0; x < m_tileSize && x < srcSize.x; x++) {
                m_itemAtlasImage.setPixel({dstX + x, dstY + y}, img.getPixel({x, y}));
            }
        }

        m_itemNameToIndex[name] = i;
    }

    std::cout << "Item atlas built: " << atlasW << "x" << atlasH << " (" << numTiles << " tiles)\n";
    return true;
}

unsigned int TextureManager::getItemTileIndex(const std::string& textureName) const {
    if (textureName.empty()) return 0;
    auto it = m_itemNameToIndex.find(textureName);
    if (it != m_itemNameToIndex.end()) {
        return it->second;
    }
    std::cerr << "Item texture not found: '" << textureName << "'\n";
    return 0;
}

std::array<sf::Vector2f, 4> TextureManager::getItemUV(unsigned int tileIndex) const {
    if (m_itemTilesPerRow == 0) {
        return {
            sf::Vector2f(0.0f, 1.0f),
            sf::Vector2f(1.0f, 1.0f),
            sf::Vector2f(1.0f, 0.0f),
            sf::Vector2f(0.0f, 0.0f)
        };
    }

    unsigned int col = tileIndex % m_itemTilesPerRow;
    unsigned int row = tileIndex / m_itemTilesPerRow;

    float ts = static_cast<float>(m_tileSize);
    float u0 = col * ts;
    float v0 = row * ts;
    float u1 = u0 + ts;
    float v1 = v0 + ts;

    return {
        sf::Vector2f(u0, v1),
        sf::Vector2f(u1, v1),
        sf::Vector2f(u1, v0),
        sf::Vector2f(u0, v0)
    };
}

bool TextureManager::buildAtlas(const std::vector<std::pair<std::string, sf::Image>>& images) {
    if (images.empty()) return false;

    m_nameToIndex.clear();

    unsigned int tilesPerRow = 16;
    m_tilesPerRow = tilesPerRow;

    unsigned int numTiles = static_cast<unsigned int>(images.size());
    unsigned int numRows = (numTiles + tilesPerRow - 1) / tilesPerRow;

    unsigned int atlasW = tilesPerRow * m_tileSize;
    unsigned int atlasH = numRows * m_tileSize;

    m_atlasImage.resize({atlasW, atlasH}, sf::Color::Black);

    for (unsigned int i = 0; i < numTiles; i++) {
        const auto& [name, img] = images[i];

        unsigned int col = i % tilesPerRow;
        unsigned int row = i / tilesPerRow;

        unsigned int dstX = col * m_tileSize;
        unsigned int dstY = row * m_tileSize;

        sf::Vector2u srcSize = img.getSize();
        for (unsigned int y = 0; y < m_tileSize && y < srcSize.y; y++) {
            for (unsigned int x = 0; x < m_tileSize && x < srcSize.x; x++) {
                m_atlasImage.setPixel({dstX + x, dstY + y}, img.getPixel({x, y}));
            }
        }

        m_nameToIndex[name] = i;
    }

    m_loaded = true;
    std::cout << "Atlas built: " << atlasW << "x" << atlasH << " (" << numTiles << " tiles)\n";
    return true;
}

unsigned int TextureManager::getTileIndex(const std::string& textureName) const {
    if (textureName.empty()) return 0;
    auto it = m_nameToIndex.find(textureName);
    if (it != m_nameToIndex.end()) {
        return it->second;
    }
    std::cerr << "Texture not found: '" << textureName << "'\n";
    return 0;
}

std::array<sf::Vector2f, 4> TextureManager::getUV(unsigned int tileIndex) const {
    if (!m_loaded || m_tilesPerRow == 0) {
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
    float u0 = col * ts;
    float v0 = row * ts;
    float u1 = u0 + ts;
    float v1 = v0 + ts;

    return {
        sf::Vector2f(u0, v1),
        sf::Vector2f(u1, v1),
        sf::Vector2f(u1, v0),
        sf::Vector2f(u0, v0)
    };
}
