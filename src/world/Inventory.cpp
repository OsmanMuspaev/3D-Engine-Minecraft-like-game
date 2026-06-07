#include "Inventory.h"
#include <algorithm>

Inventory::Inventory() {
    m_slots.fill(ItemStack());
}

ItemStack& Inventory::getSlot(int index) {
    index = std::clamp(index, 0, TOTAL_SIZE - 1);
    return m_slots[index];
}

const ItemStack& Inventory::getSlot(int index) const {
    index = std::clamp(index, 0, TOTAL_SIZE - 1);
    return m_slots[index];
}

ItemStack& Inventory::getHotbarSlot(int index) {
    index = std::clamp(index, 0, HOTBAR_SIZE - 1);
    return m_slots[index];
}

const ItemStack& Inventory::getHotbarSlot(int index) const {
    index = std::clamp(index, 0, HOTBAR_SIZE - 1);
    return m_slots[index];
}

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

void Inventory::cycleHotbar(int delta) {
    m_selectedHotbar = (m_selectedHotbar + delta) % HOTBAR_SIZE;
    if (m_selectedHotbar < 0) m_selectedHotbar += HOTBAR_SIZE;
}

bool Inventory::addItem(BlockType type, int count) {
    return addItem(ItemStack(type, count));
}

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

ItemStack Inventory::removeItem(int index, int count) {
    ItemStack& slot = getSlot(index);
    if (slot.isEmpty()) return ItemStack();

    int toRemove = std::min(count, slot.count);
    ItemStack result(slot.type, toRemove);
    slot.remove(toRemove);
    return result;
}

ItemStack Inventory::takeItem(int index) {
    ItemStack& slot = getSlot(index);
    ItemStack result = slot;
    slot = ItemStack();
    return result;
}

void Inventory::swapSlots(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= TOTAL_SIZE || toIndex < 0 || toIndex >= TOTAL_SIZE) return;
    if (fromIndex == toIndex) return;
    std::swap(m_slots[fromIndex], m_slots[toIndex]);
}

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

bool Inventory::isFull() const {
    for (const auto& slot : m_slots) {
        if (slot.isEmpty()) return false;
    }
    return true;
}

int Inventory::getFirstEmptySlot() const {
    for (int i = 0; i < TOTAL_SIZE; i++) {
        if (m_slots[i].isEmpty()) return i;
    }
    return -1;
}

int Inventory::getFirstSlotWithItem(BlockType type) const {
    for (int i = 0; i < TOTAL_SIZE; i++) {
        if (m_slots[i].type == type) return i;
    }
    return -1;
}

void Inventory::clear() {
    m_slots.fill(ItemStack());
}
