#pragma once
#include "Item.h"
#include <array>
#include <vector>
#include <algorithm>

class Inventory {
public:
    struct DragState {
        bool active = false;
        ItemStack stack;
        int sourceIndex = -1;
    };

    static constexpr int HOTBAR_SIZE = 9;
    static constexpr int MAIN_ROWS = 3;
    static constexpr int MAIN_COLS = 9;
    static constexpr int MAIN_SIZE = MAIN_ROWS * MAIN_COLS;
    static constexpr int TOTAL_SIZE = HOTBAR_SIZE + MAIN_SIZE;
    static constexpr int ARMOR_SLOTS = 4;

    static constexpr int ARMOR_HELMET = 0;
    static constexpr int ARMOR_CHESTPLATE = 1;
    static constexpr int ARMOR_LEGGINGS = 2;
    static constexpr int ARMOR_BOOTS = 3;

    Inventory();

    ItemStack& getSlot(int index);
    const ItemStack& getSlot(int index) const;

    ItemStack& getHotbarSlot(int index);
    const ItemStack& getHotbarSlot(int index) const;

    ItemStack& getMainSlot(int row, int col);
    const ItemStack& getMainSlot(int row, int col) const;

    ItemStack& getArmorSlot(int index);
    const ItemStack& getArmorSlot(int index) const;

    int getSelectedHotbarIndex() const { return m_selectedHotbar; }
    void setSelectedHotbarIndex(int index) { m_selectedHotbar = std::clamp(index, 0, HOTBAR_SIZE - 1); }
    // Wraps the hotbar selection index around valid range.
    void cycleHotbar(int delta);

    ItemStack& getSelectedItem() { return getHotbarSlot(m_selectedHotbar); }
    const ItemStack& getSelectedItem() const { return getHotbarSlot(m_selectedHotbar); }

    // Attempts to add items by type; delegates to the stack overload.
    bool addItem(BlockType type, int count);
    // Merges items into existing stacks first, then fills empty slots.
    bool addItem(const ItemStack& stack);
    // Removes a specific count of items from a slot and returns them.
    ItemStack removeItem(int index, int count);
    // Takes the entire contents of a slot, clearing it.
    ItemStack takeItem(int index);

    // Swaps the contents of two slots.
    void swapSlots(int fromIndex, int toIndex);
    // Splits a stack by moving half into the target slot.
    void splitStack(int fromIndex, int toIndex);
    // Moves or merges a stack into another slot.
    void moveStack(int fromIndex, int toIndex);

    // Returns true if every slot is occupied.
    bool isFull() const;
    // Returns the index of the first empty slot, or -1 if none.
    int getFirstEmptySlot() const;
    // Returns the index of the first slot containing the given type, or -1.
    int getFirstSlotWithItem(BlockType type) const;

    // Clears all inventory slots.
    void clear();

    DragState& getDragState() { return m_dragState; }
    const DragState& getDragState() const { return m_dragState; }

    // Checks whether a block type is any piece of armor.
    static bool isArmor(BlockType type);

private:
    std::array<ItemStack, TOTAL_SIZE> m_slots;
    std::array<ItemStack, ARMOR_SLOTS> m_armorSlots;
    int m_selectedHotbar = 0;
    DragState m_dragState;
};
