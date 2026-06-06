#include "BiomeManager.h"
#include "BlockRegistry.h"
#include <cmath>
#include <algorithm>

BiomeManager::BiomeManager() {
    m_defs[static_cast<size_t>(Biome::PLAINS)] = {
        "Plains", BlockType::GRASS, BlockType::DIRT, BlockType::STONE,
        true, 0.008f
    };
    m_defs[static_cast<size_t>(Biome::DESERT)] = {
        "Desert", BlockType::SAND, BlockType::SAND, BlockType::SANDSTONE,
        false, 0.0f
    };
    m_defs[static_cast<size_t>(Biome::SNOW)] = {
        "Snow", BlockType::SNOW, BlockType::SNOW_BLOCK, BlockType::STONE,
        true, 0.005f
    };
    m_defs[static_cast<size_t>(Biome::FOREST)] = {
        "Forest", BlockType::GRASS, BlockType::DIRT, BlockType::STONE,
        true, 0.04f
    };

    // Цвета тинтов (приблизительно как в Minecraft)
    // Формат: sf::Color(R, G, B)

    // Plains - яркий зелёный
    m_grassColors[static_cast<size_t>(Biome::PLAINS)]   = sf::Color(124, 189, 107);
    m_foliageColors[static_cast<size_t>(Biome::PLAINS)] = sf::Color(124, 189, 107);
    m_waterColors[static_cast<size_t>(Biome::PLAINS)]   = sf::Color(63, 118, 228);

    // Desert - жёлто-зелёный (песчаный)
    m_grassColors[static_cast<size_t>(Biome::DESERT)]   = sf::Color(217, 211, 87);
    m_foliageColors[static_cast<size_t>(Biome::DESERT)] = sf::Color(184, 224, 68);
    m_waterColors[static_cast<size_t>(Biome::DESERT)]   = sf::Color(63, 118, 228);

    // Snow - бледный, с зелёным оттенком
    m_grassColors[static_cast<size_t>(Biome::SNOW)]     = sf::Color(240, 240, 240);
    m_foliageColors[static_cast<size_t>(Biome::SNOW)]   = sf::Color(128, 176, 80);
    m_waterColors[static_cast<size_t>(Biome::SNOW)]     = sf::Color(59, 79, 181);

    // Forest - тёмный зелёный
    m_grassColors[static_cast<size_t>(Biome::FOREST)]   = sf::Color(85, 170, 85);
    m_foliageColors[static_cast<size_t>(Biome::FOREST)] = sf::Color(72, 181, 24);
    m_waterColors[static_cast<size_t>(Biome::FOREST)]   = sf::Color(63, 118, 228);
}

float BiomeManager::hash2D(int x, int z) const {
    int n = x + z * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float BiomeManager::noise2D(float x, float z) const {
    int ix = (int)std::floor(x);
    int iz = (int)std::floor(z);
    float fx = x - ix;
    float fz = z - iz;

    float v00 = hash2D(ix, iz);
    float v10 = hash2D(ix + 1, iz);
    float v01 = hash2D(ix, iz + 1);
    float v11 = hash2D(ix + 1, iz + 1);

    float sx = fx * fx * (3.0f - 2.0f * fx);
    float sz = fz * fz * (3.0f - 2.0f * fz);

    float a = v00 + sx * (v10 - v00);
    float b = v01 + sx * (v11 - v01);
    return a + sz * (b - a);
}

float BiomeManager::smoothNoise(float x, float z) const {
    float val = 0.0f;
    val += noise2D(x * 0.01f, z * 0.01f) * 1.0f;
    val += noise2D(x * 0.03f, z * 0.03f) * 0.5f;
    val += noise2D(x * 0.06f, z * 0.06f) * 0.25f;
    return val / 1.75f;
}

float BiomeManager::biomeNoise(float x, float z) const {
    return smoothNoise(x + 500.0f, z + 500.0f);
}

Biome BiomeManager::getBiome(int x, int z) const {
    float val = biomeNoise((float)x, (float)z);

    if (val < -0.25f) return Biome::DESERT;
    if (val < 0.1f)   return Biome::PLAINS;
    if (val < 0.35f)  return Biome::FOREST;
    return Biome::SNOW;
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
