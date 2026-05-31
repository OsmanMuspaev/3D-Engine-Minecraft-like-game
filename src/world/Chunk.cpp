#include "Chunk.h"
#include "World.h"

Chunk::Chunk(int cx, int cy, int cz) : m_cx(cx), m_cy(cy), m_cz(cz) {
    m_blocks.resize(SIZE * SIZE * SIZE, Block(BlockType::AIR));
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    m_blocks[index(x, y, z)].type = type;
    m_dirty = true;
}

Block Chunk::getBlock(int x, int y, int z) const {
    return m_blocks[index(x, y, z)];
}

void Chunk::addFaceData(const std::array<sf::Vector2f, 4>& faceUVs) const {
    unsigned int base = static_cast<unsigned int>(m_meshVerts.size()) - 4;
    // Порядок индексов для двух треугольников: (0,1,2) и (0,2,3)
    m_meshIndices.insert(m_meshIndices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
    m_meshUVs.insert(m_meshUVs.end(), {faceUVs[0], faceUVs[1], faceUVs[2], faceUVs[3]});
}

void Chunk::buildMesh(const TextureAtlas& atlas, const World& world) {
    m_meshVerts.clear();
    m_meshIndices.clear();
    m_meshUVs.clear();

    for (int y = 0; y < SIZE; y++) {
        for (int z = 0; z < SIZE; z++) {
            for (int x = 0; x < SIZE; x++) {
                Block block = getBlock(x, y, z);
                if (!block.isSolid()) continue;

                int wx = m_cx * SIZE + x;
                int wy = m_cy * SIZE + y;
                int wz = m_cz * SIZE + z;

                auto texIDs = Block::getTextures(block.type);
                float fx = (float)wx, fy = (float)wy, fz = (float)wz;

                auto check = [&](int dx, int dy, int dz) {
                    return !world.getBlock(wx + dx, wy + dy, wz + dz).isSolid();
                };

                // +X
                if (check(1, 0, 0)) {
                    m_meshVerts.push_back({fx + 1, fy,     fz + 1});
                    m_meshVerts.push_back({fx + 1, fy,     fz});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz + 1});
                    addFaceData(atlas.getUV(texIDs[FACE_PX]));
                }
                // -X
                if (check(-1, 0, 0)) {
                    m_meshVerts.push_back({fx, fy,     fz});
                    m_meshVerts.push_back({fx, fy,     fz + 1});
                    m_meshVerts.push_back({fx, fy + 1, fz + 1});
                    m_meshVerts.push_back({fx, fy + 1, fz});
                    addFaceData(atlas.getUV(texIDs[FACE_NX]));
                }
                // +Y (Верх)
                if (check(0, 1, 0)) {
                    m_meshVerts.push_back({fx,     fy + 1, fz + 1});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz + 1});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz});
                    m_meshVerts.push_back({fx,     fy + 1, fz});
                    addFaceData(atlas.getUV(texIDs[FACE_PY]));
                }
                // -Y (Низ)
                if (check(0, -1, 0)) {
                    m_meshVerts.push_back({fx,     fy, fz});
                    m_meshVerts.push_back({fx + 1, fy, fz});
                    m_meshVerts.push_back({fx + 1, fy, fz + 1});
                    m_meshVerts.push_back({fx,     fy, fz + 1});
                    addFaceData(atlas.getUV(texIDs[FACE_NY]));
                }
                // +Z
                if (check(0, 0, 1)) {
                    m_meshVerts.push_back({fx,     fy,     fz + 1});
                    m_meshVerts.push_back({fx + 1, fy,     fz + 1});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz + 1});
                    m_meshVerts.push_back({fx,     fy + 1, fz + 1});
                    addFaceData(atlas.getUV(texIDs[FACE_PZ]));
                }
                // -Z
                if (check(0, 0, -1)) {
                    m_meshVerts.push_back({fx + 1, fy,     fz});
                    m_meshVerts.push_back({fx,     fy,     fz});
                    m_meshVerts.push_back({fx,     fy + 1, fz});
                    m_meshVerts.push_back({fx + 1, fy + 1, fz});
                    addFaceData(atlas.getUV(texIDs[FACE_NZ]));
                }
            }
        }
    }
    m_dirty = false;
}

void Chunk::draw(Renderer& renderer, const TextureAtlas& atlas, 
                 const Matrix4x4& view, const Matrix4x4& proj, const Vector3& cameraPos) const {
    if (m_meshIndices.empty()) return;
    Matrix4x4 model; 
    renderer.drawMesh(m_meshVerts, m_meshIndices, m_meshUVs, atlas.getImage(), model, view, proj, cameraPos); 
}