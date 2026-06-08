#pragma once
#include "Block.h"
#include <string>
#include <unordered_map>

// Represents an item type with name, associated block, stack size, and texture.
struct Item {
    std::string name;
    BlockType blockType = BlockType::AIR;
    int maxStackSize = 64;
    std::string textureName;

    Item() = default;
    Item(const std::string& n, BlockType bt, int maxStack, const std::string& tex)
        : name(n), blockType(bt), maxStackSize(maxStack), textureName(tex) {}
};

// Singleton registry for all item types, indexed by BlockType and name.
class ItemRegistry {
public:
    static ItemRegistry& instance();

    void init();
    const Item& getItem(BlockType type) const;
    const Item& getItemByName(const std::string& name) const;
    int getMaxStackSize(BlockType type) const;

private:
    ItemRegistry() = default;
    std::unordered_map<BlockType, Item> m_items;
    std::unordered_map<std::string, Item> m_itemsByName;
    static const Item s_emptyItem;
};

// A stack of items of the same type with count tracking.
struct ItemStack {
    BlockType type = BlockType::AIR;
    int count = 0;

    ItemStack() = default;
    ItemStack(BlockType t, int c) : type(t), count(c) {}

    bool isEmpty() const { return type == BlockType::AIR || count <= 0; }
    int getMaxStackSize() const { return ItemRegistry::instance().getMaxStackSize(type); }
    int getFreeSpace() const { return getMaxStackSize() - count; }

    bool canStackWith(const ItemStack& other) const {
        return type == other.type && !isEmpty() && !other.isEmpty();
    }

    void add(int amount) { count = std::min(count + amount, getMaxStackSize()); }
    void remove(int amount) { count = std::max(0, count - amount); if (count == 0) type = BlockType::AIR; }
};
