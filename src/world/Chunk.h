#pragma once
#include <vector>
#include <array>
#include "Block.h"
#include "../math/Vector3.h"
#include "../renderer/Renderer.h"
#include "TextureManager.h"

class World;

class Chunk {
public:
    static const int SIZE = 16;

    // Initializes a chunk at the given chunk coordinates with all-air blocks.
    Chunk(int cx, int cy, int cz);

    // Sets the block type at local coordinates and marks the chunk dirty.
    void setBlock(int x, int y, int z, BlockType type);
    // Returns the block at local coordinates.
    Block getBlock(int x, int y, int z) const;

    // Rebuilds the chunk mesh from scratch using the current block data.
    void buildMesh(const TextureManager& texMgr, const World& world);
    // Draws the chunk mesh if it has geometry.
    void draw(Renderer& renderer, const TextureManager& texMgr,
              const Matrix4x4& view, const Matrix4x4& proj, const Vector3& cameraPos) const;

    bool isDirty() const { return m_dirty; }
    void markDirty() { m_dirty = true; }

    int getCX() const { return m_cx; }
    int getCY() const { return m_cy; }
    int getCZ() const { return m_cz; }

    void setDirty(bool dirty) {
        m_dirty = dirty;
        if (dirty) m_meshBuilt = false;
    }

private:
    int m_cx, m_cy, m_cz;
    std::vector<Block> m_blocks;
    bool m_dirty = true;
    bool m_meshBuilt = false;

    mutable std::vector<Vector3> m_meshVerts;
    mutable std::vector<unsigned int> m_meshIndices;
    mutable std::vector<sf::Vector2f> m_meshUVs;
    mutable std::vector<sf::Color> m_meshTints;

    // Converts local (x, y, z) to a flat array index.
    inline int index(int x, int y, int z) const {
        return (y * SIZE + z) * SIZE + x;
    }

    // Appends quad indices, UVs, and tint data for one face.
    void addFaceData(const std::array<sf::Vector2f, 4>& faceUVs, const sf::Color& tintColor) const;
};
