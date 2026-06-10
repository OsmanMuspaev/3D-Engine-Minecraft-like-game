#pragma once
#include "Block.h"
#include "TextureManager.h"
#include <string>
#include <array>
#include <unordered_map>

enum TintType : int {
    TINT_NONE = 0,
    TINT_GRASS = 1,
    TINT_FOLIAGE = 2,
    TINT_WATER = 3
};

struct BlockDef {
    std::string name;
    std::array<std::string, 6> textures;
    std::array<int, 6> tintIndex;
    bool solid = true;
    bool transparent = false;
};

class BlockRegistry {
public:
    static BlockRegistry& instance();

    // Initializes all block definitions.
    void init();
    // Looks up the BlockDef for a given block type.
    const BlockDef& getDef(BlockType type) const;
    // Returns the six atlas tile indices for the block's face textures.
    std::array<unsigned int, 6> getBlockTextures(const TextureManager& texMgr, BlockType type) const;

    static bool isTransparent(BlockType type) {
        switch (type) {
            case BlockType::LEAVES:
            case BlockType::GLASS:
            case BlockType::AIR:
                return true;
            default:
                return false;
        }
    }

    static bool isOpaque(BlockType type) {
        return !isTransparent(type) && type != BlockType::AIR;
    }

private:
    BlockRegistry() = default;
    std::unordered_map<BlockType, BlockDef> m_defs;
};
