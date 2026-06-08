#pragma once
#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"

class World;

constexpr float PLAYER_HEIGHT = 1.8f;
constexpr float PLAYER_RADIUS = 0.35f;

// Camera with physics, movement, and collision detection.
class Camera {
public:
    enum class Mode { Survival, Creative, Spectator };

    Camera(const Vector3& position, const Vector3& target, const Vector3& up);

    // Movement (each axis independently, with collision).
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

    // Accessors.
    Mode getMode() const;
    bool isFlying() const { return m_flying; }
    Vector3 getPosition() const;
    Matrix4x4 getViewMatrix() const;
    Vector3 getForward() const { return m_forward; }
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }

    // Returns the player AABB at a given position.
    void getBoundingBox(const Vector3& position, Vector3& outMin, Vector3& outMax) const {
        outMin = Vector3(position.x - PLAYER_RADIUS, position.y - PLAYER_HEIGHT, position.z - PLAYER_RADIUS);
        outMax = Vector3(position.x + PLAYER_RADIUS, position.y, position.z + PLAYER_RADIUS);
    }

private:
    // AABB collision tests against the world.
    static bool aabbTest(const World& world, float footX, float footY, float footZ, float checkHeight = PLAYER_HEIGHT);
    bool aabbTest(const World& world, const Vector3& pos) const;
    bool aabbTest(const World& world, const Vector3& min, const Vector3& max) const;

    // Transform state.
    Vector3 m_position;
    Vector3 m_forward;
    Vector3 m_up;
    Vector3 m_right;

    // Orientation and physics.
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_velocityY = 0.0f;
    bool m_onGround = false;
    bool m_flying = false;
    Mode m_mode = Mode::Survival;
    float m_lastSpaceTime = 0.0f;
    bool m_spaceWasDown = false;
};
