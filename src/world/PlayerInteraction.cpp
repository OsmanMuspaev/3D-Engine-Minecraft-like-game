#include "PlayerInteraction.h"
#include "Block.h"
#include <cmath>
#include <algorithm>

PlayerInteraction::PlayerInteraction() = default;

// Performs a step-based raycast from the camera position along its forward direction.
RaycastResult PlayerInteraction::raycast(const Camera& camera, const World& world) const {
    RaycastResult result;
    Vector3 pos = camera.getPosition();
    Vector3 dir = camera.getForward().normalize();

    float maxDist = REACH_DISTANCE;
    float step = 0.05f;

    Vector3 lastPos = pos;
    for (float dist = 0; dist < maxDist; dist += step) {
        Vector3 current = pos + dir * dist;
        int bx = static_cast<int>(std::floor(current.x));
        int by = static_cast<int>(std::floor(current.y));
        int bz = static_cast<int>(std::floor(current.z));

        Block block = world.getBlock(bx, by, bz);
        if (block.isSolid()) {
            result.hit = true;
            result.blockPos = Vector3(static_cast<float>(bx), static_cast<float>(by), static_cast<float>(bz));
            result.blockType = block.type;
            result.distance = dist;

            // Determine surface normal from the last step position
            int lbx = static_cast<int>(std::floor(lastPos.x));
            int lby = static_cast<int>(std::floor(lastPos.y));
            int lbz = static_cast<int>(std::floor(lastPos.z));

            if (bx != lbx) result.normal = (bx > lbx) ? Vector3(-1, 0, 0) : Vector3(1, 0, 0);
            else if (by != lby) result.normal = (by > lby) ? Vector3(0, -1, 0) : Vector3(0, 1, 0);
            else if (bz != lbz) result.normal = (bz > lbz) ? Vector3(0, 0, -1) : Vector3(0, 0, 1);

            return result;
        }
        lastPos = current;
    }

    return result;
}

// Checks whether placing a block at pos would collide with the player's body.
bool PlayerInteraction::canPlaceBlock(const World& world, const Vector3& pos, const Camera& camera) const {
    int bx = static_cast<int>(pos.x);
    int by = static_cast<int>(pos.y);
    int bz = static_cast<int>(pos.z);

    Block block = world.getBlock(bx, by, bz);
    if (block.isSolid()) return false;

    Vector3 playerPos = camera.getPosition();
    float playerFootY = playerPos.y - PLAYER_HEIGHT;
    float playerHeadY = playerPos.y;

    if ((bx == static_cast<int>(playerPos.x) || bx == static_cast<int>(playerPos.x) - 1) &&
        (bz == static_cast<int>(playerPos.z) || bz == static_cast<int>(playerPos.z) - 1)) {
        if (by >= static_cast<int>(playerFootY) && by <= static_cast<int>(playerHeadY)) {
            return false;
        }
    }

    return true;
}

// Updates the current target block via raycast each frame.
void PlayerInteraction::update(float dt, const Camera& camera, World& world, Inventory& inventory) {
    RaycastResult result = raycast(camera, world);

    if (result.hit && result.blockType != BlockType::BEDROCK) {
        m_currentTarget = true;
        m_targetBlockPos = result.blockPos;
    } else {
        m_currentTarget = false;
        m_breakProgress = 0.0f;
    }
}

// Handles left click: breaks the targeted block and adds it to inventory in survival mode.
void PlayerInteraction::handleLeftClick(const Camera& camera, World& world, Inventory& inventory) {
    RaycastResult result = raycast(camera, world);

    if (!result.hit) return;

    if (result.blockType == BlockType::BEDROCK) return;

    Camera::Mode mode = camera.getMode();

    if (mode == Camera::Mode::Survival) {
        world.setBlock(
            static_cast<int>(result.blockPos.x),
            static_cast<int>(result.blockPos.y),
            static_cast<int>(result.blockPos.z),
            BlockType::AIR
        );

        const Item& item = ItemRegistry::instance().getItem(result.blockType);
        if (!item.name.empty() && item.blockType != BlockType::AIR) {
            inventory.addItem(result.blockType, 1);
        }
    } else if (mode == Camera::Mode::Creative) {
        world.setBlock(
            static_cast<int>(result.blockPos.x),
            static_cast<int>(result.blockPos.y),
            static_cast<int>(result.blockPos.z),
            BlockType::AIR
        );
    }
}

// Handles right click: places the selected block on the adjacent face of the targeted block.
void PlayerInteraction::handleRightClick(const Camera& camera, World& world, Inventory& inventory) {
    RaycastResult result = raycast(camera, world);

    if (!result.hit) return;

    Vector3 placePos = result.blockPos + result.normal;

    if (!canPlaceBlock(world, placePos, camera)) return;

    ItemStack& selectedItem = inventory.getSelectedItem();

    if (selectedItem.isEmpty()) return;

    const Item& item = ItemRegistry::instance().getItem(selectedItem.type);
    if (item.blockType == BlockType::AIR) return;
    if (Inventory::isArmor(selectedItem.type)) return;

    Camera::Mode mode = camera.getMode();

    if (mode == Camera::Mode::Survival) {
        world.setBlock(
            static_cast<int>(placePos.x),
            static_cast<int>(placePos.y),
            static_cast<int>(placePos.z),
            item.blockType
        );
        selectedItem.remove(1);
    } else if (mode == Camera::Mode::Creative) {
        world.setBlock(
            static_cast<int>(placePos.x),
            static_cast<int>(placePos.y),
            static_cast<int>(placePos.z),
            item.blockType
        );
    }
}
