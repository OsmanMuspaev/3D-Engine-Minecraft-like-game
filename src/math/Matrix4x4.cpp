#include <iostream>
#include "Matrix4x4.h"

// Конструктор по умолчанию
Matrix4x4::Matrix4x4() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (i == j) {
                m[i][j] = 1.0f;
            } else {
                m[i][j] = 0.0f;
            }
        }
    }
}

// Значение по индексу
float* Matrix4x4::operator[](int row) {
    return m[row];
}

// Значение по индексу (константа)
const float* Matrix4x4::operator[](int row) const {
    return m[row];
}

// Умножение 
Matrix4x4 Matrix4x4::operator*(const Matrix4x4& other) const {
    Matrix4x4 result;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.m[i][j] = m[i][0] * other.m[0][j] +
                        m[i][1] * other.m[1][j] +
                        m[i][2] * other.m[2][j] +
                        m[i][3] * other.m[3][j];
        }
    }
    return result;
}

// Умножение на вектор
Vector4 Matrix4x4::operator*(const Vector4& v) const {
    return Vector4(
        m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3]*v.w,
        m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3]*v.w,
        m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3]*v.w,
        m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3]*v.w
    );
}




// Статические методы для создания матриц
Matrix4x4 Matrix4x4::identity() {
    return Matrix4x4();
}

Matrix4x4 Matrix4x4::translation(float x, float y, float z) {
    Matrix4x4 result;
    
    result.m[0][3] = x;
    result.m[1][3] = y;
    result.m[2][3] = z;
    
    return result;
}

Matrix4x4 Matrix4x4::scale(float x, float y, float z) {
    Matrix4x4 result;
    
    result.m[0][0] = x;
    result.m[1][1] = y;
    result.m[2][2] = z;
    
    return result;
}

Matrix4x4 Matrix4x4::rotationX(float angle) {
    Matrix4x4 result;
    
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[1][1] = c;
    result.m[1][2] = -s;
    result.m[2][1] = s;
    result.m[2][2] = c;
    
    return result;
}

Matrix4x4 Matrix4x4::rotationY(float angle) {
    Matrix4x4 result;
    
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[0][0] = c;
    result.m[0][2] = s;
    result.m[2][0] = -s;
    result.m[2][2] = c;
    
    return result;
}

Matrix4x4 Matrix4x4::rotationZ(float angle) {
    Matrix4x4 result;
    
    float c = std::cos(angle);
    float s = std::sin(angle);
    
    result.m[0][0] = c;
    result.m[0][1] = -s;
    result.m[1][0] = s;
    result.m[1][1] = c;
    
    return result;
}

Matrix4x4 Matrix4x4::perspective(float fov, float aspect, float near, float far) {
    Matrix4x4 result;
    
    float tanHalfFov = std::tan(fov * 3.14159f / 180.0f / 2.0f);
    
    result.m[0][0] = 1.0f / (aspect * tanHalfFov);
    result.m[1][1] = 1.0f / tanHalfFov;
    result.m[2][2] = -(far + near) / (far - near);
    result.m[2][3] = -(2.0f * far * near) / (far - near);
    result.m[3][2] = -1.0f;
    result.m[3][3] = 0.0f;
    
    return result;
}

Matrix4x4 Matrix4x4::lookAt(const Vector3& eye, const Vector3& target, const Vector3& up) {
    Vector3 forward = (target - eye).normalize();
    
    Vector3 right = forward.cross(up).normalize();
    
    Vector3 realUp = right.cross(forward);
    
    Matrix4x4 result;
    
    result.m[0][0] = right.x;
    result.m[0][1] = right.y;
    result.m[0][2] = right.z;
    
    result.m[1][0] = realUp.x;
    result.m[1][1] = realUp.y;
    result.m[1][2] = realUp.z;
    
    result.m[2][0] = -forward.x;
    result.m[2][1] = -forward.y;
    result.m[2][2] = -forward.z;
    
    result.m[0][3] = -right.dot(eye);
    result.m[1][3] = -realUp.dot(eye);
    result.m[2][3] = forward.dot(eye);
    
    return result;
}