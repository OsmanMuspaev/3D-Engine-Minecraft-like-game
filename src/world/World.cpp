#include "World.h"
#include <algorithm>
#include <cmath>
#include <ctime>

// Вспомогательная функция шума для генерации ландшафта
static float generateHeight(int x, int z) {
    float fx = (float)x;
    float fz = (float)z;

    // Слой 1: Большие холмы
    float h1 = (std::sin(fx * 0.05f) + std::cos(fz * 0.05f)) * 4.0f;
    // Слой 2: Мелкие неровности (кочки)
    float h2 = (std::sin(fx * 0.2f) * std::cos(fz * 0.2f)) * 1.5f;
    // Слой 3: Случайный "шум"
    float h3 = (std::sin(fx * 0.5f) + std::cos(fz * 0.5f)) * 0.5f;

    return h1 + h2 + h3 + 8.0f; // 8.0f - базовый уровень земли
}

World::World(int sizeX, int sizeY, int sizeZ)
    : m_sizeX(sizeX), m_sizeY(sizeY), m_sizeZ(sizeZ)
{
    m_blocks.resize(sizeX * sizeY * sizeZ);
    m_meshDirty = true;
}

void World::createTree(int x, int y, int z) {
    int treeType = std::rand() % 3; // 0 - маленькое, 1 - среднее, 2 - высокое
    int trunkHeight = 3 + treeType + (std::rand() % 2);
    
    // Генерируем крону в зависимости от типа
    int leafRadius = (treeType == 0) ? 1 : 2;
    
    for (int ly = -leafRadius; ly <= leafRadius + 1; ly++) {
        for (int lx = -leafRadius; lx <= leafRadius; lx++) {
            for (int lz = -leafRadius; lz <= leafRadius; lz++) {
                
                // Рандомно "откусываем" листву по углам для органичности
                if ((std::abs(lx) + std::abs(ly) + std::abs(lz)) > leafRadius + 1) continue;
                if (std::rand() % 10 == 0) continue; 

                int nx = x + lx, ny = y + trunkHeight + ly, nz = z + lz;
                if (nx >= 0 && nx < m_sizeX && ny >= 0 && ny < m_sizeY && nz >= 0 && nz < m_sizeZ) {
                    if (getBlock(nx, ny, nz).type == BlockType::AIR) {
                        setBlock(nx, ny, nz, BlockType::LEAVES);
                    }
                }
            }
        }
    }

    // Ствол
    for (int i = 0; i < trunkHeight; i++) {
        if (y + i < m_sizeY) setBlock(x, y + i, z, BlockType::WOOD);
    }
}

void World::generate() {
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Для рандома

    for (int x = 0; x < m_sizeX; x++) {
        for (int z = 0; z < m_sizeZ; z++) {
            int height = static_cast<int>(std::round(generateHeight(x, z)));
            height = std::clamp(height, 1, m_sizeY - 1);

            for (int y = 0; y < m_sizeY; y++) {
                if (y < height - 1) {
                    setBlock(x, y, z, BlockType::STONE);
                } else if (y == height - 1) {
                    setBlock(x, y, z, BlockType::GRASS);
                    
                    // --- ШАНС НА ДЕРЕВО ---
                    // Только если мы не слишком близко к краю карты
                    if (x > 2 && x < m_sizeX - 2 && z > 2 && z < m_sizeZ - 2) {
                        if (std::rand() % 100 < 2) { // 2% шанс
                            createTree(x, y + 1, z);
                        }
                    }
                } else {
                    // Чтобы дерево не затиралось воздухом, 
                    // ставим воздух только если там еще ничего нет
                    if (getBlock(x, y, z).type == BlockType::AIR) {
                         setBlock(x, y, z, BlockType::AIR);
                    }
                }
            }
        }
    }
    m_meshDirty = true;
}

void World::setBlock(int x, int y, int z, BlockType type) {
    if (x < 0 || x >= m_sizeX || y < 0 || y >= m_sizeY || z < 0 || z >= m_sizeZ)
        return;

    int idx = index(x, y, z);
    if (m_blocks[idx].type != type) {
        m_blocks[idx].type = type;
        m_meshDirty = true;
    }
}

Block World::getBlock(int x, int y, int z) const {
    if (x < 0 || x >= m_sizeX || y < 0 || y >= m_sizeY || z < 0 || z >= m_sizeZ)
        return Block();

    return m_blocks[index(x, y, z)];
}

bool World::hasNeighbor(int x, int y, int z) const {
    if (x < 0 || x >= m_sizeX || y < 0 || y >= m_sizeY || z < 0 || z >= m_sizeZ)
        return false;
    return m_blocks[index(x, y, z)].isSolid();
}

void World::addFaceData(const std::array<sf::Vector2f, 4>& faceUVs) const {
    unsigned int base = static_cast<unsigned int>(m_meshVerts.size()) - 4;
    m_meshIndices.insert(m_meshIndices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    m_meshUVs.insert(m_meshUVs.end(), {faceUVs[0], faceUVs[1], faceUVs[2], faceUVs[3]});
}

void World::buildMesh(const TextureAtlas& atlas) const {
    m_meshVerts.clear();
    m_meshIndices.clear();
    m_meshUVs.clear();

    m_meshVerts.reserve(m_sizeX * m_sizeY * m_sizeZ * 4);
    m_meshUVs.reserve(m_sizeX * m_sizeY * m_sizeZ * 4);
    m_meshIndices.reserve(m_sizeX * m_sizeY * m_sizeZ * 6);

    const float low = 0.0f;
    const float high = 1.0f;

    for (int y = 0; y < m_sizeY; y++) {
        for (int z = 0; z < m_sizeZ; z++) {
            for (int x = 0; x < m_sizeX; x++) {
                
                Block block = getBlock(x, y, z);
                if (!block.isSolid()) continue;

                float wx = (float)x;
                float wy = (float)y;
                float wz = (float)z;

                // Получаем массив ID текстур для этого блока
                auto texIDs = Block::getTextures(block.type);

                // +X
                if (!hasNeighbor(x + 1, y, z)) {
                    m_meshVerts.push_back(Vector3(wx + high, wy + low,  wz + high));
                    m_meshVerts.push_back(Vector3(wx + high, wy + low,  wz + low));
                    m_meshVerts.push_back(Vector3(wx + high, wy + high, wz + low));
                    m_meshVerts.push_back(Vector3(wx + high, wy + high, wz + high));
                    addFaceData(atlas.getUV(texIDs[FACE_PX]));
                }
                // -X
                if (!hasNeighbor(x - 1, y, z)) {
                    m_meshVerts.push_back(Vector3(wx + low, wy + low,  wz + low));
                    m_meshVerts.push_back(Vector3(wx + low, wy + low,  wz + high));
                    m_meshVerts.push_back(Vector3(wx + low, wy + high, wz + high));
                    m_meshVerts.push_back(Vector3(wx + low, wy + high, wz + low));
                    addFaceData(atlas.getUV(texIDs[FACE_NX]));
                }
                // +Y (Верх)
                if (!hasNeighbor(x, y + 1, z)) {
                    m_meshVerts.push_back(Vector3(wx + low,  wy + high, wz + low));
                    m_meshVerts.push_back(Vector3(wx + high, wy + high, wz + low));
                    m_meshVerts.push_back(Vector3(wx + high, wy + high, wz + high));
                    m_meshVerts.push_back(Vector3(wx + low,  wy + high, wz + high));
                    addFaceData(atlas.getUV(texIDs[FACE_PY]));
                }
                // -Y (Низ)
                if (!hasNeighbor(x, y - 1, z)) {
                    m_meshVerts.push_back(Vector3(wx + low,  wy + low, wz + high));
                    m_meshVerts.push_back(Vector3(wx + high, wy + low, wz + high));
                    m_meshVerts.push_back(Vector3(wx + high, wy + low, wz + low));
                    m_meshVerts.push_back(Vector3(wx + low,  wy + low, wz + low));
                    addFaceData(atlas.getUV(texIDs[FACE_NY]));
                }
                // +Z
                if (!hasNeighbor(x, y, z + 1)) {
                    m_meshVerts.push_back(Vector3(wx + high, wy + low,  wz + high));
                    m_meshVerts.push_back(Vector3(wx + low,  wy + low,  wz + high));
                    m_meshVerts.push_back(Vector3(wx + low,  wy + high, wz + high));
                    m_meshVerts.push_back(Vector3(wx + high, wy + high, wz + high));
                    addFaceData(atlas.getUV(texIDs[FACE_PZ]));
                }
                // -Z
                if (!hasNeighbor(x, y, z - 1)) {
                    m_meshVerts.push_back(Vector3(wx + low,  wy + low,  wz + low));
                    m_meshVerts.push_back(Vector3(wx + high, wy + low,  wz + low));
                    m_meshVerts.push_back(Vector3(wx + high, wy + high, wz + low));
                    m_meshVerts.push_back(Vector3(wx + low,  wy + high, wz + low));
                    addFaceData(atlas.getUV(texIDs[FACE_NZ]));
                }
            }
        }
    }
    m_meshDirty = false;
}

void World::draw(Renderer& renderer, const TextureAtlas& atlas,
                 const Matrix4x4& view, const Matrix4x4& proj,
                 const Vector3& cameraPos) const
{
    if (m_meshDirty) {
        buildMesh(atlas);
    }

    if (m_meshIndices.empty()) return;

    Matrix4x4 model; 
    renderer.drawMesh(m_meshVerts, m_meshIndices, m_meshUVs, 
                      atlas.getImage(), model, view, proj, cameraPos);
}

bool World::isBlocking(float worldX, float worldY, float worldZ) const {
    int bx = static_cast<int>(std::floor(worldX));
    int by = static_cast<int>(std::floor(worldY));
    int bz = static_cast<int>(std::floor(worldZ));
    return getBlock(bx, by, bz).isSolid();
}

