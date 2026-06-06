#pragma once
#include "Biome.h"
#include "Noise.h"
#include <SFML/Graphics.hpp>
#include <array>

class BiomeManager {
public:
    BiomeManager();

    Biome getBiome(int x, int z) const;
    const BiomeDef& getDef(Biome biome) const;
    sf::Color getTintColor(Biome biome, int tintType) const;
    float getBaseHeight(int x, int z) const;

private:
    std::array<BiomeDef, static_cast<size_t>(Biome::COUNT)> m_defs;
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_grassColors;
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_foliageColors;
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_waterColors;

    Noise m_noise;
};
