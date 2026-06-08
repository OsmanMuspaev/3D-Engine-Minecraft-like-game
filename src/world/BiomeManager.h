#pragma once
#include "Biome.h"
#include "Noise.h"
#include <SFML/Graphics.hpp>
#include <array>

// Manages biome definitions, selection, tint colors, and terrain height
class BiomeManager {
public:
    BiomeManager();

    // Determines the biome at the given world coordinates
    Biome getBiome(int x, int z) const;
    const BiomeDef& getDef(Biome biome) const;
    // Returns the tint color for a biome and tint type (grass, foliage, water)
    sf::Color getTintColor(Biome biome, int tintType) const;
    // Computes the terrain height at the given world coordinates
    float getBaseHeight(int x, int z) const;

private:
    std::array<BiomeDef, static_cast<size_t>(Biome::COUNT)> m_defs;
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_grassColors;
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_foliageColors;
    std::array<sf::Color, static_cast<size_t>(Biome::COUNT)> m_waterColors;

    Noise m_noise;
};
