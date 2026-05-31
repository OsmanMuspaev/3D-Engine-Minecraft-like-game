#include "Camera.h"
#include "../world/World.h"
#include <cmath>
#include <algorithm>

// Константы игрока
constexpr float PLAYER_HEIGHT = 1.8f;
constexpr float PLAYER_RADIUS = 0.3f; // Немного увеличил для стабильности коллизий
constexpr float GRAVITY = 24.0f;      // Чуть выше для "приятного" веса
constexpr float MAX_FALL_SPEED = 40.0f;
constexpr float JUMP_VELOCITY = 8.5f;

bool Camera::aabbTest(const World& world, float footX, float footY, float footZ)
{
    float minX = footX - PLAYER_RADIUS;
    float maxX = footX + PLAYER_RADIUS;
    float minY = footY;
    float maxY = footY + PLAYER_HEIGHT;
    float minZ = footZ - PLAYER_RADIUS;
    float maxZ = footZ + PLAYER_RADIUS;

    // Проверяем блоки в радиусе хитбокса
    int bxMin = static_cast<int>(std::floor(minX));
    int bxMax = static_cast<int>(std::floor(maxX));
    int byMin = static_cast<int>(std::floor(minY));
    int byMax = static_cast<int>(std::floor(maxY));
    int bzMin = static_cast<int>(std::floor(minZ));
    int bzMax = static_cast<int>(std::floor(maxZ));

    for (int by = byMin; by <= byMax; by++) {
        for (int bx = bxMin; bx <= bxMax; bx++) {
            for (int bz = bzMin; bz <= bzMax; bz++) {
                if (world.isBlocking((float)bx, (float)by, (float)bz))
                    return true;
            }
        }
    }
    return false;
}

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

void Camera::moveForward(float distance, const World& world)
{
    if (m_mode == Mode::Spectator) {
        m_position = m_position + m_forward * distance;
        return;
    }

    // Движение только в плоскости XZ, чтобы не летать при взгляде вверх/вниз
    Vector3 moveDir = Vector3(m_forward.x, 0, m_forward.z).normalize();
    float feetY = m_position.y - PLAYER_HEIGHT;

    float nx = m_position.x + moveDir.x * distance;
    if (!aabbTest(world, nx, feetY, m_position.z)) m_position.x = nx;

    float nz = m_position.z + moveDir.z * distance;
    if (!aabbTest(world, m_position.x, feetY, nz)) m_position.z = nz;
}

void Camera::moveRight(float distance, const World& world)
{
    if (m_mode == Mode::Spectator) {
        m_position = m_position + m_right * distance;
        return;
    }

    Vector3 moveDir = Vector3(m_right.x, 0, m_right.z).normalize();
    float feetY = m_position.y - PLAYER_HEIGHT;

    float nx = m_position.x + moveDir.x * distance;
    if (!aabbTest(world, nx, feetY, m_position.z)) m_position.x = nx;

    float nz = m_position.z + moveDir.z * distance;
    if (!aabbTest(world, m_position.x, feetY, nz)) m_position.z = nz;
}

void Camera::moveUp(float distance)
{
    if (m_mode == Mode::Survival) return;
    m_position.y += distance;
}

void Camera::jump()
{
    // Прыгаем только если на земле и в режиме выживания
    if (m_onGround && m_mode == Mode::Survival) {
        m_velocityY = JUMP_VELOCITY;
        m_onGround = false;
    }
}

bool Camera::isOnGround(const World& world) const
{
    // Небольшой зазор под ногами для детекции земли
    return aabbTest(world, m_position.x, m_position.y - PLAYER_HEIGHT - 0.05f, m_position.z);
}

void Camera::updatePhysics(float dt, const World& world)
{
    if (m_mode == Mode::Spectator) return;

    // Обновляем состояние земли
    m_onGround = isOnGround(world);

    if (m_mode == Mode::Survival) {
        m_velocityY -= GRAVITY * dt;
        m_velocityY = std::max(m_velocityY, -MAX_FALL_SPEED);
    } else {
        m_velocityY = 0; // В Креативе нет гравитации
        return;
    }

    // Итеративное движение по Y (защита от пролета сквозь пол/потолок)
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
            if (m_velocityY < 0.0f) // Ударились о землю
            {
                m_position.y = std::floor(feetY) + 1.0f + PLAYER_HEIGHT;
                m_onGround = true;
            } 
            else // Ударились головой о потолок
            {
                m_position.y = std::floor(nextY) - 0.01f; 
            }
            m_velocityY = 0.0f;
            break;
        }

        m_position.y = nextY;
        movedY += delta;
    }
}

void Camera::rotate(float yaw, float pitch)
{
    m_yaw += yaw;
    m_pitch += pitch;
    
    // Ограничение взгляда (85 градусов)
    m_pitch = std::clamp(m_pitch, -1.48f, 1.48f);

    m_forward = Vector3(
        std::cos(m_pitch) * std::cos(m_yaw),
        std::sin(m_pitch),
        std::cos(m_pitch) * std::sin(m_yaw)
    ).normalize();

    m_right = m_forward.cross(Vector3(0, 1, 0)).normalize();
    m_up = m_right.cross(m_forward).normalize();
}

void Camera::cycleMode()
{
    switch (m_mode)
    {
        case Mode::Survival:  m_mode = Mode::Creative;  break;
        case Mode::Creative:  m_mode = Mode::Spectator; break;
        case Mode::Spectator: m_mode = Mode::Survival;  break;
    }
    m_velocityY = 0; // Сброс скорости при смене режима
}

Camera::Mode Camera::getMode() const { return m_mode; }
Vector3 Camera::getPosition() const { return m_position; }

Matrix4x4 Camera::getViewMatrix() const
{
    return Matrix4x4::lookAt(m_position, m_position + m_forward, Vector3(0, 1, 0));
}