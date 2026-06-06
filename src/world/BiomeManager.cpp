#include "BiomeManager.h"
#include "BlockRegistry.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>

BiomeManager::BiomeManager() {
    std::random_device rd;
    unsigned int seed = rd();
    m_noise = Noise(seed);
    std::cout << "World seed: " << seed << "\n";

    m_defs[static_cast<size_t>(Biome::PLAINS)] = {
        "Plains", BlockType::GRASS, BlockType::DIRT, BlockType::STONE, true, 0.008f
    };
    m_defs[static_cast<size_t>(Biome::DESERT)] = {
        "Desert", BlockType::SAND, BlockType::SAND, BlockType::SANDSTONE, false, 0.0f
    };
    m_defs[static_cast<size_t>(Biome::SNOW)] = {
        "Snow", BlockType::SNOW, BlockType::SNOW_BLOCK, BlockType::STONE, true, 0.005f
    };
    m_defs[static_cast<size_t>(Biome::FOREST)] = {
        "Forest", BlockType::GRASS, BlockType::DIRT, BlockType::STONE, true, 0.04f
    };

    m_grassColors[static_cast<size_t>(Biome::PLAINS)]   = sf::Color(124, 189, 107);
    m_foliageColors[static_cast<size_t>(Biome::PLAINS)] = sf::Color(124, 189, 107);
    m_waterColors[static_cast<size_t>(Biome::PLAINS)]   = sf::Color(63, 118, 228);

    m_grassColors[static_cast<size_t>(Biome::DESERT)]   = sf::Color(217, 211, 87);
    m_foliageColors[static_cast<size_t>(Biome::DESERT)] = sf::Color(184, 224, 68);
    m_waterColors[static_cast<size_t>(Biome::DESERT)]   = sf::Color(63, 118, 228);

    m_grassColors[static_cast<size_t>(Biome::SNOW)]     = sf::Color(240, 240, 240);
    m_foliageColors[static_cast<size_t>(Biome::SNOW)]   = sf::Color(128, 176, 80);
    m_waterColors[static_cast<size_t>(Biome::SNOW)]     = sf::Color(59, 79, 181);

    m_grassColors[static_cast<size_t>(Biome::FOREST)]   = sf::Color(85, 170, 85);
    m_foliageColors[static_cast<size_t>(Biome::FOREST)] = sf::Color(72, 181, 24);
    m_waterColors[static_cast<size_t>(Biome::FOREST)]   = sf::Color(63, 118, 228);
}

Biome BiomeManager::getBiome(int x, int z) const {
    float temp = m_noise.temperature((float)x, (float)z);
    float moist = m_noise.moisture((float)x, (float)z);

    if (temp < -0.1f)  return Biome::SNOW;
    if (temp > 0.3f)   return Biome::DESERT;
    if (moist > 0.15f) return Biome::FOREST;
    return Biome::PLAINS;
}

const BiomeDef& BiomeManager::getDef(Biome biome) const {
    return m_defs[static_cast<size_t>(biome)];
}

sf::Color BiomeManager::getTintColor(Biome biome, int tintType) const {
    size_t idx = static_cast<size_t>(biome);
    switch (tintType) {
        case TINT_GRASS:   return m_grassColors[idx];
        case TINT_FOLIAGE: return m_foliageColors[idx];
        case TINT_WATER:   return m_waterColors[idx];
        default:           return sf::Color::White;
    }
}

float BiomeManager::getBaseHeight(int x, int z) const {
    float height = m_noise.fbm((float)x * 0.01f, (float)z * 0.01f, 6, 2.0f, 0.5f);
    float detail = m_noise.fbm((float)x * 0.05f, (float)z * 0.05f, 3, 2.0f, 0.5f) * 0.3f;

    float h = 10.0f + height * 6.0f + detail * 2.0f;

    Biome biome = getBiome(x, z);

    if (biome == Biome::DESERT)  h = 8.0f + height * 2.0f + detail * 1.0f;
    if (biome == Biome::FOREST)  h = 10.0f + height * 5.0f + detail * 3.0f;
    if (biome == Biome::SNOW)    h = 11.0f + height * 5.0f + detail * 2.0f;

    return std::max(1.0f, h);
}
