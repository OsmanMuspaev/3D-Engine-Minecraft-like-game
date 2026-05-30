#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"
#include "TextureAtlas.h"

class Renderer;

class Cube {
public:
    Cube();
    
    // Задать тайл для конкретной грани (0-5: front, back, right, left, top, bottom)
    void setFaceTile(int face, unsigned int tileIndex);
    
    // Задать все грани одним тайлом
    void setAllFaces(unsigned int tileIndex);
    
    // Отрисовка
    void draw(Renderer& renderer,
              const TextureAtlas& atlas,
              const Matrix4x4& model,
              const Matrix4x4& view,
              const Matrix4x4& proj,
              const Vector3& cameraPos) const;
    
private:
    static constexpr float s = 0.5f;
    
    std::vector<Vector3> m_vertices;
    std::vector<unsigned int> m_indices;
    mutable std::vector<sf::Vector2f> m_uvs;
    unsigned int m_faceTiles[6];  // тайлы для каждой грани
    
    void updateUVs(const TextureAtlas& atlas) const;
};