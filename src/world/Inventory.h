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

    Inventory();

    ItemStack& getSlot(int index);
    const ItemStack& getSlot(int index) const;

    ItemStack& getHotbarSlot(int index);
    const ItemStack& getHotbarSlot(int index) const;

    ItemStack& getMainSlot(int row, int col);
    const ItemStack& getMainSlot(int row, int col) const;

    int getSelectedHotbarIndex() const { return m_selectedHotbar; }
    void setSelectedHotbarIndex(int index) { m_selectedHotbar = std::clamp(index, 0, HOTBAR_SIZE - 1); }
    void cycleHotbar(int delta);

    ItemStack& getSelectedItem() { return getHotbarSlot(m_selectedHotbar); }
    const ItemStack& getSelectedItem() const { return getHotbarSlot(m_selectedHotbar); }

    bool addItem(BlockType type, int count);
    bool addItem(const ItemStack& stack);
    ItemStack removeItem(int index, int count);
    ItemStack takeItem(int index);

    void swapSlots(int fromIndex, int toIndex);
    void splitStack(int fromIndex, int toIndex);
    void moveStack(int fromIndex, int toIndex);

    bool isFull() const;
    int getFirstEmptySlot() const;
    int getFirstSlotWithItem(BlockType type) const;

    void clear();

    DragState& getDragState() { return m_dragState; }
    const DragState& getDragState() const { return m_dragState; }

private:
    std::array<ItemStack, TOTAL_SIZE> m_slots;
    int m_selectedHotbar = 0;
    DragState m_dragState;
};
