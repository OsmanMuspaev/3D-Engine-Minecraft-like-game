#pragma once
#include "../math/Vector3.h"
#include "World.h"
#include "Inventory.h"
#include "Item.h"
#include "../core/Camera.h"

// Result of a raycast against the world, storing hit info and surface normal.
struct RaycastResult {
    bool hit = false;
    Vector3 blockPos;
    Vector3 normal;
    float distance = 0.0f;
    BlockType blockType = BlockType::AIR;
};

// Handles player block interactions: breaking, placing, and raycasting.
class PlayerInteraction {
public:
    static constexpr float REACH_DISTANCE = 5.0f;
    static constexpr float BREAK_SPEED = 1.0f;

    PlayerInteraction();

    void update(float dt, const Camera& camera, World& world, Inventory& inventory);
    void handleLeftClick(const Camera& camera, World& world, Inventory& inventory);
    void handleRightClick(const Camera& camera, World& world, Inventory& inventory);
    void setBreakProgress(float progress) { m_breakProgress = progress; }
    float getBreakProgress() const { return m_breakProgress; }
    const Vector3* getTargetBlockPos() const { return m_currentTarget ? &m_targetBlockPos : nullptr; }

private:
    // Performs a step-based raycast and returns the first solid block hit.
    RaycastResult raycast(const Camera& camera, const World& world) const;

    // Checks if a block can be placed at the given position without overlapping the player.
    bool canPlaceBlock(const World& world, const Vector3& pos, const Camera& camera) const;

    float m_breakProgress = 0.0f;
    Vector3 m_targetBlockPos;
    bool m_currentTarget = false;
    float m_breakStartTime = 0.0f;
};
