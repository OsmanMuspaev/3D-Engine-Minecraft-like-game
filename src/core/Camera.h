#pragma once
#include "../math/Vector3.h"
#include "../math/Matrix4x4.h"

class World;

constexpr float PLAYER_HEIGHT = 1.8f;
constexpr float PLAYER_RADIUS = 0.35f;

class Camera {
public:
    enum class Mode { Survival, Creative, Spectator };

    // Initializes camera at position looking toward target.
    Camera(const Vector3& position, const Vector3& target, const Vector3& up);

    // Moves along the forward axis with collision.
    void moveForward(float distance, const World& world);
    // Moves along the right axis with collision.
    void moveRight(float distance, const World& world);
    // Moves vertically (creative/spectator only).
    void moveUp(float distance, const World& world);
    // Handles space bar press for jumping and creative fly toggle.
    void handleSpacePress(const World& world, float currentTime);
    void handleSpaceRelease();
    // Resets velocity when flying.
    void updateFlying(float dt, const World& world);
    // Returns true if the player is standing on a solid block.
    bool isOnGround(const World& world) const;
    // Applies gravity, resolves vertical collisions, and updates ground state.
    void updatePhysics(float deltaTime, const World& world);
    // Applies yaw/pitch rotation and rebuilds direction vectors.
    void rotate(float yaw, float pitch);
    // Cycles through Survival, Creative, and Spectator modes.
    void cycleMode();

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
    // Tests AABB collision against the world.
    static bool aabbTest(const World& world, float footX, float footY, float footZ, float checkHeight = PLAYER_HEIGHT);
    bool aabbTest(const World& world, const Vector3& pos) const;
    bool aabbTest(const World& world, const Vector3& min, const Vector3& max) const;

    Vector3 m_position;
    Vector3 m_forward;
    Vector3 m_up;
    Vector3 m_right;

    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
    float m_velocityY = 0.0f;
    bool m_onGround = false;
    bool m_flying = false;
    Mode m_mode = Mode::Survival;
    float m_lastSpaceTime = 0.0f;
    bool m_spaceWasDown = false;
};
