#pragma once
#include <vector>
#include <array>
#include "Block.h"
#include "../math/Vector3.h"
#include "../renderer/Renderer.h"
#include "TextureAtlas.h"

class World; // Предварительное объявление

class Chunk {
public:
    static const int SIZE = 16; // Размер 16x16x16

    Chunk(int cx, int cy, int cz);

    void setBlock(int x, int y, int z, BlockType type);
    Block getBlock(int x, int y, int z) const;
    
    void buildMesh(const TextureAtlas& atlas, const World& world);
    void draw(Renderer& renderer, const TextureAtlas& atlas, 
              const Matrix4x4& view, const Matrix4x4& proj, const Vector3& cameraPos) const;

    bool isDirty() const { return m_dirty; }
    void markDirty() { m_dirty = true; }

    int getCX() const { return m_cx; }
    int getCY() const { return m_cy; }
    int getCZ() const { return m_cz; }

private:
    int m_cx, m_cy, m_cz; 
    std::vector<Block> m_blocks;
    bool m_dirty = true;

    mutable std::vector<Vector3> m_meshVerts;
    mutable std::vector<unsigned int> m_meshIndices;
    mutable std::vector<sf::Vector2f> m_meshUVs;

    inline int index(int x, int y, int z) const {
        return (y * SIZE + z) * SIZE + x;
    }
    void addFaceData(const std::array<sf::Vector2f, 4>& faceUVs) const;
};