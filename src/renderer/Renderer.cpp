#include "Renderer.h"
#include <cmath>

Renderer::Renderer(unsigned int width, unsigned int height) {
    sf::ContextSettings settings;
    settings.depthBits = 24;
    m_renderTexture = sf::RenderTexture({width, height}, settings);
    m_sprite.emplace(m_renderTexture.getTexture());
    m_lightDir = Vector3(0.3f, -0.8f, -0.5f).normalize();
    m_clearColor = sf::Color::Black;
}

void Renderer::setLightDirection(const Vector3& dir) {
    m_lightDir = dir.normalize();
}

void Renderer::clear(sf::Color color) {
    m_clearColor = color;
    m_renderTexture.clear(m_clearColor);
}

static std::vector<Vector4> transformVertices(
    const std::vector<Vector3>& vertices,
    const Matrix4x4& mvp,
    float halfW, float halfH
) {
    std::vector<Vector4> result;
    result.reserve(vertices.size());
    
    for (const Vector3& v : vertices) {
        Vector4 v4(v.x, v.y, v.z, 1.0f);
        Vector4 projected = mvp * v4;
        
        if (projected.w != 0.0f) {
            projected.x /= projected.w;
            projected.y /= projected.w;
            projected.z /= projected.w;
        }
        
        projected.x = (projected.x + 1.0f) * halfW;
        projected.y = (1.0f - projected.y) * halfH;
        
        result.push_back(projected);
    }
    return result;
}

void Renderer::drawMesh(const std::vector<Vector3>& vertices,
                        const std::vector<unsigned int>& indices,
                        const Matrix4x4& model,
                        const Matrix4x4& view,
                        const Matrix4x4& proj,
                        sf::Color color,
                        const Vector3& cameraPos) {
    
    Matrix4x4 mvp = proj * view * model;
    auto size = m_renderTexture.getSize();
    auto transformed = transformVertices(vertices, mvp, size.x * 0.5f, size.y * 0.5f);
    drawMeshInternal(transformed, vertices, indices, nullptr, nullptr, model, color, cameraPos);
}

void Renderer::drawMesh(const std::vector<Vector3>& vertices,
                        const std::vector<unsigned int>& indices,
                        const std::vector<sf::Vector2f>& uvs,
                        const sf::Texture& texture,
                        const Matrix4x4& model,
                        const Matrix4x4& view,
                        const Matrix4x4& proj,
                        const Vector3& cameraPos) {
    
    Matrix4x4 mvp = proj * view * model;
    auto size = m_renderTexture.getSize();
    auto transformed = transformVertices(vertices, mvp, size.x * 0.5f, size.y * 0.5f);
    drawMeshInternal(transformed, vertices, indices, &uvs, &texture, model, sf::Color::White, cameraPos);
}

void Renderer::drawMeshInternal(
    const std::vector<Vector4>& transformed,
    const std::vector<Vector3>& vertices,
    const std::vector<unsigned int>& indices,
    const std::vector<sf::Vector2f>* uvs,
    const sf::Texture* texture,
    const Matrix4x4& model,
    sf::Color baseColor,
    const Vector3& cameraPos) {
    
    sf::VertexArray triangles(sf::PrimitiveType::Triangles);

    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];
        
        // Backface culling
        Vector4 w0 = model * Vector4(vertices[i0].x, vertices[i0].y, vertices[i0].z, 1.0f);
        Vector4 w1 = model * Vector4(vertices[i1].x, vertices[i1].y, vertices[i1].z, 1.0f);
        Vector4 w2 = model * Vector4(vertices[i2].x, vertices[i2].y, vertices[i2].z, 1.0f);
        
        Vector3 edge1(w1.x - w0.x, w1.y - w0.y, w1.z - w0.z);
        Vector3 edge2(w2.x - w0.x, w2.y - w0.y, w2.z - w0.z);
        Vector3 normal = edge1.cross(edge2).normalize();
        
        Vector3 toCamera(cameraPos.x - w0.x, cameraPos.y - w0.y, cameraPos.z - w0.z);
        if (normal.dot(toCamera) <= 0.0f) continue;
        
        // Освещение
        float brightness = normal.dot(m_lightDir * -1.0f);
        if (brightness < 0.0f) brightness = 0.0f;
        if (brightness > 1.0f) brightness = 1.0f;
        
        sf::Color litColor(
            static_cast<uint8_t>(baseColor.r * brightness),
            static_cast<uint8_t>(baseColor.g * brightness),
            static_cast<uint8_t>(baseColor.b * brightness)
        );

        if (uvs) {
            triangles.append({{transformed[i0].x, transformed[i0].y}, litColor, (*uvs)[i0]});
            triangles.append({{transformed[i1].x, transformed[i1].y}, litColor, (*uvs)[i1]});
            triangles.append({{transformed[i2].x, transformed[i2].y}, litColor, (*uvs)[i2]});
        } else {
            triangles.append({{transformed[i0].x, transformed[i0].y}, litColor});
            triangles.append({{transformed[i1].x, transformed[i1].y}, litColor});
            triangles.append({{transformed[i2].x, transformed[i2].y}, litColor});
        }
    }
    sf::RenderStates states;
    states.texture = texture;
    m_renderTexture.draw(triangles, states);
}

void Renderer::display(sf::RenderWindow& window) {
    m_renderTexture.display();
    window.draw(*m_sprite);
}