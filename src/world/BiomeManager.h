#pragma once
#include "Biome.h"
#include <SFML/Graphics.hpp>
#include <array>

class BiomeManager {
public:
    BiomeManager();

    Biome getBiome(int x, int z) const;
    const BiomeDef& getDef(Biome biome) const;

    // Возвращает цвет тинта для данного биома и типа тинтинга
    sf::Color getTintColor(Biome biome, int tintType) const;

private:
    std::array<BiomeDef, static_cast<size_t>(Biome::COUNT)> m_defs;

    // Цвета для каждого биома: {grass, foliage, water}
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_grassColors;
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_foliageColors;
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_waterColors;

    float hash2D(int x, int z) const;
    float noise2D(float x, float z) const;
    float smoothNoise(float x, float z) const;
    float biomeNoise(float x, float z) const;
};
