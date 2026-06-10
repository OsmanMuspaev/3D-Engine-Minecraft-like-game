#include "Chunk.h"
#include "World.h"
#include "BlockRegistry.h"

// Initializes a chunk at the given chunk coordinates with all-air blocks.
Chunk::Chunk(int cx, int cy, int cz) : m_cx(cx), m_cy(cy), m_cz(cz) {
    m_blocks.resize(SIZE * SIZE * SIZE, Block(BlockType::AIR));
}

// Sets the block type at local coordinates and marks the chunk dirty.
void Chunk::setBlock(int x, int y, int z, BlockType type) {
    m_blocks[index(x, y, z)].type = type;
    m_dirty = true;
}

// Returns the block at local coordinates.
Block Chunk::getBlock(int x, int y, int z) const {
    return m_blocks[index(x, y, z)];
}

// Appends two triangles (6 indices) for a quad face.
void Chunk::addFaceData(const std::array<sf::Vector2f, 4>& faceUVs,
                         const sf::Color& tintColor) const
{
    unsigned int base = static_cast<unsigned int>(m_meshVerts.size()) - 4;
    m_meshIndices.insert(m_meshIndices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    m_meshUVs.insert(m_meshUVs.end(), {faceUVs[0], faceUVs[1], faceUVs[2], faceUVs[3]});
    for (int i = 0; i < 4; i++) {
        m_meshTints.push_back(tintColor);
    }
}

// Rebuilds the chunk mesh by iterating all blocks and emitting visible faces.
void Chunk::buildMesh(const TextureManager& texMgr, const World& world) {
    m_meshVerts.clear();
    m_meshIndices.clear();
    m_meshUVs.clear();
    m_meshTints.clear();

    const auto& blockReg = BlockRegistry::instance();

    for (int y = 0; y < SIZE; y++) {
        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                Block block = getBlock(x, y, z);
                if (block.type == BlockType::AIR) continue;

                int wx = m_cx * SIZE + x;
                int wy = m_cy * SIZE + y;
                int wz = m_cz * SIZE + z;

                auto texIDs = blockReg.getBlockTextures(texMgr, block.type);
                const auto& def = blockReg.getDef(block.type);
                float fx = (float)wx, fy = (float)wy, fz = (float)wz;

                Biome biome = world.getBiome(wx, wz);

                bool isTransparent = BlockRegistry::isTransparent(block.type);

                auto shouldRenderFace = [&](int dx, int dy, int dz) {
                    Block neighbor = world.getBlock(wx + dx, wy + dy, wz + dz);
                    if (isTransparent) return true;
                    return !neighbor.isSolid() || BlockRegistry::isTransparent(neighbor.type);
                };

                // +X face
                if (shouldRenderFace(1, 0, 0)) {
                    m_meshVerts.push_back({fx + 1, fy,     fz + 1});
                    m_meshVerts.push_back({fx + 1, fy,     fz});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz + 1});
                    addFaceData(texMgr.getUV(texIDs[FACE_PX]),
                                world.getBiomeManager().getTintColor(biome, def.tintIndex[FACE_PX]));
                }
                // -X face
                if (shouldRenderFace(-1, 0, 0)) {
                    m_meshVerts.push_back({fx, fy,     fz});
                    m_meshVerts.push_back({fx, fy,     fz + 1});
                    m_meshVerts.push_back({fx, fy + 1, fz + 1});
                    m_meshVerts.push_back({fx, fy + 1, fz});
                    addFaceData(texMgr.getUV(texIDs[FACE_NX]),
                                world.getBiomeManager().getTintColor(biome, def.tintIndex[FACE_NX]));
                }
                // +Y face (top)
                if (shouldRenderFace(0, 1, 0)) {
                    m_meshVerts.push_back({fx,     fy + 1, fz + 1});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz + 1});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz});
                    m_meshVerts.push_back({fx,     fy + 1, fz});
                    addFaceData(texMgr.getUV(texIDs[FACE_PY]),
                                world.getBiomeManager().getTintColor(biome, def.tintIndex[FACE_PY]));
                }
                // -Y face (bottom)
                if (shouldRenderFace(0, -1, 0)) {
                    m_meshVerts.push_back({fx,     fy, fz});
                    m_meshVerts.push_back({fx + 1, fy, fz});
                    m_meshVerts.push_back({fx + 1, fy, fz + 1});
                    m_meshVerts.push_back({fx,     fy, fz + 1});
                    addFaceData(texMgr.getUV(texIDs[FACE_NY]),
                                world.getBiomeManager().getTintColor(biome, def.tintIndex[FACE_NY]));
                }
                // +Z face
                if (shouldRenderFace(0, 0, 1)) {
                    m_meshVerts.push_back({fx,     fy,     fz + 1});
                    m_meshVerts.push_back({fx + 1, fy,     fz + 1});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz + 1});
                    m_meshVerts.push_back({fx,     fy + 1, fz + 1});
                    addFaceData(texMgr.getUV(texIDs[FACE_PZ]),
                                world.getBiomeManager().getTintColor(biome, def.tintIndex[FACE_PZ]));
                }
                // -Z face
                if (shouldRenderFace(0, 0, -1)) {
                    m_meshVerts.push_back({fx + 1, fy,     fz});
                    m_meshVerts.push_back({fx,     fy,     fz});
                    m_meshVerts.push_back({fx,     fy + 1, fz});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz});
                    addFaceData(texMgr.getUV(texIDs[FACE_NZ]),
                                world.getBiomeManager().getTintColor(biome, def.tintIndex[FACE_NZ]));
                }
            }
        }
    }
    m_dirty = false;
}

// Draws the chunk mesh if it has geometry.
void Chunk::draw(Renderer& renderer, const TextureManager& texMgr,
                 const Matrix4x4& view, const Matrix4x4& proj, const Vector3& cameraPos) const {
    if (m_meshIndices.empty()) return;
    Matrix4x4 model;
    renderer.drawMesh(m_meshVerts, m_meshIndices, m_meshUVs, m_meshTints, texMgr.getImage(), model, view, proj, cameraPos);
}
