#pragma once
#include <vector>
#include <array>
#include "Block.h"
#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"
#include "../renderer/Renderer.h"
#include "TextureAtlas.h"

class World {
public:
    World(int sizeX, int sizeY, int sizeZ);
    
    void setBlock(int x, int y, int z, BlockType type);
    Block getBlock(int x, int y, int z) const;
    
    void buildMesh(const TextureAtlas& atlas) const;
    
    void draw(Renderer& renderer, const TextureAtlas& atlas,
              const Matrix4x4& view, const Matrix4x4& proj,
              const Vector3& cameraPos) const;
    
    // Геттеры для GPU Рендерера
    const std::vector<Vector3>& getVertices() const { return m_meshVerts; }
    const std::vector<unsigned int>& getIndices() const { return m_meshIndices; }
    const std::vector<sf::Vector2f>& getUVs() const { return m_meshUVs; }

    int getSizeX() const { return m_sizeX; }
    int getSizeY() const { return m_sizeY; }
    int getSizeZ() const { return m_sizeZ; }
    
    bool isBlocking(float worldX, float worldY, float worldZ) const;
    void generate();
    
private:
    int m_sizeX, m_sizeY, m_sizeZ;
    std::vector<Block> m_blocks; 
    
    mutable std::vector<Vector3> m_meshVerts;
    mutable std::vector<unsigned int> m_meshIndices;
    mutable std::vector<sf::Vector2f> m_meshUVs;
    mutable bool m_meshDirty = true;
    
    inline int index(int x, int y, int z) const {
        return (y * m_sizeZ + z) * m_sizeX + x;
    }
    
    bool hasNeighbor(int x, int y, int z) const;
    void createTree(int x, int y, int z);
    void addFaceData(const std::array<sf::Vector2f, 4>& faceUVs) const;
};