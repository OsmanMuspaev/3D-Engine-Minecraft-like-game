#include "Renderer.h"
#include <algorithm>
#include <limits>
#include <cmath>

// Конструктор
Renderer::Renderer(unsigned int width, unsigned int height)
    : m_width(width)
    , m_height(height)
{
    m_frameBuffer = sf::Image({width, height}, sf::Color::Black);
    m_texture.loadFromImage(m_frameBuffer);
    m_sprite.emplace(m_texture);
    
    m_zBuffer.resize(width * height, std::numeric_limits<float>::max());

    m_lightDir = Vector3(0.3f, -0.8f, -0.5f).normalize();
}

// Очистка буфера
void Renderer::clear(sf::Color color) {
    for (unsigned int y = 0; y < m_height; y++) {
        for (unsigned int x = 0; x < m_width; x++) {
            m_frameBuffer.setPixel({x, y}, color);
        }
    }
    std::fill(m_zBuffer.begin(), m_zBuffer.end(), std::numeric_limits<float>::max());
}

// Поставить пиксель с Z-тестом
void Renderer::setPixel(int x, int y, float z, sf::Color color) {
    if (x < 0 || x >= static_cast<int>(m_width) || 
        y < 0 || y >= static_cast<int>(m_height)) {
        return;
    }
    
    unsigned int index = y * m_width + x;
    
    if (z < m_zBuffer[index]) {
        m_zBuffer[index] = z;
        m_frameBuffer.setPixel({static_cast<unsigned int>(x), 
                                 static_cast<unsigned int>(y)}, color);
    }
}

// Рисование горизонтальной линии (span)
void Renderer::drawScanLine(int y, const Vector4& left, const Vector4& right, sf::Color color) {
    if (y < 0 || y >= static_cast<int>(m_height)) return;
    
    int x1 = static_cast<int>(std::ceil(left.x - 0.5f));
    int x2 = static_cast<int>(std::ceil(right.x - 0.5f));
    
    if (x1 > x2) {
        std::swap(x1, x2);
    }
    
    for (int x = x1; x < x2; x++) {
        if (x < 0 || x >= static_cast<int>(m_width)) continue;
        
        float t = (x2 - x1 > 0) ? static_cast<float>(x - x1) / static_cast<float>(x2 - x1) : 0.0f;
        float z = left.z + t * (right.z - left.z);
        
        setPixel(x, y, z, color);
    }
}

// Растеризация треугольника
void Renderer::drawTriangle(const Vector4& v0, const Vector4& v1, const Vector4& v2, sf::Color color) {
    Vector4 top = v0, mid = v1, bottom = v2;
    
    if (top.y > mid.y) std::swap(top, mid);
    if (mid.y > bottom.y) std::swap(mid, bottom);
    if (top.y > mid.y) std::swap(top, mid);
    
    float totalHeight = bottom.y - top.y;
    
    if (totalHeight <= 0.0f) return;
    
    if (mid.y > top.y) {
        float segmentHeight = mid.y - top.y;
        
        int yStart = static_cast<int>(std::ceil(top.y));
        int yEnd   = static_cast<int>(std::ceil(mid.y));
        
        for (int y = yStart; y < yEnd; y++) {
            float t = static_cast<float>(y - top.y) / totalHeight;
            float s = (segmentHeight > 0.0f) ? static_cast<float>(y - top.y) / segmentHeight : 0.0f;
            
            Vector4 left = top + (bottom - top) * t;
            Vector4 right = top + (mid - top) * s;
            
            if (left.x > right.x) std::swap(left, right);
            
            drawScanLine(y, left, right, color);
        }
    }

    if (bottom.y > mid.y) {
        float segmentHeight = bottom.y - mid.y;
        
        int yStart = static_cast<int>(std::ceil(mid.y));
        int yEnd   = static_cast<int>(std::ceil(bottom.y));
        
        for (int y = yStart; y < yEnd; y++) {
            float t = static_cast<float>(y - top.y) / totalHeight;
            float s = (segmentHeight > 0.0f) ? static_cast<float>(y - mid.y) / segmentHeight : 0.0f;
            
            Vector4 left = top + (bottom - top) * t;
            Vector4 right = mid + (bottom - mid) * s;
            
            if (left.x > right.x) std::swap(left, right);
            
            drawScanLine(y, left, right, color);
        }
    }
}

// Отрисовка меша с трансформациями
void Renderer::drawMesh(const std::vector<Vector3>& vertices,
                        const std::vector<unsigned int>& indices,
                        const Matrix4x4& model,
                        const Matrix4x4& view,
                        const Matrix4x4& proj,
                        const sf::Color color,
                        const Vector3& cameraPos) {
    
    Matrix4x4 mvp = proj * view * model;
    
    std::vector<Vector4> transformed;
    transformed.reserve(vertices.size());
    
    for (const Vector3& v : vertices) {
        Vector4 v4(v.x, v.y, v.z, 1.0f);
        
        Vector4 projected = mvp * v4;
        
        if (projected.w != 0.0f) {
            projected.x /= projected.w;
            projected.y /= projected.w;
            projected.z /= projected.w;
        }
        
        projected.x = (projected.x + 1.0f) * 0.5f * static_cast<float>(m_width);
        projected.y = (1.0f - projected.y) * 0.5f * static_cast<float>(m_height);
        
        transformed.push_back(projected);
    }
    
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        Vector4 w0 = model * Vector4(vertices[i0].x, vertices[i0].y, vertices[i0].z, 1.0f);
        Vector4 w1 = model * Vector4(vertices[i1].x, vertices[i1].y, vertices[i1].z, 1.0f);
        Vector4 w2 = model * Vector4(vertices[i2].x, vertices[i2].y, vertices[i2].z, 1.0f);

        Vector3 edge1(w1.x - w0.x, w1.y - w0.y, w1.z - w0.z);
        Vector3 edge2(w2.x - w0.x, w2.y - w0.y, w2.z - w0.z);
        Vector3 normal = edge1.cross(edge2).normalize();

        Vector3 toCamera(cameraPos.x - w0.x, cameraPos.y - w0.y, cameraPos.z - w0.z);
        if (normal.dot(toCamera) <= 0.0f) {
            continue;
        }

        float brightness = normal.dot(m_lightDir * -1.0f);
        if (brightness < 0.0f) brightness = 0.0f;
        if (brightness > 1.0f) brightness = 1.0f;

        sf::Color litColor(
            static_cast<uint8_t>(color.r * brightness),
            static_cast<uint8_t>(color.g * brightness),
            static_cast<uint8_t>(color.b * brightness)
        );
        
        drawTriangle(transformed[i0], transformed[i1], transformed[i2], litColor);
    }
}

// Отображение буфера на экране
void Renderer::display(sf::RenderWindow& window) {
    m_texture.update(m_frameBuffer);
    window.draw(*m_sprite);
}

// Задать направление света
void Renderer::setLightDirection(const Vector3& direction) {
    m_lightDir = direction.normalize();
}