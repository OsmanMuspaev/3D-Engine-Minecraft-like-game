#include "Cube.h"
#include "../renderer/Renderer.h"

Cube::Cube() {
    // 24 вершины (по 4 на каждую из 6 граней для корректного наложения текстур)
    m_vertices = {
        // 0-3: передняя (z+)
        Vector3(-s, -s,  s), Vector3( s, -s,  s), Vector3( s,  s,  s), Vector3(-s,  s,  s),
        // 4-7: задняя (z-)
        Vector3( s, -s, -s), Vector3(-s, -s, -s), Vector3(-s,  s, -s), Vector3( s,  s, -s),
        // 8-11: правая (x+)
        Vector3( s, -s,  s), Vector3( s, -s, -s), Vector3( s,  s, -s), Vector3( s,  s,  s),
        // 12-15: левая (x-)
        Vector3(-s, -s, -s), Vector3(-s, -s,  s), Vector3(-s,  s,  s), Vector3(-s,  s, -s),
        // 16-19: верхняя (y+)
        Vector3(-s,  s,  s), Vector3( s,  s,  s), Vector3( s,  s, -s), Vector3(-s,  s, -s),
        // 20-23: нижняя (y-)
        Vector3(-s, -s, -s), Vector3( s, -s, -s), Vector3( s, -s,  s), Vector3(-s, -s,  s),
    };
    
    m_indices = {
         0, 1, 2,  0, 2, 3,     // front
         4, 5, 6,  4, 6, 7,     // back
         8, 9,10,  8,10,11,     // right
        12,13,14, 12,14,15,     // left
        16,17,18, 16,18,19,     // top
        20,21,22, 20,22,23      // bottom
    };
    
    m_uvs.resize(24);
    for (int i = 0; i < 6; i++) {
        m_faceTiles[i] = 0;
    }
    m_needsUVUpdate = true;
}

void Cube::setFaceTile(int face, unsigned int tileIndex) {
    if (face >= 0 && face < 6) {
        if (m_faceTiles[face] != tileIndex) {
            m_faceTiles[face] = tileIndex;
            m_needsUVUpdate = true; // Данные устарели
        }
    }
}

void Cube::setAllFaces(unsigned int tileIndex) {
    bool changed = false;
    for (int i = 0; i < 6; i++) {
        if (m_faceTiles[i] != tileIndex) {
            m_faceTiles[i] = tileIndex;
            changed = true;
        }
    }
    if (changed) m_needsUVUpdate = true;
}

void Cube::updateUVs(const TextureAtlas& atlas) const {
    // Если ничего не менялось, выходим немедленно
    if (!m_needsUVUpdate) return;

    for (int face = 0; face < 6; face++) {
        // Получаем 4 координаты угла из атласа для текущего тайла
        auto faceUVs = atlas.getUV(m_faceTiles[face]);
        int base = face * 4;
        
        m_uvs[base + 0] = faceUVs[0];
        m_uvs[base + 1] = faceUVs[1];
        m_uvs[base + 2] = faceUVs[2];
        m_uvs[base + 3] = faceUVs[3];
    }
    
    m_needsUVUpdate = false; // Помечаем, что UV теперь актуальны
}

void Cube::draw(Renderer& renderer,
                const TextureAtlas& atlas,
                const Matrix4x4& model,
                const Matrix4x4& view,
                const Matrix4x4& proj,
                const Vector3& cameraPos) const {
    
    // Обновляем UV только если это необходимо
    updateUVs(atlas);
    
    // Передаем всё рендереру
    renderer.drawMesh(
        m_vertices, 
        m_indices, 
        m_uvs, 
        atlas.getImage(), 
        model, 
        view, 
        proj, 
        cameraPos
    );
}