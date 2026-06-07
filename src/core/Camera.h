#pragma once
#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"

class World;

constexpr float PLAYER_HEIGHT = 1.8f;
constexpr float PLAYER_RADIUS = 0.3f;

class Camera {
public:
    enum class Mode { Survival, Creative, Spectator };

    Camera(const Vector3& position, const Vector3& target, const Vector3& up);

    void moveForward(float distance, const World& world);
    void moveRight(float distance, const World& world);
    void moveUp(float distance, const World& world);
    void handleSpacePress(const World& world, float currentTime);
    void handleSpaceRelease();
    void updateFlying(float dt, const World& world);
    bool isOnGround(const World& world) const;
    void updatePhysics(float deltaTime, const World& world);
    void rotate(float yaw, float pitch);
    void cycleMode();

    Mode getMode() const;
    bool isFlying() const { return m_flying; }
    Vector3 getPosition() const;
    Matrix4x4 getViewMatrix() const;
    Vector3 getForward() const { return m_forward; }

private:
    static bool aabbTest(const World& world, float footX, float footY, float footZ, float checkHeight = PLAYER_HEIGHT);

    Vector3 m_position;
    Vector3 m_forward;
    Vector3 m_up;
    Vector3 m_right;

    float m_yaw, m_pitch;
    float m_velocityY = 0.0f;
    bool m_onGround = false;
    bool m_flying = false;
    Mode m_mode = Mode::Survival;
    float m_lastSpaceTime = 0.0f;
    bool m_spaceWasDown = false;
};
