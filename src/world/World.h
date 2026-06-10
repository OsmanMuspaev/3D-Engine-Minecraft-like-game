#pragma once
#include <map>
#include <memory>
#include <tuple>
#include "Chunk.h"
#include "BiomeManager.h"

class World {
public:
    World();

    void setBlock(int x, int y, int z, BlockType type);
    Block getBlock(int x, int y, int z) const;

    void generate(int radiusX, int radiusZ);

    void draw(Renderer& renderer, const TextureManager& texMgr,
              const Matrix4x4& view, const Matrix4x4& proj,
              const Vector3& cameraPos) const;

    bool isBlocking(float worldX, float worldY, float worldZ) const;

    Biome getBiome(int x, int z) const;
    const BiomeManager& getBiomeManager() const { return m_biomeMgr; }
    BiomeManager& getBiomeManager() { return m_biomeMgr; }
    const std::map<std::tuple<int, int, int>, std::unique_ptr<Chunk>>& getChunks() const { return m_chunks; }

private:
    std::map<std::tuple<int, int, int>, std::unique_ptr<Chunk>> m_chunks;
    BiomeManager m_biomeMgr;

    void createTree(int x, int y, int z);
};
