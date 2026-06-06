#include "BlockRegistry.h"
#include <iostream>

static const std::array<int, 6> NO_TINT = {TINT_NONE, TINT_NONE, TINT_NONE, TINT_NONE, TINT_NONE, TINT_NONE};
static const std::array<int, 6> ALL_GRASS = {TINT_GRASS, TINT_GRASS, TINT_GRASS, TINT_GRASS, TINT_GRASS, TINT_GRASS};
static const std::array<int, 6> ALL_FOLIAGE = {TINT_FOLIAGE, TINT_FOLIAGE, TINT_FOLIAGE, TINT_FOLIAGE, TINT_FOLIAGE, TINT_FOLIAGE};
static const std::array<int, 6> ALL_WATER = {TINT_WATER, TINT_WATER, TINT_WATER, TINT_WATER, TINT_WATER, TINT_WATER};

BlockRegistry& BlockRegistry::instance() {
    static BlockRegistry reg;
    return reg;
}

void BlockRegistry::init() {
    m_defs.clear();

    m_defs[BlockType::GRASS] = {
        "grass",
        {"grass_block_side", "grass_block_side",
         "grass_block_top",
         "dirt",
         "grass_block_side", "grass_block_side"},
        {TINT_NONE, TINT_NONE, TINT_GRASS, TINT_NONE, TINT_NONE, TINT_NONE},
        true, false
    };

    m_defs[BlockType::DIRT] = {
        "dirt",
        {"dirt", "dirt", "dirt", "dirt", "dirt", "dirt"},
        NO_TINT,
        true, false
    };

    m_defs[BlockType::STONE] = {
        "stone",
        {"stone", "stone", "stone", "stone", "stone", "stone"},
        NO_TINT,
        true, false
    };

    m_defs[BlockType::WOOD] = {
        "wood",
        {"oak_log", "oak_log",
         "oak_log_top", "oak_log_top",
         "oak_log", "oak_log"},
        NO_TINT,
        true, false
    };

    m_defs[BlockType::LEAVES] = {
        "leaves",
        {"oak_leaves", "oak_leaves", "oak_leaves",
         "oak_leaves", "oak_leaves", "oak_leaves"},
        ALL_FOLIAGE,
        true, true
    };

    m_defs[BlockType::AIR] = {
        "air",
        {"", "", "", "", "", ""},
        NO_TINT,
        false, true
    };

    m_defs[BlockType::SAND] = {
        "sand",
        {"sand", "sand", "sand", "sand", "sand", "sand"},
        NO_TINT,
        true, false
    };

    m_defs[BlockType::SNOW] = {
        "snow",
        {"snow", "snow", "snow", "dirt", "snow", "snow"},
        NO_TINT,
        true, false
    };

    m_defs[BlockType::BEDROCK] = {
        "bedrock",
        {"bedrock", "bedrock", "bedrock", "bedrock", "bedrock", "bedrock"},
        NO_TINT,
        true, false
    };

    m_defs[BlockType::WATER] = {
        "water",
        {"water", "water", "water", "water", "water", "water"},
        ALL_WATER,
        false, true
    };

    m_defs[BlockType::SANDSTONE] = {
        "sandstone",
        {"sandstone", "sandstone", "sandstone_top", "sandstone_bottom",
         "sandstone", "sandstone"},
        NO_TINT,
        true, false
    };

    m_defs[BlockType::SNOW_BLOCK] = {
        "snow_block",
        {"snow", "snow", "snow", "snow", "snow", "snow"},
        NO_TINT,
        true, false
    };

    std::cout << "BlockRegistry: initialized " << m_defs.size() << " block types\n";
}

const BlockDef& BlockRegistry::getDef(BlockType type) const {
    auto it = m_defs.find(type);
    if (it != m_defs.end()) {
        return it->second;
    }
    static const BlockDef empty = {"unknown", {"stone","stone","stone","stone","stone","stone"}, NO_TINT, true, false};
    return empty;
}

std::array<unsigned int, 6> BlockRegistry::getBlockTextures(
    const TextureManager& texMgr, BlockType type) const
{
    const auto& def = getDef(type);
    std::array<unsigned int, 6> texs;
    for (int i = 0; i < 6; i++) {
        texs[i] = texMgr.getTileIndex(def.textures[i]);
    }
    return texs;
}
