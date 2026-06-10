#pragma once
#include "Block.h"
#include <string>
#include <unordered_map>

struct Item {
    std::string name;
    BlockType blockType = BlockType::AIR;
    int maxStackSize = 64;
    std::string textureName;

    Item() = default;
    Item(const std::string& n, BlockType bt, int maxStack, const std::string& tex)
        : name(n), blockType(bt), maxStackSize(maxStack), textureName(tex) {}
};

class ItemRegistry {
public:
    // Returns the singleton instance of ItemRegistry.
    static ItemRegistry& instance();

    // Registers all block types and armor items as items with their textures.
    void init();
    // Looks up an item by its BlockType, returns empty item if not found.
    const Item& getItem(BlockType type) const;
    // Looks up an item by its string name, returns empty item if not found.
    const Item& getItemByName(const std::string& name) const;
    // Returns the max stack size for a given BlockType, default 64.
    int getMaxStackSize(BlockType type) const;

private:
    ItemRegistry() = default;
    std::unordered_map<BlockType, Item> m_items;
    std::unordered_map<std::string, Item> m_itemsByName;
    static const Item s_emptyItem;
};

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
