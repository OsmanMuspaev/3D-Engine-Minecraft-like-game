#pragma once
#include "Block.h"
#include <array>

enum class Biome : unsigned int {
    PLAINS = 0,
    DESERT = 1,
    SNOW = 2,
    FOREST = 3,
    COUNT
};

struct BiomeDef {
    const char* name;
    BlockType surface;
    BlockType subsurface;
    BlockType stone;
    bool trees;
    float treeDensity;
};
