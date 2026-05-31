#include "Renderer.h"
#include <cmath>
#include <algorithm>

Renderer::Renderer(unsigned int width, unsigned int height)
    : m_width(width), m_height(height)
{
    m_depthBuffer.resize(width * height, std::numeric_limits<float>::infinity());
    m_image.resize({width, height});
    m_imageTexture.resize({width, height});
    m_lightDir = Vector3(0.3f, -0.8f, -0.5f).normalize();
    m_clearColor = sf::Color::Black;
}

void Renderer::setLightDirection(const Vector3& dir) {
    m_lightDir = dir.normalize();
}

void Renderer::clear(sf::Color color)
{
    m_clearColor = color;
    if (m_image.getSize().x != m_width || m_image.getSize().y != m_height) {
        m_image.resize({m_width, m_height}, color);
    } else {
        m_image.resize({m_width, m_height}, color); 
    }
    std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), std::numeric_limits<float>::infinity());
}

static std::vector<Vector4> transformVertices(
    const std::vector<Vector3>& vertices,
    const Matrix4x4& mvp,
    float halfW, float halfH
) {
    std::vector<Vector4> result;
    result.reserve(vertices.size());

    for (const Vector3& v : vertices) {
        Vector4 projected = mvp * Vector4(v.x, v.y, v.z, 1.0f);
        float originalW = projected.w;

        if (projected.w != 0.0f) {
            float invW = 1.0f / projected.w;
            projected.x *= invW;
            projected.y *= invW;
            projected.z *= invW;
        }

        projected.x = (projected.x + 1.0f) * halfW;
        projected.y = (1.0f - projected.y) * halfH;
        projected.w = originalW; 

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
    auto transformed = transformVertices(vertices, mvp, m_width * 0.5f, m_height * 0.5f);
    drawMeshInternal(transformed, vertices, indices, nullptr, nullptr, model, color, cameraPos);
}

void Renderer::drawMesh(const std::vector<Vector3>& vertices,
                        const std::vector<unsigned int>& indices,
                        const std::vector<sf::Vector2f>& uvs,
                        const sf::Image& image,
                        const Matrix4x4& model,
                        const Matrix4x4& view,
                        const Matrix4x4& proj,
                        const Vector3& cameraPos) {
    Matrix4x4 mvp = proj * view * model;
    auto transformed = transformVertices(vertices, mvp, m_width * 0.5f, m_height * 0.5f);
    drawMeshInternal(transformed, vertices, indices, &uvs, &image, model, sf::Color::White, cameraPos);
}

void Renderer::rasterizeTriangle(
    const Vector4& v0, const Vector4& v1, const Vector4& v2,
    sf::Color c0, sf::Color c1, sf::Color c2,
    sf::Vector2f uv0, sf::Vector2f uv1, sf::Vector2f uv2,
    const uint8_t* texPixels, sf::Vector2u texSize)
{
    int minX = std::max(0,              (int)std::floor(std::min({v0.x, v1.x, v2.x})));
    int maxX = std::min((int)m_width-1, (int)std::ceil (std::max({v0.x, v1.x, v2.x})));
    int minY = std::max(0,              (int)std::floor(std::min({v0.y, v1.y, v2.y})));
    int maxY = std::min((int)m_height-1,(int)std::ceil (std::max({v0.y, v1.y, v2.y})));

    float denom = (v1.y - v2.y)*(v0.x - v2.x) + (v2.x - v1.x)*(v0.y - v2.y);
    if (std::abs(denom) < 1e-6f) return;
    float invDenom = 1.0f / denom;

    float invW0 = 1.0f / v0.w;
    float invW1 = 1.0f / v1.w;
    float invW2 = 1.0f / v2.w;

    for (int py = minY; py <= maxY; py++) {
        for (int px = minX; px <= maxX; px++) {
            float fx = px + 0.5f, fy = py + 0.5f;

            float w0 = ((v1.y-v2.y)*(fx-v2.x) + (v2.x-v1.x)*(fy-v2.y)) * invDenom;
            float w1 = ((v2.y-v0.y)*(fx-v2.x) + (v0.x-v2.x)*(fy-v2.y)) * invDenom;
            float w2 = 1.0f - w0 - w1;

            if (w0 < 0 || w1 < 0 || w2 < 0) continue;

            float z = w0*v0.z + w1*v1.z + w2*v2.z;
            int idx = py * m_width + px;

            if (texPixels) {
                float interpInvW = w0*invW0 + w1*invW1 + w2*invW2;
                float u = (w0*uv0.x*invW0 + w1*uv1.x*invW1 + w2*uv2.x*invW2) / interpInvW;
                float v = (w0*uv0.y*invW0 + w1*uv1.y*invW1 + w2*uv2.y*invW2) / interpInvW;
                
                int tx = std::clamp((int)u, 0, (int)texSize.x - 1);
                int ty = std::clamp((int)v, 0, (int)texSize.y - 1);
                int ti = (ty * (int)texSize.x + tx) * 4;

                // --- ФИКС МЕРЦАНИЯ (ALPHA TEST) ---
                // Если пиксель в атласе прозрачный (альфа < 128), мы его игнорируем полностью
                if (texPixels[ti + 3] < 128) continue;

                // Только после проверки прозрачности проверяем Z-буфер
                if (z >= m_depthBuffer[idx]) continue;
                m_depthBuffer[idx] = z;

                m_image.setPixel({(unsigned int)px, (unsigned int)py}, sf::Color(
                    (uint8_t)(texPixels[ti+0] * c0.r / 255.0f),
                    (uint8_t)(texPixels[ti+1] * c0.g / 255.0f),
                    (uint8_t)(texPixels[ti+2] * c0.b / 255.0f),
                    texPixels[ti+3]
                ));
            } else {
                // Обычная заливка цветом (без текстур)
                if (z >= m_depthBuffer[idx]) continue;
                m_depthBuffer[idx] = z;
                
                m_image.setPixel({(unsigned int)px, (unsigned int)py}, sf::Color(
                    (uint8_t)(w0*c0.r + w1*c1.r + w2*c2.r),
                    (uint8_t)(w0*c0.g + w1*c1.g + w2*c2.g),
                    (uint8_t)(w0*c0.b + w1*c1.b + w2*c2.b)
                ));
            }
        }
    }
}

void Renderer::drawMeshInternal(
    const std::vector<Vector4>& transformed,
    const std::vector<Vector3>& vertices,
    const std::vector<unsigned int>& indices,
    const std::vector<sf::Vector2f>* uvs,
    const sf::Image* image,
    const Matrix4x4& model,
    sf::Color baseColor,
    const Vector3& cameraPos)
{
    const uint8_t* texPixels = nullptr;
    sf::Vector2u texSize;
    if (image) {
        texPixels = image->getPixelsPtr();
        texSize   = image->getSize();
    }

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        unsigned int i0 = indices[i], i1 = indices[i+1], i2 = indices[i+2];

        if (transformed[i0].w <= 0.01f || transformed[i1].w <= 0.01f || transformed[i2].w <= 0.01f)
            continue;

        Vector4 w0 = model * Vector4(vertices[i0].x, vertices[i0].y, vertices[i0].z, 1.0f);
        Vector4 w1 = model * Vector4(vertices[i1].x, vertices[i1].y, vertices[i1].z, 1.0f);
        Vector4 w2 = model * Vector4(vertices[i2].x, vertices[i2].y, vertices[i2].z, 1.0f);
        
        Vector3 edge1(w1.x-w0.x, w1.y-w0.y, w1.z-w0.z);
        Vector3 edge2(w2.x-w0.x, w2.y-w0.y, w2.z-w0.z);
        Vector3 normal = edge1.cross(edge2).normalize();

        constexpr float AMBIENT = 0.2f;
        float dotLight = normal.dot(m_lightDir * -1.0f);
        float brightness = std::clamp(dotLight, 0.0f, 1.0f);
        brightness = AMBIENT + (1.0f - AMBIENT) * brightness;

        sf::Color lit(
            (uint8_t)(baseColor.r * brightness),
            (uint8_t)(baseColor.g * brightness),
            (uint8_t)(baseColor.b * brightness)
        );

        sf::Vector2f uv0, uv1, uv2;
        if (uvs) { uv0=(*uvs)[i0]; uv1=(*uvs)[i1]; uv2=(*uvs)[i2]; }

        if (transformed[i0].z < 0.0f || transformed[i1].z < 0.0f || transformed[i2].z < 0.0f)
            continue;

        rasterizeTriangle(
            transformed[i0], transformed[i1], transformed[i2],
            lit, lit, lit,
            uv0, uv1, uv2,
            texPixels, texSize
        );
    }
}

void Renderer::display(sf::RenderWindow& window) {
    m_imageTexture.update(m_image);
    sf::Sprite sprite(m_imageTexture);
    sf::Vector2u windowSize = window.getSize();
    
    float scaleX = (float)windowSize.x / m_width;
    float scaleY = (float)windowSize.y / m_height;
    
    sprite.setScale({scaleX, scaleY});
    window.draw(sprite);
}