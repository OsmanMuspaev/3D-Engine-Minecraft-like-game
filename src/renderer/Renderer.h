#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <limits>
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../math/Matrix4x4.h"

struct RenderTriangle {
    Vector4 v0, v1, v2;
    sf::Vector2f uv0, uv1, uv2;
    sf::Color color;
    float invDenom;
    float invW0, invW1, invW2;
};

class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);

    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }

    void setLightDirection(const Vector3& dir);
    void clear(sf::Color color);

    void drawMesh(const std::vector<Vector3>& vertices,
                  const std::vector<unsigned int>& indices,
                  const Matrix4x4& model,
                  const Matrix4x4& view,
                  const Matrix4x4& proj,
                  sf::Color color,
                  const Vector3& cameraPos);

    void drawMesh(const std::vector<Vector3>& vertices,
                const std::vector<unsigned int>& indices,
                const std::vector<sf::Vector2f>& uvs,
                const sf::Image& image,
                const Matrix4x4& model,
                const Matrix4x4& view,
                const Matrix4x4& proj,
                const Vector3& cameraPos);

    void display(sf::RenderWindow& window);

private:
    unsigned int m_width, m_height;
    sf::Image m_image;
    sf::Texture m_imageTexture;
    std::vector<float> m_depthBuffer;
    Vector3 m_lightDir;
    sf::Color m_clearColor;

    void rasterizeTriangle(
        const Vector4& v0, const Vector4& v1, const Vector4& v2,
        sf::Color c0, sf::Color c1, sf::Color c2,
        sf::Vector2f uv0, sf::Vector2f uv1, sf::Vector2f uv2,
        const uint8_t* texPixels, sf::Vector2u texSize);

    void rasterizeStripe(int yStart, int yEnd, const std::vector<RenderTriangle>& triangles, 
                     const uint8_t* texPixels, sf::Vector2u texSize);

    void drawMeshInternal(
        const std::vector<Vector4>& transformed,
        const std::vector<Vector3>& vertices,
        const std::vector<unsigned int>& indices,
        const std::vector<sf::Vector2f>* uvs,
        const sf::Image* image,
        const Matrix4x4& model,
        sf::Color baseColor,
        const Vector3& cameraPos);
};