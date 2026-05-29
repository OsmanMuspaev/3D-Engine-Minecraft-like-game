#include "Camera.h"
#include <cmath>

Camera::Camera(const Vector3& position, const Vector3& target, const Vector3& up)
    : m_position(position)
    , m_yaw(-1.57f)
    , m_pitch(0.0f)
{
    m_forward = (target - position).normalize();
    m_right = m_forward.cross(up).normalize();
    m_up = m_right.cross(m_forward).normalize();
}

void Camera::moveForward(float distance) {
    m_position = m_position + m_forward * distance;
}

void Camera::moveRight(float distance) {
    m_position = m_position + m_right * distance;
}

void Camera::moveUp(float distance) {
    m_position = m_position + Vector3(0, 1, 0) * distance;
}

void Camera::rotate(float yaw, float pitch) {
    m_yaw += yaw;
    m_pitch += pitch;
    
    if (m_pitch > 1.5f) m_pitch = 1.5f;
    if (m_pitch < -1.5f) m_pitch = -1.5f;
    
    m_forward = Vector3(
        std::cos(m_pitch) * std::cos(m_yaw),
        std::sin(m_pitch),
        std::cos(m_pitch) * std::sin(m_yaw)
    ).normalize();
    
    m_right = m_forward.cross(Vector3(0, 1, 0)).normalize();
    m_up = m_right.cross(m_forward).normalize();
}

Matrix4x4 Camera::getViewMatrix() const {
    Vector3 target = m_position + m_forward;
    return Matrix4x4::lookAt(m_position, target, Vector3(0, 1, 0));
}

Vector3 Camera::getPosition() const {
    return m_position;
}