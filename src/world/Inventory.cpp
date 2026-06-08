#include "Inventory.h"
#include <algorithm>

// Initialize all slots to empty stacks.
Inventory::Inventory() {
    m_slots.fill(ItemStack());
    m_armorSlots.fill(ItemStack());
}

// Return a clamped reference to a main inventory slot by flat index.
ItemStack& Inventory::getSlot(int index) {
    index = std::clamp(index, 0, TOTAL_SIZE - 1);
    return m_slots[index];
}

const ItemStack& Inventory::getSlot(int index) const {
    index = std::clamp(index, 0, TOTAL_SIZE - 1);
    return m_slots[index];
}

// Return a clamped reference to a hotbar slot (indices 0-8).
ItemStack& Inventory::getHotbarSlot(int index) {
    index = std::clamp(index, 0, HOTBAR_SIZE - 1);
    return m_slots[index];
}

const ItemStack& Inventory::getHotbarSlot(int index) const {
    index = std::clamp(index, 0, HOTBAR_SIZE - 1);
    return m_slots[index];
}

// Return a reference to a slot in the 3x9 main grid using row and column.
ItemStack& Inventory::getMainSlot(int row, int col) {
    row = std::clamp(row, 0, MAIN_ROWS - 1);
    col = std::clamp(col, 0, MAIN_COLS - 1);
    int index = HOTBAR_SIZE + row * MAIN_COLS + col;
    return m_slots[index];
}

const ItemStack& Inventory::getMainSlot(int row, int col) const {
    row = std::clamp(row, 0, MAIN_ROWS - 1);
    col = std::clamp(col, 0, MAIN_COLS - 1);
    int index = HOTBAR_SIZE + row * MAIN_COLS + col;
    return m_slots[index];
}

// Return a reference to one of the four armor slots.
ItemStack& Inventory::getArmorSlot(int index) {
    return m_armorSlots[index];
}

const ItemStack& Inventory::getArmorSlot(int index) const {
    return m_armorSlots[index];
}

// Check whether a block type is any piece of armor.
bool Inventory::isArmor(BlockType type) {
    switch (type) {
        case BlockType::LEATHER_HELMET:
        case BlockType::LEATHER_CHESTPLATE:
        case BlockType::LEATHER_LEGGINGS:
        case BlockType::LEATHER_BOOTS:
        case BlockType::IRON_HELMET:
        case BlockType::IRON_CHESTPLATE:
        case BlockType::IRON_LEGGINGS:
        case BlockType::IRON_BOOTS:
        case BlockType::DIAMOND_HELMET:
        case BlockType::DIAMOND_CHESTPLATE:
        case BlockType::DIAMOND_LEGGINGS:
        case BlockType::DIAMOND_BOOTS:
        case BlockType::GOLDEN_HELMET:
        case BlockType::GOLDEN_CHESTPLATE:
        case BlockType::GOLDEN_LEGGINGS:
        case BlockType::GOLDEN_BOOTS:
        case BlockType::CHAINMAIL_HELMET:
        case BlockType::CHAINMAIL_CHESTPLATE:
        case BlockType::CHAINMAIL_LEGGINGS:
        case BlockType::CHAINMAIL_BOOTS:
        case BlockType::NETHERITE_HELMET:
        case BlockType::NETHERITE_CHESTPLATE:
        case BlockType::NETHERITE_LEGGINGS:
        case BlockType::NETHERITE_BOOTS:
            return true;
        default:
            return false;
    }
}

// Wrap the hotbar selection index around valid range.
void Inventory::cycleHotbar(int delta) {
    m_selectedHotbar = (m_selectedHotbar + delta) % HOTBAR_SIZE;
    if (m_selectedHotbar < 0) m_selectedHotbar += HOTBAR_SIZE;
}

// Attempt to add items by type; delegates to the stack overload.
bool Inventory::addItem(BlockType type, int count) {
    return addItem(ItemStack(type, count));
}

// Merge items into existing stacks first, then fill empty slots.
bool Inventory::addItem(const ItemStack& stack) {
    if (stack.isEmpty()) return true;

    int remaining = stack.count;
    BlockType type = stack.type;

    for (int i = 0; i < TOTAL_SIZE && remaining > 0; i++) {
        ItemStack& slot = m_slots[i];
        if (slot.type == type && slot.count < slot.getMaxStackSize()) {
            int space = slot.getFreeSpace();
            int toAdd = std::min(space, remaining);
            slot.add(toAdd);
            remaining -= toAdd;
        }
    }

    for (int i = 0; i < TOTAL_SIZE && remaining > 0; i++) {
        ItemStack& slot = m_slots[i];
        if (slot.isEmpty()) {
            int toAdd = std::min(slot.getMaxStackSize(), remaining);
            slot.type = type;
            slot.count = toAdd;
            remaining -= toAdd;
        }
    }

    return remaining == 0;
}

// Remove a specific count of items from a slot and return them.
ItemStack Inventory::removeItem(int index, int count) {
    ItemStack& slot = getSlot(index);
    if (slot.isEmpty()) return ItemStack();

    int toRemove = std::min(count, slot.count);
    ItemStack result(slot.type, toRemove);
    slot.remove(toRemove);
    return result;
}

// Take the entire contents of a slot, clearing it.
ItemStack Inventory::takeItem(int index) {
    ItemStack& slot = getSlot(index);
    ItemStack result = slot;
    slot = ItemStack();
    return result;
}

// Swap the contents of two slots.
void Inventory::swapSlots(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= TOTAL_SIZE || toIndex < 0 || toIndex >= TOTAL_SIZE) return;
    if (fromIndex == toIndex) return;
    std::swap(m_slots[fromIndex], m_slots[toIndex]);
}

// Split a stack by moving half into the target slot.
void Inventory::splitStack(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= TOTAL_SIZE || toIndex < 0 || toIndex >= TOTAL_SIZE) return;
    if (fromIndex == toIndex) return;

    ItemStack& from = m_slots[fromIndex];
    ItemStack& to = m_slots[toIndex];

    if (from.isEmpty()) return;

    if (to.isEmpty()) {
        int half = (from.count + 1) / 2;
        to.type = from.type;
        to.count = half;
        from.count -= half;
        if (from.count == 0) from.type = BlockType::AIR;
    } else if (from.canStackWith(to)) {
        int space = to.getFreeSpace();
        if (space > 0) {
            int move = std::min((from.count + 1) / 2, space);
            to.add(move);
            from.remove(move);
        }
    }
}

// Move or merge a stack into another slot.
void Inventory::moveStack(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= TOTAL_SIZE || toIndex < 0 || toIndex >= TOTAL_SIZE) return;
    if (fromIndex == toIndex) return;

    ItemStack& from = m_slots[fromIndex];
    ItemStack& to = m_slots[toIndex];

    if (from.isEmpty()) return;

    if (to.isEmpty()) {
        to = from;
        from = ItemStack();
    } else if (from.canStackWith(to)) {
        int space = to.getFreeSpace();
        int move = std::min(from.count, space);
        to.add(move);
        from.remove(move);
    }
}

// Return true if every slot is occupied.
bool Inventory::isFull() const {
    for (const auto& slot : m_slots) {
        if (slot.isEmpty()) return false;
    }
    return true;
}

// Return the index of the first empty slot, or -1 if none.
int Inventory::getFirstEmptySlot() const {
    for (int i = 0; i < TOTAL_SIZE; i++) {
        if (m_slots[i].isEmpty()) return i;
    }
    return -1;
}

// Return the index of the first slot containing the given type, or -1.
int Inventory::getFirstSlotWithItem(BlockType type) const {
    for (int i = 0; i < TOTAL_SIZE; i++) {
        if (m_slots[i].type == type) return i;
    }
    return -1;
}

// Clear all inventory slots.
void Inventory::clear() {
    m_slots.fill(ItemStack());
}
