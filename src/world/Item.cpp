#include "Item.h"
#include "BlockRegistry.h"
#include <iostream>

const Item ItemRegistry::s_emptyItem = Item();

ItemRegistry& ItemRegistry::instance() {
    static ItemRegistry reg;
    return reg;
}

void ItemRegistry::init() {
    m_items.clear();
    m_itemsByName.clear();

    auto addItem = [&](BlockType type,
                        const std::string& name,
                        int maxStack,
                        const std::string& texture)
    {
        Item item(name, type, maxStack, texture);
        m_items[type] = item;
        m_itemsByName[name] = item;
    };

    // ===== BLOCK ITEMS (ВАЖНО: textureName должен существовать в TextureManager) =====

    addItem(BlockType::GRASS, "grass", 64, "grass_block_top");
    addItem(BlockType::DIRT, "dirt", 64, "dirt");
    addItem(BlockType::STONE, "stone", 64, "stone");

    // лог — обычно используется side/top variant
    addItem(BlockType::WOOD, "oak_log", 64, "oak_log_top");

    addItem(BlockType::LEAVES, "oak_leaves", 64, "oak_leaves");

    addItem(BlockType::SAND, "sand", 64, "sand");

    // ❗ FIX: snow НЕ должен быть просто "snow"
    // В атласе обычно это snow_block
    addItem(BlockType::SNOW, "snow", 64, "snow");
    addItem(BlockType::SNOW_BLOCK, "snow_block", 64, "snow");

    addItem(BlockType::BEDROCK, "bedrock", 64, "bedrock");

    addItem(BlockType::SANDSTONE, "sandstone", 64, "sandstone_top");

    std::cout << "ItemRegistry: initialized " << m_items.size() << " items\n";

    // (опционально) debug проверка — убирает silent bugs
    for (const auto& [type, item] : m_items) {
        std::cout << "[Item] " << item.name
                  << " -> texture: " << item.textureName << "\n";
    }
}

const Item& ItemRegistry::getItem(BlockType type) const {
    auto it = m_items.find(type);
    if (it != m_items.end())
        return it->second;

    return s_emptyItem;
}

const Item& ItemRegistry::getItemByName(const std::string& name) const {
    auto it = m_itemsByName.find(name);
    if (it != m_itemsByName.end())
        return it->second;

    return s_emptyItem;
}

int ItemRegistry::getMaxStackSize(BlockType type) const {
    auto it = m_items.find(type);
    if (it != m_items.end())
        return it->second.maxStackSize;

    return 64;
}