#pragma once
#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"

class World;

class Camera {
public:
    enum class Mode { Survival, Creative, Spectator };

    Camera(const Vector3& position, const Vector3& target, const Vector3& up);

    void moveForward(float distance, const World& world);
    void moveRight(float distance, const World& world);
    void moveUp(float distance);
    void jump();
    bool isOnGround(const World& world) const;
    void updatePhysics(float deltaTime, const World& world);
    void rotate(float yaw, float pitch);
    void cycleMode();

    Mode getMode() const;
    Vector3 getPosition() const;
    Matrix4x4 getViewMatrix() const;

private:
    static bool aabbTest(const World& world, float footX, float footY, float footZ);

    Vector3 m_position;
    Vector3 m_forward;
    Vector3 m_up;
    Vector3 m_right;

    float m_yaw, m_pitch;
    float m_velocityY = 0.0f;
    bool m_onGround = false;
    Mode m_mode = Mode::Survival;
};
