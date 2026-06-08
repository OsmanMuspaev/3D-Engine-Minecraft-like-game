#include "Camera.h"
#include "../world/World.h"
#include <cmath>
#include <algorithm>

constexpr float GRAVITY = 24.0f;
constexpr float MAX_FALL_SPEED = 40.0f;
constexpr float JUMP_VELOCITY = 8.5f;
constexpr float FLY_SPEED = 12.0f;
constexpr float DOUBLE_TAP_MS = 0.3f;

// Tests whether the player AABB overlaps any blocking block.
bool Camera::aabbTest(const World& world, float footX, float footY, float footZ, float checkHeight)
{
    float minX = footX - PLAYER_RADIUS;
    float maxX = footX + PLAYER_RADIUS;
    float minY = footY;
    float maxY = footY + checkHeight;
    float minZ = footZ - PLAYER_RADIUS;
    float maxZ = footZ + PLAYER_RADIUS;

    int bxMin = static_cast<int>(std::floor(minX));
    int bxMax = static_cast<int>(std::floor(maxX));
    int byMin = static_cast<int>(std::floor(minY));
    int byMax = static_cast<int>(std::floor(maxY));
    int bzMin = static_cast<int>(std::floor(minZ));
    int bzMax = static_cast<int>(std::floor(maxZ));

    for (int by = byMin; by <= byMax; by++)
        for (int bx = bxMin; bx <= bxMax; bx++)
            for (int bz = bzMin; bz <= bzMax; bz++)
                if (world.isBlocking((float)bx, (float)by, (float)bz))
                    return true;
    return false;
}

// Initializes orientation from target direction.
Camera::Camera(const Vector3& position, const Vector3& target, const Vector3& up)
    : m_position(position)
    , m_velocityY(0.0f)
    , m_mode(Mode::Survival)
{
    m_forward = (target - position).normalize();
    m_right = m_forward.cross(up).normalize();
    m_up = m_right.cross(m_forward).normalize();
    m_yaw = std::atan2(m_forward.z, m_forward.x);
    m_pitch = std::asin(std::clamp(m_forward.y, -1.0f, 1.0f));
}

// Moves along the forward axis with collision (survival/creative walking).
void Camera::moveForward(float distance, const World& world)
{
    if (m_mode == Mode::Spectator) {
        m_position = m_position + m_forward * distance;
        return;
    }

    if (m_mode == Mode::Creative && m_flying) {
        Vector3 flatForward = Vector3(m_forward.x, 0, m_forward.z).normalize();
        float nx = m_position.x + flatForward.x * distance;
        float nz = m_position.z + flatForward.z * distance;
        float feetY = m_position.y - PLAYER_HEIGHT;
        if (!aabbTest(world, nx, feetY, m_position.z)) m_position.x = nx;
        if (!aabbTest(world, m_position.x, feetY, nz)) m_position.z = nz;
        return;
    }

    Vector3 moveDir = Vector3(m_forward.x, 0, m_forward.z).normalize();
    float feetY = m_position.y - PLAYER_HEIGHT;
    // Use 1.0-block height check to prevent slipping through 1-block gaps.
    float checkHeight = 1.0f;

    float nx = m_position.x + moveDir.x * distance;
    if (!aabbTest(world, nx, feetY, m_position.z, checkHeight)) m_position.x = nx;

    float nz = m_position.z + moveDir.z * distance;
    if (!aabbTest(world, m_position.x, feetY, nz, checkHeight)) m_position.z = nz;
}

// Moves along the right axis with collision (survival/creative walking).
void Camera::moveRight(float distance, const World& world)
{
    if (m_mode == Mode::Spectator) {
        m_position = m_position + m_right * distance;
        return;
    }

    if (m_mode == Mode::Creative && m_flying) {
        Vector3 flatRight = Vector3(m_right.x, 0, m_right.z).normalize();
        float nx = m_position.x + flatRight.x * distance;
        float nz = m_position.z + flatRight.z * distance;
        float feetY = m_position.y - PLAYER_HEIGHT;
        if (!aabbTest(world, nx, feetY, m_position.z)) m_position.x = nx;
        if (!aabbTest(world, m_position.x, feetY, nz)) m_position.z = nz;
        return;
    }

    Vector3 moveDir = Vector3(m_right.x, 0, m_right.z).normalize();
    float feetY = m_position.y - PLAYER_HEIGHT;
    float checkHeight = 1.0f;

    float nx = m_position.x + moveDir.x * distance;
    if (!aabbTest(world, nx, feetY, m_position.z, checkHeight)) m_position.x = nx;

    float nz = m_position.z + moveDir.z * distance;
    if (!aabbTest(world, m_position.x, feetY, nz, checkHeight)) m_position.z = nz;
}

// Moves vertically (creative/spectator only).
void Camera::moveUp(float distance, const World& world)
{
    if (m_mode == Mode::Survival) return;

    if (m_mode == Mode::Spectator) {
        m_position.y += distance;
        return;
    }

    if (m_flying) {
        float nextY = m_position.y + distance;
        float feetY = nextY - PLAYER_HEIGHT;
        if (!aabbTest(world, m_position.x, feetY, m_position.z)) {
            m_position.y = nextY;
        }
    }
}

// Handles space bar press for jumping and creative double-tap fly toggle.
void Camera::handleSpacePress(const World& world, float currentTime)
{
    if (m_mode == Mode::Survival) {
        float headY = m_position.y;
        if (aabbTest(world, m_position.x, headY, m_position.z, 0.1f)) {
            float pushY = std::floor(headY) + 1.0f + PLAYER_HEIGHT;
            if (!aabbTest(world, m_position.x, pushY - PLAYER_HEIGHT, m_position.z)) {
                m_position.y = pushY;
                return;
            }
        }
        if (m_onGround) {
            m_velocityY = JUMP_VELOCITY;
            m_onGround = false;
        }
        return;
    }

    if (m_mode == Mode::Creative) {
        float diff = currentTime - m_lastSpaceTime;
        m_lastSpaceTime = currentTime;

        if (diff < DOUBLE_TAP_MS) {
            m_flying = !m_flying;
            m_velocityY = 0;
        } else if (m_onGround && !m_flying) {
            m_velocityY = JUMP_VELOCITY;
            m_onGround = false;
        }
    }
}

void Camera::handleSpaceRelease()
{
}

// Resets velocity when flying (creative mode).
void Camera::updateFlying(float dt, const World& world)
{
    if (m_mode != Mode::Creative || !m_flying) return;
    m_velocityY = 0;
}

// Returns true if the player is standing on a solid block.
bool Camera::isOnGround(const World& world) const
{
    return aabbTest(world, m_position.x, m_position.y - PLAYER_HEIGHT - 0.05f, m_position.z);
}

// Applies gravity, resolves vertical collisions, and updates ground state.
void Camera::updatePhysics(float dt, const World& world)
{
    if (m_mode == Mode::Spectator) return;

    m_onGround = isOnGround(world);

    if (m_mode == Mode::Creative && m_flying) {
        m_velocityY = 0;
        return;
    }

    m_velocityY -= GRAVITY * dt;
    m_velocityY = std::max(m_velocityY, -MAX_FALL_SPEED);

    float moveY = m_velocityY * dt;
    float stepY = 0.05f;
    float movedY = 0.0f;
    int dirY = (moveY >= 0) ? 1 : -1;

    while (std::abs(movedY) < std::abs(moveY))
    {
        float delta = dirY * std::min(stepY, std::abs(moveY) - std::abs(movedY));
        float nextY = m_position.y + delta;
        float feetY = nextY - PLAYER_HEIGHT;

        if (aabbTest(world, m_position.x, feetY, m_position.z))
        {
            if (m_velocityY < 0.0f) {
                m_position.y = std::floor(feetY) + 1.0f + PLAYER_HEIGHT;
                m_onGround = true;
            } else {
                m_position.y = std::floor(nextY) - 0.01f;
            }
            m_velocityY = 0.0f;
            break;
        }

        m_position.y = nextY;
        movedY += delta;
    }
}

// Applies yaw/pitch rotation and rebuilds the forward/right/up vectors.
void Camera::rotate(float yaw, float pitch)
{
    m_yaw += yaw;
    m_pitch += pitch;

    const float MAX_PITCH_UP = 1.57f;
    const float MAX_PITCH_DOWN = 1.57f;

    m_pitch = std::clamp(m_pitch, -MAX_PITCH_DOWN, MAX_PITCH_UP);

    m_forward = Vector3(
        std::cos(m_pitch) * std::cos(m_yaw),
        std::sin(m_pitch),
        std::cos(m_pitch) * std::sin(m_yaw)
    ).normalize();

    m_right = m_forward.cross(Vector3(0, 1, 0)).normalize();
    m_up = m_right.cross(m_forward).normalize();
}

// Cycles through Survival -> Creative -> Spectator -> Survival.
void Camera::cycleMode()
{
    switch (m_mode)
    {
        case Mode::Survival:  m_mode = Mode::Creative;  break;
        case Mode::Creative:  m_mode = Mode::Spectator; break;
        case Mode::Spectator: m_mode = Mode::Survival;  break;
    }
    m_velocityY = 0;
    m_flying = false;
}

Camera::Mode Camera::getMode() const { return m_mode; }
Vector3 Camera::getPosition() const { return m_position; }

Matrix4x4 Camera::getViewMatrix() const
{
    return Matrix4x4::lookAt(m_position, m_position + m_forward, Vector3(0, 1, 0));
}
