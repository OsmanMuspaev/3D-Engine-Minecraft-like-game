#pragma once
#include "Vector4.h"
#include "Vector3.h"

class Matrix4x4 {
public:
    float m[4][4];  // m[строка][столбец]

    Matrix4x4();  // единичная матрица

    // Доступ по индексу
    float* operator[](int row);
    const float* operator[](int row) const;

    // Умножение матриц
    Matrix4x4 operator*(const Matrix4x4& other) const;

    // Умножение матрицы на вектор
    Vector4 operator*(const Vector4& v) const;

    // Статические методы для создания матриц
    static Matrix4x4 identity();
    static Matrix4x4 translation(float x, float y, float z);
    static Matrix4x4 scale(float x, float y, float z);
    static Matrix4x4 rotationX(float angle);
    static Matrix4x4 rotationY(float angle);
    static Matrix4x4 rotationZ(float angle);
    static Matrix4x4 perspective(float fov, float aspect, float near, float far);
    static Matrix4x4 lookAt(const Vector3& eye, const Vector3& target, const Vector3& up);
};