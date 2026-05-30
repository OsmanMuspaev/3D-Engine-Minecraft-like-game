#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <optional>
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../math/Matrix4x4.h"

class Renderer {
public:
    Renderer(unsigned int width, unsigned int height);
    
    void clear(sf::Color color = sf::Color::Black);
    
    // С цветом
    void drawMesh(const std::vector<Vector3>& vertices,
                  const std::vector<unsigned int>& indices,
                  const Matrix4x4& model,
                  const Matrix4x4& view,
                  const Matrix4x4& proj,
                  sf::Color color,
                  const Vector3& cameraPos);
    
    // С текстурой
    void drawMesh(const std::vector<Vector3>& vertices,
                  const std::vector<unsigned int>& indices,
                  const std::vector<sf::Vector2f>& uvs,
                  const sf::Texture& texture,
                  const Matrix4x4& model,
                  const Matrix4x4& view,
                  const Matrix4x4& proj,
                  const Vector3& cameraPos);
    
    void display(sf::RenderWindow& window);
    void setLightDirection(const Vector3& direction);
    
private:
    sf::RenderTexture m_renderTexture;
    std::optional<sf::Sprite> m_sprite;
    sf::Color m_clearColor;
    Vector3 m_lightDir;
    
    // Общая часть для обоих drawMesh
    void drawMeshInternal(const std::vector<Vector4>& transformed,
                          const std::vector<Vector3>& vertices,
                          const std::vector<unsigned int>& indices,
                          const std::vector<sf::Vector2f>* uvs,
                          const sf::Texture* texture,
                          const Matrix4x4& model,
                          sf::Color baseColor,
                          const Vector3& cameraPos);
};