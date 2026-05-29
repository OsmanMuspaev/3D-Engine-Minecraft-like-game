#pragma once
#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"

class Camera {
public:
    Camera(const Vector3& position, const Vector3& target, const Vector3& up);
    
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    void rotate(float yaw, float pitch);
    
    Matrix4x4 getViewMatrix() const;
    Vector3 getPosition() const;
    
private:
    Vector3 m_position;
    Vector3 m_forward;
    Vector3 m_up;
    Vector3 m_right;
    
    float m_yaw;
    float m_pitch;
    
    void updateVectors();
};