#pragma once

// 3D vector with basic arithmetic and geometric operations
class Vector3 {
public:
    float x, y, z;

    Vector3();
    Vector3(float x, float y, float z);

    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator*(float scalar) const;

    // Dot product
    float dot(const Vector3& other) const;
    // Cross product
    Vector3 cross(const Vector3& other) const;

    float length() const;
    // Returns a unit vector; zero vector if length is near zero
    Vector3 normalize() const;
};
