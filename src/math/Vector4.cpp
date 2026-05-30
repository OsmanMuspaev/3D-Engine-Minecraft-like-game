#include <cmath>
#include "Vector4.h"

// Конструктор по умолчанию
Vector4::Vector4() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}

// Конструктор с параметрами 
Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

// Сложение 
Vector4 Vector4::operator+(const Vector4& other) const {
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

// Вычитание
Vector4 Vector4::operator-(const Vector4& other) const {
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

// Умножение на число
Vector4 Vector4::operator*(float scalar) const {
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

// Скалярное произведение
float Vector4::dot(const Vector4& other) const {
    return (x * other.x) + (y * other.y) + (z * other.z) + (w * other.w);
}

// Длина вектора
float Vector4::length() const {
    return std::sqrt((x*x) + (y*y) + (z*z) + (w*w));
}

// Нормализация вектора
Vector4 Vector4::normalize() const {
    float len = length();
        if (len > 0.0001f) {
        float invLen = 1.0f / len;
        return Vector4(x * invLen, y * invLen, z * invLen, w * invLen);
    }
    return Vector4(0.0f, 0.0f, 0.0f, 0.0f);
}