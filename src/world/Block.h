#pragma once
#include <array>

enum class BlockType : unsigned int {
    AIR = 0,
    GRASS = 1,
    DIRT = 2,
    STONE = 3,
    WOOD = 4,
    LEAVES = 5,
    SAND = 6,
    SNOW = 7,
    BEDROCK = 8,
    WATER = 9,
    SANDSTONE = 10,
    SNOW_BLOCK = 11
};

struct Block {
    BlockType type = BlockType::AIR;

    Block() : type(BlockType::AIR) {}
    Block(BlockType t) : type(t) {}

    bool isSolid() const {
        return type != BlockType::AIR && type != BlockType::WATER;
    }
};
