#include "World.h"
#include <cmath>
#include <ctime>

World::World() {}

static std::tuple<int, int, int> toChunkCoords(int x, int y, int z) {
    auto coord = [](int v) { return (v >= 0) ? v / Chunk::SIZE : (v - Chunk::SIZE + 1) / Chunk::SIZE; };
    return {coord(x), coord(y), coord(z)};
}

void World::setBlock(int x, int y, int z, BlockType type) {
    auto [cx, cy, cz] = toChunkCoords(x, y, z);
    auto key = std::make_tuple(cx, cy, cz);

    // Если дерево лезет в несуществующий чанк — создаем его!
    if (m_chunks.find(key) == m_chunks.end()) {
        m_chunks[key] = std::make_unique<Chunk>(cx, cy, cz);
    }

    m_chunks[key]->setBlock(x - cx * Chunk::SIZE, y - cy * Chunk::SIZE, z - cz * Chunk::SIZE, type);
}

Block World::getBlock(int x, int y, int z) const {
    auto [cx, cy, cz] = toChunkCoords(x, y, z);
    auto it = m_chunks.find({cx, cy, cz});
    if (it == m_chunks.end()) return Block(BlockType::AIR);
    return it->second->getBlock(x - cx * Chunk::SIZE, y - cy * Chunk::SIZE, z - cz * Chunk::SIZE);
}

// Вставь сюда свою функцию generateHeight из прошлого сообщения
static float getH(int x, int z) {
    float fx = (float)x, fz = (float)z;
    return (std::sin(fx * 0.05f) + std::cos(fz * 0.05f)) * 4.0f + 8.0f;
}

void World::generate(int rx, int rz) {
    for (int cx = -rx; cx <= rx; cx++) {
        for (int cz = -rz; cz <= rz; cz++) {
            for (int x = 0; x < Chunk::SIZE; x++) {
                for (int z = 0; z < Chunk::SIZE; z++) {
                    int wx = cx * Chunk::SIZE + x;
                    int wz = cz * Chunk::SIZE + z;
                    int h = (int)std::round(getH(wx, wz));
                    for (int y = 0; y < h; y++) {
                        setBlock(wx, y, wz, (y == h - 1) ? BlockType::GRASS : BlockType::STONE);
                    }
                    if (std::rand() % 100 < 1 && h > 0) createTree(wx, h, wz);
                }
            }
        }
    }
}

void World::createTree(int x, int y, int z) {
    int trunkHeight = 4 + (std::rand() % 3); // Высота ствола 4-6 блоков

    // 1. СТВОЛ
    for (int i = 0; i < trunkHeight; i++) {
        setBlock(x, y + i, z, BlockType::WOOD);
    }

    // 2. КРОНА (ЛИСТВА)
    // Делаем простую крону 3x3x3 или 5x5x3 на вершине
    int leafStart = y + trunkHeight - 2;
    for (int ly = leafStart; ly <= y + trunkHeight + 1; ly++) {
        int radius = (ly < y + trunkHeight) ? 2 : 1; // Внизу пошире, на макушке поуже
        for (int lx = x - radius; lx <= x + radius; lx++) {
            for (int lz = z - radius; lz <= z + radius; lz++) {
                
                // Не ставим листву там, где уже есть ствол
                if (lx == x && lz == z && ly < y + trunkHeight) continue;
                
                // Рандомно убираем углы для "округлости"
                if (std::abs(lx - x) == radius && std::abs(lz - z) == radius) {
                    if (std::rand() % 2 == 0) continue;
                }

                // ВАЖНО: setBlock сам найдет нужный чанк, даже если он соседний!
                if (getBlock(lx, ly, lz).type == BlockType::AIR) {
                    setBlock(lx, ly, lz, BlockType::LEAVES);
                }
            }
        }
    }
}
void World::draw(Renderer& renderer, const TextureAtlas& atlas,
                 const Matrix4x4& view, const Matrix4x4& proj,
                 const Vector3& cameraPos) const {
    
    const float RENDER_DIST = 32.0f; // Максимальная дистанция (2 чанка)

    for (auto const& [coords, chunk] : m_chunks) {
        auto [cx, cy, cz] = coords;
        float dx = (cx * Chunk::SIZE + 8) - cameraPos.x;
        float dz = (cz * Chunk::SIZE + 8) - cameraPos.z;
        
        // ОГРАНИЧЕНИЕ ПРОРИСОВКИ
        if (std::sqrt(dx*dx + dz*dz) > RENDER_DIST) continue;

        if (chunk->isDirty()) chunk->buildMesh(atlas, *this);
        chunk->draw(renderer, atlas, view, proj, cameraPos);
    }
}

bool World::isBlocking(float x, float y, float z) const {
    return getBlock((int)std::floor(x), (int)std::floor(y), (int)std::floor(z)).isSolid();
}