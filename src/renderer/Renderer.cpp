#include "Renderer.h"
#include <cmath>
#include <algorithm>
#include <limits>
#include <future>
#include <vector>

Renderer::Renderer(unsigned int width, unsigned int height)
    : m_width(width), m_height(height)
{
    m_depthBuffer.resize(width * height, std::numeric_limits<float>::infinity());
    m_image.resize({width, height}, sf::Color::Black);
    (void)m_imageTexture.resize({width, height});
    m_lightDir = Vector3(0.3f, -0.8f, -0.5f).normalize();
    m_clearColor = sf::Color::Black;
}

void Renderer::setLightDirection(const Vector3& dir) {
    m_lightDir = dir.normalize();
}

void Renderer::clear(sf::Color color)
{
    m_clearColor = color;
    m_image.resize({m_width, m_height}, color);
    std::fill(m_depthBuffer.begin(), m_depthBuffer.end(), std::numeric_limits<float>::infinity());
}

void Renderer::clearRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, sf::Color color)
{
    for (unsigned int py = y; py < y + h && py < m_height; ++py) {
        for (unsigned int px = x; px < x + w && px < m_width; ++px) {
            m_image.setPixel({px, py}, color);
            m_depthBuffer[py * m_width + px] = std::numeric_limits<float>::infinity();
        }
    }
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

void Renderer::rasterizeStripe(
    int yStart, int yEnd,
    const std::vector<RenderTriangle>& triangles,
    const uint8_t* texPixels, sf::Vector2u texSize)
{
    for (const auto& tri : triangles) {
        int minX = std::max(0, (int)std::floor(std::min({tri.v0.x, tri.v1.x, tri.v2.x})));
        int maxX = std::min((int)m_width - 1, (int)std::ceil(std::max({tri.v0.x, tri.v1.x, tri.v2.x})));
        int minY = std::max(yStart, (int)std::floor(std::min({tri.v0.y, tri.v1.y, tri.v2.y})));
        int maxY = std::min(yEnd - 1, (int)std::ceil(std::max({tri.v0.y, tri.v1.y, tri.v2.y})));

        if (minX > maxX || minY > maxY) continue;

        float invDenom = tri.invDenom;

        for (int py = minY; py <= maxY; py++) {
            for (int px = minX; px <= maxX; px++) {
                float fx = px + 0.5f;
                float fy = py + 0.5f;

                float w0 = ((tri.v1.y - tri.v2.y) * (fx - tri.v2.x) + (tri.v2.x - tri.v1.x) * (fy - tri.v2.y)) * invDenom;
                float w1 = ((tri.v2.y - tri.v0.y) * (fx - tri.v2.x) + (tri.v0.x - tri.v2.x) * (fy - tri.v2.y)) * invDenom;
                float w2 = 1.0f - w0 - w1;

                if (w0 < 0 || w1 < 0 || w2 < 0) continue;

                float z = w0 * tri.v0.z + w1 * tri.v1.z + w2 * tri.v2.z;
                int idx = py * m_width + px;

                if (z >= m_depthBuffer[idx]) continue;

                if (texPixels) {
                    float interpInvW = w0 * tri.invW0 + w1 * tri.invW1 + w2 * tri.invW2;
                    float u = (w0 * tri.uv0.x * tri.invW0 + w1 * tri.uv1.x * tri.invW1 + w2 * tri.uv2.x * tri.invW2) / interpInvW;
                    float v = (w0 * tri.uv0.y * tri.invW0 + w1 * tri.uv1.y * tri.invW1 + w2 * tri.uv2.y * tri.invW2) / interpInvW;

                    int tx = (int)u % texSize.x;
                    int ty = (int)v % texSize.y;
                    int ti = (ty * (int)texSize.x + tx) * 4;

                    if (texPixels[ti + 3] < 128) continue;

                    m_depthBuffer[idx] = z;
                    m_image.setPixel({(unsigned int)px, (unsigned int)py}, sf::Color(
                        (uint8_t)(texPixels[ti+0] * tri.color.r / 255.0f * tri.tint.r / 255.0f),
                        (uint8_t)(texPixels[ti+1] * tri.color.g / 255.0f * tri.tint.g / 255.0f),
                        (uint8_t)(texPixels[ti+2] * tri.color.b / 255.0f * tri.tint.b / 255.0f),
                        texPixels[ti+3]
                    ));
                } else {
                    m_depthBuffer[idx] = z;
                    m_image.setPixel({(unsigned int)px, (unsigned int)py}, tri.color);
                }
            }
        }
    }
}

void Renderer::drawMeshInternal(
    const std::vector<Vector4>& transformed,
    const std::vector<Vector3>& vertices,
    const std::vector<unsigned int>& indices,
    const std::vector<sf::Vector2f>* uvs,
    const std::vector<sf::Color>* tints,
    const sf::Image* image,
    const Matrix4x4& model,
    sf::Color baseColor,
    const Vector3& cameraPos,
    bool disableCulling)
{
    std::vector<RenderTriangle> triangles;
    triangles.reserve(indices.size() / 3);

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        unsigned int i0 = indices[i], i1 = indices[i+1], i2 = indices[i+2];

        if (transformed[i0].w <= 0.01f || transformed[i1].w <= 0.01f || transformed[i2].w <= 0.01f)
            continue;

        Vector4 w0 = model * Vector4(vertices[i0].x, vertices[i0].y, vertices[i0].z, 1.0f);
        Vector4 w1 = model * Vector4(vertices[i1].x, vertices[i1].y, vertices[i1].z, 1.0f);
        Vector4 w2 = model * Vector4(vertices[i2].x, vertices[i2].y, vertices[i2].z, 1.0f);

        Vector3 worldV0(w0.x, w0.y, w0.z);
        Vector3 edge1(w1.x - w0.x, w1.y - w0.y, w1.z - w0.z);
        Vector3 edge2(w2.x - w0.x, w2.y - w0.y, w2.z - w0.z);
        Vector3 normal = edge1.cross(edge2).normalize();

        Vector3 viewDir = (worldV0 - cameraPos).normalize();
        if (!disableCulling) {
            if (normal.dot(viewDir) > 0.1f) continue;
        }

        float dotLight = normal.dot(m_lightDir * -1.0f);
        float brightness = std::max(0.25f, std::min(1.0f, 0.25f + 0.75f * dotLight));

        RenderTriangle tri;
        tri.v0 = transformed[i0]; tri.v1 = transformed[i1]; tri.v2 = transformed[i2];
        tri.color = sf::Color(
            (uint8_t)(baseColor.r * brightness),
            (uint8_t)(baseColor.g * brightness),
            (uint8_t)(baseColor.b * brightness)
        );
        tri.tint = (tints && tints->size() > i0) ? (*tints)[i0] : sf::Color::White;
        if (uvs) { tri.uv0 = (*uvs)[i0]; tri.uv1 = (*uvs)[i1]; tri.uv2 = (*uvs)[i2]; }

        float denom = (tri.v1.y - tri.v2.y)*(tri.v0.x - tri.v2.x) + (tri.v2.x - tri.v1.x)*(tri.v0.y - tri.v2.y);
        if (std::abs(denom) < 1e-6f) continue;
        tri.invDenom = 1.0f / denom;
        tri.invW0 = 1.0f / tri.v0.w; tri.invW1 = 1.0f / tri.v1.w; tri.invW2 = 1.0f / tri.v2.w;

        triangles.push_back(tri);
    }

    if (triangles.empty()) return;

    const uint8_t* texPixels = (image) ? image->getPixelsPtr() : nullptr;
    sf::Vector2u texSize = (image) ? image->getSize() : sf::Vector2u(0,0);

    unsigned int numThreads = std::thread::hardware_concurrency();
    std::vector<std::future<void>> futures;
    int stripeHeight = m_height / numThreads;

    for (unsigned int t = 0; t < numThreads; t++) {
        int yStart = t * stripeHeight;
        int yEnd = (t == numThreads - 1) ? m_height : (t + 1) * stripeHeight;

        futures.push_back(std::async(std::launch::async, &Renderer::rasterizeStripe, this,
                                     yStart, yEnd, std::ref(triangles), texPixels, texSize));
    }

    for (auto& f : futures) f.get();
}

void Renderer::drawMesh(const std::vector<Vector3>& vertices, const std::vector<unsigned int>& indices,
                        const std::vector<sf::Vector2f>& uvs, const std::vector<sf::Color>& tints,
                        const sf::Image& image,
                        const Matrix4x4& model, const Matrix4x4& view, const Matrix4x4& proj, const Vector3& cameraPos, bool disableCulling) {
    Matrix4x4 mvp = proj * view * model;
    auto transformed = transformVertices(vertices, mvp, m_width * 0.5f, m_height * 0.5f);
    drawMeshInternal(transformed, vertices, indices, &uvs, &tints, &image, model, sf::Color::White, cameraPos, disableCulling);
}

void Renderer::display(sf::RenderWindow& window) {
    m_imageTexture.update(m_image);
    sf::Sprite sprite(m_imageTexture);
    sf::Vector2u windowSize = window.getSize();
    sprite.setScale({ (float)windowSize.x / m_width, (float)windowSize.y / m_height });
    window.draw(sprite);
}
