#include "Cube.h"
#include "../renderer/Renderer.h"

Cube::Cube() {
    // 24 вершины (по 4 на грань)
    m_vertices = {
        // 0-3: передняя (z+)
        Vector3(-s, -s, s), Vector3( s, -s, s), Vector3( s,  s, s), Vector3(-s,  s, s),
        // 4-7: задняя (z-)
        Vector3( s, -s,-s), Vector3(-s, -s,-s), Vector3(-s,  s,-s), Vector3( s,  s,-s),
        // 8-11: правая (x+)
        Vector3( s, -s, s), Vector3( s, -s,-s), Vector3( s,  s,-s), Vector3( s,  s, s),
        // 12-15: левая (x-)
        Vector3(-s, -s,-s), Vector3(-s, -s, s), Vector3(-s,  s, s), Vector3(-s,  s,-s),
        // 16-19: верхняя (y+)
        Vector3(-s,  s, s), Vector3( s,  s, s), Vector3( s,  s,-s), Vector3(-s,  s,-s),
        // 20-23: нижняя (y-)
        Vector3(-s, -s,-s), Vector3( s, -s,-s), Vector3( s, -s, s), Vector3(-s, -s, s),
    };
    
    m_indices = {
         0, 1, 2,  0, 2, 3,
         4, 5, 6,  4, 6, 7,
         8, 9,10,  8,10,11,
        12,13,14, 12,14,15,
        16,17,18, 16,18,19,
        20,21,22, 20,22,23,
    };
    
    m_uvs.resize(24);
    
    for (int i = 0; i < 6; i++) m_faceTiles[i] = 0;
}

void Cube::setFaceTile(int face, unsigned int tileIndex) {
    if (face >= 0 && face < 6) m_faceTiles[face] = tileIndex;
}

void Cube::setAllFaces(unsigned int tileIndex) {
    for (int i = 0; i < 6; i++) m_faceTiles[i] = tileIndex;
}

void Cube::updateUVs(const TextureAtlas& atlas) const {
    for (int face = 0; face < 6; face++) {
        auto faceUVs = atlas.getUV(m_faceTiles[face]);
        int base = face * 4;
        m_uvs[base + 0] = faceUVs[0];
        m_uvs[base + 1] = faceUVs[1];
        m_uvs[base + 2] = faceUVs[2];
        m_uvs[base + 3] = faceUVs[3];
    }
}

void Cube::draw(Renderer& renderer,
                const TextureAtlas& atlas,
                const Matrix4x4& model,
                const Matrix4x4& view,
                const Matrix4x4& proj,
                const Vector3& cameraPos) const {
    updateUVs(atlas);
    
    renderer.drawMesh(m_vertices, m_indices, m_uvs, atlas.getTexture(), model, view, proj, cameraPos);
}
