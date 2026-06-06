#include "World.h"
#include "BlockRegistry.h"
#include <cmath>
#include <set>

World::World() {}

static std::tuple<int, int, int> toChunkCoords(int x, int y, int z) {
    auto coord = [](int v) { return (v >= 0) ? v / Chunk::SIZE : (v - Chunk::SIZE + 1) / Chunk::SIZE; };
    return {coord(x), coord(y), coord(z)};
}

void World::setBlock(int x, int y, int z, BlockType type) {
    auto [cx, cy, cz] = toChunkCoords(x, y, z);
    auto key = std::make_tuple(cx, cy, cz);

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

void World::generate(int rx, int rz) {
    std::vector<std::pair<int, int>> treeCandidates;

    // Генерация блоков
    for (int cx = -rx; cx <= rx; cx++) {
        for (int cz = -rz; cz <= rz; cz++) {
            for (int x = 0; x < Chunk::SIZE; x++) {
                for (int z = 0; z < Chunk::SIZE; z++) {
                    int wx = cx * Chunk::SIZE + x;
                    int wz = cz * Chunk::SIZE + z;
                    int h = (int)std::round(m_biomeMgr.getBaseHeight(wx, wz));

                    Biome biome = m_biomeMgr.getBiome(wx, wz);
                    const auto& def = m_biomeMgr.getDef(biome);

                    for (int y = 0; y < h; y++) {
                        BlockType type;
                        if (y == h - 1)      type = def.surface;
                        else if (y >= h - 3) type = def.subsurface;
                        else                  type = def.stone;
                        setBlock(wx, y, wz, type);
                    }

                    if (h > 0 && def.trees) {
                        float roll = (float)(std::rand() % 10000) / 10000.0f;
                        if (roll < def.treeDensity) {
                            treeCandidates.push_back({wx, wz});
                        }
                    }
                }
            }
        }
    }

    // Размещение деревьев с минимальным расстоянием 6 блоков
    constexpr float TREE_SPACING = 6.0f;
    std::set<std::pair<int, int>> placedTrees;

    for (auto [tx, tz] : treeCandidates) {
        bool tooClose = false;
        for (auto& [px, pz] : placedTrees) {
            float dx = (float)(tx - px);
            float dz = (float)(tz - pz);
            if (dx * dx + dz * dz < TREE_SPACING * TREE_SPACING) {
                tooClose = true;
                break;
            }
        }
        if (tooClose) continue;

        int h = (int)std::round(m_biomeMgr.getBaseHeight(tx, tz));
        if (h > 0) {
            createTree(tx, h, tz);
            placedTrees.insert({tx, tz});
        }
    }
}

void World::createTree(int x, int y, int z) {
    int trunkHeight = 4 + (std::rand() % 3);

    for (int i = 0; i < trunkHeight; i++) {
        setBlock(x, y + i, z, BlockType::WOOD);
    }

    int leafStart = y + trunkHeight - 2;
    for (int ly = leafStart; ly <= y + trunkHeight + 1; ly++) {
        int radius = (ly < y + trunkHeight) ? 2 : 1;
        for (int lx = x - radius; lx <= x + radius; lx++) {
            for (int lz = z - radius; lz <= z + radius; lz++) {
                if (lx == x && lz == z && ly < y + trunkHeight) continue;
                if (std::abs(lx - x) == radius && std::abs(lz - z) == radius) {
                    if (std::rand() % 2 == 0) continue;
                }
                if (getBlock(lx, ly, lz).type == BlockType::AIR) {
                    setBlock(lx, ly, lz, BlockType::LEAVES);
                }
            }
        }
    }
}

void World::draw(Renderer& renderer, const TextureManager& texMgr,
                 const Matrix4x4& view, const Matrix4x4& proj,
                 const Vector3& cameraPos) const {
    const float RENDER_DIST = 32.0f;

    for (auto const& [coords, chunk] : m_chunks) {
        auto [cx, cy, cz] = coords;
        float dx = (cx * Chunk::SIZE + 8) - cameraPos.x;
        float dz = (cz * Chunk::SIZE + 8) - cameraPos.z;

        if (std::sqrt(dx*dx + dz*dz) > RENDER_DIST) continue;

        if (chunk->isDirty()) chunk->buildMesh(texMgr, *this);
        chunk->draw(renderer, texMgr, view, proj, cameraPos);
    }
}

bool World::isBlocking(float x, float y, float z) const {
    return getBlock((int)std::floor(x), (int)std::floor(y), (int)std::floor(z)).isSolid();
}

Biome World::getBiome(int x, int z) const {
    return m_biomeMgr.getBiome(x, z);
}
