#pragma once
#include <map>
#include <memory>
#include <tuple>
#include "Chunk.h"

class World {
public:
    World();
    
    void setBlock(int x, int y, int z, BlockType type);
    Block getBlock(int x, int y, int z) const;
    
    void generate(int radiusX, int radiusZ); // Генерация в чанках
    
    void draw(Renderer& renderer, const TextureAtlas& atlas,
              const Matrix4x4& view, const Matrix4x4& proj,
              const Vector3& cameraPos) const;
    
    bool isBlocking(float worldX, float worldY, float worldZ) const;

private:
    std::map<std::tuple<int, int, int>, std::unique_ptr<Chunk>> m_chunks;
    
    void createTree(int x, int y, int z);
};