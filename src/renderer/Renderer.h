#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <limits>
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../math/Matrix4x4.h"

// Triangle data prepared for rasterization
struct RenderTriangle {
    Vector4 v0, v1, v2;
    sf::Vector2f uv0, uv1, uv2;
    sf::Color color;
    sf::Color tint;
    float invDenom;
    float invW0, invW1, invW2;
};

// Software rasterizer with depth buffer and texture support
class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);

    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }

    void setLightDirection(const Vector3& dir);
    void clear(sf::Color color);
    void clearRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, sf::Color color);

    // Draws a textured or flat-colored mesh with per-vertex tinting
    void drawMesh(const std::vector<Vector3>& vertices,
                  const std::vector<unsigned int>& indices,
                  const std::vector<sf::Vector2f>& uvs,
                  const std::vector<sf::Color>& tints,
                  const sf::Image& image,
                  const Matrix4x4& model,
                  const Matrix4x4& view,
                  const Matrix4x4& proj,
                  const Vector3& cameraPos,
                  bool disableCulling = false);

    // Copies the internal image to the SFML render window
    void display(sf::RenderWindow& window);

private:
    unsigned int m_width, m_height;
    sf::Image m_image;
    sf::Texture m_imageTexture;
    std::vector<float> m_depthBuffer;
    Vector3 m_lightDir;
    sf::Color m_clearColor;

    // Rasterizes triangles within a horizontal stripe [yStart, yEnd)
    void rasterizeStripe(int yStart, int yEnd, const std::vector<RenderTriangle>& triangles,
                         const uint8_t* texPixels, sf::Vector2u texSize);

    // Core mesh drawing: builds triangles, culls, lights, and dispatches rasterization
    void drawMeshInternal(
        const std::vector<Vector4>& transformed,
        const std::vector<Vector3>& vertices,
        const std::vector<unsigned int>& indices,
        const std::vector<sf::Vector2f>* uvs,
        const std::vector<sf::Color>* tints,
        const sf::Image* image,
        const Matrix4x4& model,
        sf::Color baseColor,
        const Vector3& cameraPos,
        bool disableCulling = false);
};
