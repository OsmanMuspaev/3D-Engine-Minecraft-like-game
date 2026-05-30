#include <cmath>
#include "Vector3.h"

// Конструктор по умолчанию
Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

// Конструктор с параметрами 
Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

// Сложение 
Vector3 Vector3::operator+(const Vector3& other) const {
    return Vector3(x + other.x, y + other.y, z + other.z);
}

// Вычитание
Vector3 Vector3::operator-(const Vector3& other) const {
    return Vector3(x - other.x, y - other.y, z - other.z);
}

// Умножение на число
Vector3 Vector3::operator*(float scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
}

// Скалярное произведение
float Vector3::dot(const Vector3& other) const {
    return (x * other.x) + (y * other.y) + (z * other.z);
}

// Векторное произведение
Vector3 Vector3::cross(const Vector3& other) const {
    return Vector3(
        (y * other.z) - (z * other.y),
        (z * other.x) - (x * other.z),
        (x * other.y) - (y * other.x)
    );
}

// Длина вектора
float Vector3::length() const {
    return std::sqrt((x*x) + (y*y) + (z*z));
}

// Нормализация вектора
Vector3 Vector3::normalize() const {
    float len = length();
        if (len > 0.0001f) {
        float invLen = 1.0f / len;
        return Vector3(x * invLen, y * invLen, z * invLen);
    }
    return Vector3(0.0f, 0.0f, 0.0f);
}