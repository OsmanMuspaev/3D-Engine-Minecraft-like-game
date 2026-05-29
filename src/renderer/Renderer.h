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
    
    // Очистка буфера
    void clear(sf::Color color = sf::Color::Black);
    
    // Нарисовать треугольник (вершины уже в экранных координатах)
    void drawTriangle(const Vector4& v0, const Vector4& v1, const Vector4& v2, sf::Color color);
    
    // Нарисовать меш с трансформациями
    void drawMesh(const std::vector<Vector3>& vertices,
                  const std::vector<unsigned int>& indices,
                  const Matrix4x4& model,
                  const Matrix4x4& view,
                  const Matrix4x4& proj,
                  const sf::Color color,
                  const Vector3& cameraPos);
    
    // Показать буфер на экране
    void display(sf::RenderWindow& window);

    // Задать направление света
    void setLightDirection(const Vector3& direction);
    
private:
    sf::Image m_frameBuffer;
    sf::Texture m_texture;
    std::optional<sf::Sprite> m_sprite;
    std::vector<float> m_zBuffer;
    unsigned int m_width;
    unsigned int m_height;
    
    // Поставить пиксель с проверкой Z-буфера
    void setPixel(int x, int y, float z, sf::Color color);
    
    // Нарисовать горизонтальную линию между двумя точками
    void drawScanLine(int y, const Vector4& left, const Vector4& right, sf::Color color);

    // Свет
    Vector3 m_lightDir;
};