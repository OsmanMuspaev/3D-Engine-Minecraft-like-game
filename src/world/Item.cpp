#include "Item.h"
#include "BlockRegistry.h"
#include <iostream>

const Item ItemRegistry::s_emptyItem = Item();

// Returns the singleton instance of ItemRegistry.
ItemRegistry& ItemRegistry::instance() {
    static ItemRegistry reg;
    return reg;
}

// Registers all block types and armor items as items with their textures.
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

    // Helper: use the block's PX face texture name from BlockRegistry
    auto addFromBlock = [&](BlockType type, const std::string& name, int maxStack = 64) {
        const auto& def = BlockRegistry::instance().getDef(type);
        std::string tex = def.textures[0];
        if (tex.empty()) tex = "stone";
        addItem(type, name, maxStack, tex);
    };

    addItem(BlockType::GRASS, "grass", 64, "grass_block_top");
    addItem(BlockType::DIRT, "dirt", 64, "dirt");
    addItem(BlockType::STONE, "stone", 64, "stone");
    addItem(BlockType::WOOD, "oak_log", 64, "oak_log_top");
    addItem(BlockType::LEAVES, "oak_leaves", 64, "oak_leaves");
    addItem(BlockType::SAND, "sand", 64, "sand");
    addItem(BlockType::SNOW, "snow", 64, "snow");
    addItem(BlockType::SNOW_BLOCK, "snow_block", 64, "snow");
    addItem(BlockType::BEDROCK, "bedrock", 64, "bedrock");
    addItem(BlockType::SANDSTONE, "sandstone", 64, "sandstone_top");

    // Earth / ground blocks
    addFromBlock(BlockType::COBBLESTONE, "cobblestone");
    addFromBlock(BlockType::GRAVEL, "gravel");
    addFromBlock(BlockType::CLAY, "clay");
    addFromBlock(BlockType::COARSE_DIRT, "coarse_dirt");
    addFromBlock(BlockType::PODZOL, "podzol");
    addFromBlock(BlockType::MYCELIUM, "mycelium");
    addFromBlock(BlockType::GRANITE, "granite");
    addFromBlock(BlockType::DIORITE, "diorite");
    addFromBlock(BlockType::ANDESITE, "andesite");
    addFromBlock(BlockType::MUD, "mud");
    addFromBlock(BlockType::PACKED_MUD, "packed_mud");
    addFromBlock(BlockType::MUD_BRICKS, "mud_bricks");
    addFromBlock(BlockType::DEEPSLATE, "deepslate");
    addFromBlock(BlockType::COBBLED_DEEPSLATE, "cobbled_deepslate");
    addFromBlock(BlockType::DEEPSLATE_BRICKS, "deepslate_bricks");
    addFromBlock(BlockType::DEEPSLATE_TILES, "deepslate_tiles");
    addFromBlock(BlockType::POLISHED_DEEPSLATE, "polished_deepslate");
    addFromBlock(BlockType::STONE_BRICKS, "stone_bricks");
    addFromBlock(BlockType::MOSSY_STONE_BRICKS, "mossy_stone_bricks");
    addFromBlock(BlockType::CRACKED_STONE_BRICKS, "cracked_stone_bricks");
    addFromBlock(BlockType::CHISELED_STONE_BRICKS, "chiseled_stone_bricks");
    addFromBlock(BlockType::MOSSY_COBBLESTONE, "mossy_cobblestone");
    addFromBlock(BlockType::SMOOTH_STONE, "smooth_stone");

    // Ores
    addFromBlock(BlockType::COAL_ORE, "coal_ore");
    addFromBlock(BlockType::IRON_ORE, "iron_ore");
    addFromBlock(BlockType::COPPER_ORE, "copper_ore");
    addFromBlock(BlockType::GOLD_ORE, "gold_ore");
    addFromBlock(BlockType::DIAMOND_ORE, "diamond_ore");
    addFromBlock(BlockType::EMERALD_ORE, "emerald_ore");
    addFromBlock(BlockType::LAPIS_ORE, "lapis_ore");
    addFromBlock(BlockType::REDSTONE_ORE, "redstone_ore");
    addFromBlock(BlockType::DEEPSLATE_COAL_ORE, "deepslate_coal_ore");
    addFromBlock(BlockType::DEEPSLATE_IRON_ORE, "deepslate_iron_ore");
    addFromBlock(BlockType::DEEPSLATE_COPPER_ORE, "deepslate_copper_ore");
    addFromBlock(BlockType::DEEPSLATE_GOLD_ORE, "deepslate_gold_ore");
    addFromBlock(BlockType::DEEPSLATE_DIAMOND_ORE, "deepslate_diamond_ore");
    addFromBlock(BlockType::DEEPSLATE_EMERALD_ORE, "deepslate_emerald_ore");
    addFromBlock(BlockType::DEEPSLATE_LAPIS_ORE, "deepslate_lapis_ore");
    addFromBlock(BlockType::DEEPSLATE_REDSTONE_ORE, "deepslate_redstone_ore");

    // Mineral blocks
    addFromBlock(BlockType::IRON_BLOCK, "iron_block");
    addFromBlock(BlockType::GOLD_BLOCK, "gold_block");
    addFromBlock(BlockType::DIAMOND_BLOCK, "diamond_block");
    addFromBlock(BlockType::EMERALD_BLOCK, "emerald_block");
    addFromBlock(BlockType::LAPIS_BLOCK, "lapis_block");
    addFromBlock(BlockType::REDSTONE_BLOCK, "redstone_block");
    addFromBlock(BlockType::COAL_BLOCK, "coal_block");
    addFromBlock(BlockType::NETHERITE_BLOCK, "netherite_block");
    addFromBlock(BlockType::COPPER_BLOCK, "copper_block");
    addFromBlock(BlockType::RAW_IRON_BLOCK, "raw_iron_block");
    addFromBlock(BlockType::RAW_GOLD_BLOCK, "raw_gold_block");
    addFromBlock(BlockType::RAW_COPPER_BLOCK, "raw_copper_block");

    // Planks
    addFromBlock(BlockType::OAK_PLANKS, "oak_planks");
    addFromBlock(BlockType::SPRUCE_PLANKS, "spruce_planks");
    addFromBlock(BlockType::BIRCH_PLANKS, "birch_planks");
    addFromBlock(BlockType::JUNGLE_PLANKS, "jungle_planks");
    addFromBlock(BlockType::ACACIA_PLANKS, "acacia_planks");
    addFromBlock(BlockType::DARK_OAK_PLANKS, "dark_oak_planks");
    addFromBlock(BlockType::MANGROVE_PLANKS, "mangrove_planks");
    addFromBlock(BlockType::CRIMSON_PLANKS, "crimson_planks");
    addFromBlock(BlockType::WARPED_PLANKS, "warped_planks");

    // Logs
    addFromBlock(BlockType::OAK_LOG, "oak_log");
    addFromBlock(BlockType::SPRUCE_LOG, "spruce_log");
    addFromBlock(BlockType::BIRCH_LOG, "birch_log");
    addFromBlock(BlockType::JUNGLE_LOG, "jungle_log");
    addFromBlock(BlockType::ACACIA_LOG, "acacia_log");
    addFromBlock(BlockType::DARK_OAK_LOG, "dark_oak_log");
    addFromBlock(BlockType::MANGROVE_LOG, "mangrove_log");
    addFromBlock(BlockType::CRIMSON_STEM, "crimson_stem");
    addFromBlock(BlockType::WARPED_STEM, "warped_stem");

    // Stripped logs
    addFromBlock(BlockType::STRIPPED_OAK_LOG, "stripped_oak_log");
    addFromBlock(BlockType::STRIPPED_SPRUCE_LOG, "stripped_spruce_log");
    addFromBlock(BlockType::STRIPPED_BIRCH_LOG, "stripped_birch_log");
    addFromBlock(BlockType::STRIPPED_JUNGLE_LOG, "stripped_jungle_log");
    addFromBlock(BlockType::STRIPPED_ACACIA_LOG, "stripped_acacia_log");
    addFromBlock(BlockType::STRIPPED_DARK_OAK_LOG, "stripped_dark_oak_log");
    addFromBlock(BlockType::STRIPPED_MANGROVE_LOG, "stripped_mangrove_log");

    // Leaves
    addFromBlock(BlockType::OAK_LEAVES, "oak_leaves");
    addFromBlock(BlockType::SPRUCE_LEAVES, "spruce_leaves");
    addFromBlock(BlockType::BIRCH_LEAVES, "birch_leaves");
    addFromBlock(BlockType::JUNGLE_LEAVES, "jungle_leaves");
    addFromBlock(BlockType::ACACIA_LEAVES, "acacia_leaves");
    addFromBlock(BlockType::DARK_OAK_LEAVES, "dark_oak_leaves");
    addFromBlock(BlockType::MANGROVE_LEAVES, "mangrove_leaves");
    addFromBlock(BlockType::AZALEA_LEAVES, "azalea_leaves");
    addFromBlock(BlockType::FLOWERING_AZALEA_LEAVES, "flowering_azalea_leaves");

    // Nether blocks
    addFromBlock(BlockType::NETHERRACK, "netherrack");
    addFromBlock(BlockType::NETHER_BRICKS, "nether_bricks");
    addFromBlock(BlockType::RED_NETHER_BRICKS, "red_nether_bricks");
    addFromBlock(BlockType::CRIMSON_NYLIUM, "crimson_nylium");
    addFromBlock(BlockType::WARPED_NYLIUM, "warped_nylium");
    addFromBlock(BlockType::SOUL_SAND, "soul_sand");
    addFromBlock(BlockType::SOUL_SOIL, "soul_soil");
    addFromBlock(BlockType::BLACKSTONE, "blackstone");
    addFromBlock(BlockType::BASALT, "basalt");
    addFromBlock(BlockType::POLISHED_BLACKSTONE, "polished_blackstone");
    addFromBlock(BlockType::POLISHED_BLACKSTONE_BRICKS, "polished_blackstone_bricks");
    addFromBlock(BlockType::POLISHED_BASALT, "polished_basalt");
    addFromBlock(BlockType::GLOWSTONE, "glowstone");
    addFromBlock(BlockType::NETHER_QUARTZ_ORE, "nether_quartz_ore");
    addFromBlock(BlockType::NETHER_GOLD_ORE, "nether_gold_ore");
    addFromBlock(BlockType::ANCIENT_DEBRIS, "ancient_debris");
    addFromBlock(BlockType::MAGMA, "magma");
    addFromBlock(BlockType::CRYING_OBSIDIAN, "crying_obsidian");

    // Building blocks
    addFromBlock(BlockType::BRICKS, "bricks");
    addFromBlock(BlockType::RED_SANDSTONE, "red_sandstone");
    addFromBlock(BlockType::QUARTZ_BLOCK, "quartz_block");
    addFromBlock(BlockType::QUARTZ_PILLAR, "quartz_pillar");
    addFromBlock(BlockType::QUARTZ_BRICKS, "quartz_bricks");
    addFromBlock(BlockType::PURPUR_BLOCK, "purpur_block");
    addFromBlock(BlockType::PURPUR_PILLAR, "purpur_pillar");
    addFromBlock(BlockType::PRISMARINE, "prismarine");
    addFromBlock(BlockType::DARK_PRISMARINE, "dark_prismarine");
    addFromBlock(BlockType::PRISMARINE_BRICKS, "prismarine_bricks");
    addFromBlock(BlockType::END_STONE, "end_stone");
    addFromBlock(BlockType::END_STONE_BRICKS, "end_stone_bricks");

    // Utility / redstone
    addFromBlock(BlockType::FURNACE, "furnace");
    addFromBlock(BlockType::DISPENSER, "dispenser");
    addFromBlock(BlockType::DROPPER, "dropper");
    addFromBlock(BlockType::OBSERVER, "observer");
    addFromBlock(BlockType::CRAFTING_TABLE, "crafting_table");
    addFromBlock(BlockType::BOOKSHELF, "bookshelf");
    addFromBlock(BlockType::NOTE_BLOCK, "note_block");
    addFromBlock(BlockType::JUKEBOX, "jukebox");
    addFromBlock(BlockType::TNT, "tnt");
    addFromBlock(BlockType::OBSIDIAN, "obsidian");
    addFromBlock(BlockType::BEACON, "beacon");
    addFromBlock(BlockType::CHEST, "chest");
    addFromBlock(BlockType::ENDER_CHEST, "ender_chest");
    addFromBlock(BlockType::ANVIL, "anvil");
    addFromBlock(BlockType::ENCHANTING_TABLE, "enchanting_table");
    addFromBlock(BlockType::REDSTONE_LAMP, "redstone_lamp");
    addFromBlock(BlockType::PISTON, "piston");
    addFromBlock(BlockType::STICKY_PISTON, "sticky_piston");

    // Glass
    addFromBlock(BlockType::GLASS, "glass");
    addFromBlock(BlockType::TINTED_GLASS, "tinted_glass");
    addFromBlock(BlockType::WHITE_STAINED_GLASS, "white_stained_glass");
    addFromBlock(BlockType::ORANGE_STAINED_GLASS, "orange_stained_glass");
    addFromBlock(BlockType::MAGENTA_STAINED_GLASS, "magenta_stained_glass");
    addFromBlock(BlockType::LIGHT_BLUE_STAINED_GLASS, "light_blue_stained_glass");
    addFromBlock(BlockType::YELLOW_STAINED_GLASS, "yellow_stained_glass");
    addFromBlock(BlockType::LIME_STAINED_GLASS, "lime_stained_glass");
    addFromBlock(BlockType::PINK_STAINED_GLASS, "pink_stained_glass");
    addFromBlock(BlockType::GRAY_STAINED_GLASS, "gray_stained_glass");
    addFromBlock(BlockType::LIGHT_GRAY_STAINED_GLASS, "light_gray_stained_glass");
    addFromBlock(BlockType::CYAN_STAINED_GLASS, "cyan_stained_glass");
    addFromBlock(BlockType::PURPLE_STAINED_GLASS, "purple_stained_glass");
    addFromBlock(BlockType::BLUE_STAINED_GLASS, "blue_stained_glass");
    addFromBlock(BlockType::BROWN_STAINED_GLASS, "brown_stained_glass");
    addFromBlock(BlockType::GREEN_STAINED_GLASS, "green_stained_glass");
    addFromBlock(BlockType::RED_STAINED_GLASS, "red_stained_glass");
    addFromBlock(BlockType::BLACK_STAINED_GLASS, "black_stained_glass");

    // Terracotta
    addFromBlock(BlockType::TERRACOTTA, "terracotta");
    addFromBlock(BlockType::WHITE_TERRACOTTA, "white_terracotta");
    addFromBlock(BlockType::ORANGE_TERRACOTTA, "orange_terracotta");
    addFromBlock(BlockType::MAGENTA_TERRACOTTA, "magenta_terracotta");
    addFromBlock(BlockType::LIGHT_BLUE_TERRACOTTA, "light_blue_terracotta");
    addFromBlock(BlockType::YELLOW_TERRACOTTA, "yellow_terracotta");
    addFromBlock(BlockType::LIME_TERRACOTTA, "lime_terracotta");
    addFromBlock(BlockType::PINK_TERRACOTTA, "pink_terracotta");
    addFromBlock(BlockType::GRAY_TERRACOTTA, "gray_terracotta");
    addFromBlock(BlockType::LIGHT_GRAY_TERRACOTTA, "light_gray_terracotta");
    addFromBlock(BlockType::CYAN_TERRACOTTA, "cyan_terracotta");
    addFromBlock(BlockType::PURPLE_TERRACOTTA, "purple_terracotta");
    addFromBlock(BlockType::BLUE_TERRACOTTA, "blue_terracotta");
    addFromBlock(BlockType::BROWN_TERRACOTTA, "brown_terracotta");
    addFromBlock(BlockType::GREEN_TERRACOTTA, "green_terracotta");
    addFromBlock(BlockType::RED_TERRACOTTA, "red_terracotta");
    addFromBlock(BlockType::BLACK_TERRACOTTA, "black_terracotta");

    // Wool
    addFromBlock(BlockType::WHITE_WOOL, "white_wool");
    addFromBlock(BlockType::ORANGE_WOOL, "orange_wool");
    addFromBlock(BlockType::MAGENTA_WOOL, "magenta_wool");
    addFromBlock(BlockType::LIGHT_BLUE_WOOL, "light_blue_wool");
    addFromBlock(BlockType::YELLOW_WOOL, "yellow_wool");
    addFromBlock(BlockType::LIME_WOOL, "lime_wool");
    addFromBlock(BlockType::PINK_WOOL, "pink_wool");
    addFromBlock(BlockType::GRAY_WOOL, "gray_wool");
    addFromBlock(BlockType::LIGHT_GRAY_WOOL, "light_gray_wool");
    addFromBlock(BlockType::CYAN_WOOL, "cyan_wool");
    addFromBlock(BlockType::PURPLE_WOOL, "purple_wool");
    addFromBlock(BlockType::BLUE_WOOL, "blue_wool");
    addFromBlock(BlockType::BROWN_WOOL, "brown_wool");
    addFromBlock(BlockType::GREEN_WOOL, "green_wool");
    addFromBlock(BlockType::RED_WOOL, "red_wool");
    addFromBlock(BlockType::BLACK_WOOL, "black_wool");

    // Concrete
    addFromBlock(BlockType::WHITE_CONCRETE, "white_concrete");
    addFromBlock(BlockType::ORANGE_CONCRETE, "orange_concrete");
    addFromBlock(BlockType::MAGENTA_CONCRETE, "magenta_concrete");
    addFromBlock(BlockType::LIGHT_BLUE_CONCRETE, "light_blue_concrete");
    addFromBlock(BlockType::YELLOW_CONCRETE, "yellow_concrete");
    addFromBlock(BlockType::LIME_CONCRETE, "lime_concrete");
    addFromBlock(BlockType::PINK_CONCRETE, "pink_concrete");
    addFromBlock(BlockType::GRAY_CONCRETE, "gray_concrete");
    addFromBlock(BlockType::LIGHT_GRAY_CONCRETE, "light_gray_concrete");
    addFromBlock(BlockType::CYAN_CONCRETE, "cyan_concrete");
    addFromBlock(BlockType::PURPLE_CONCRETE, "purple_concrete");
    addFromBlock(BlockType::BLUE_CONCRETE, "blue_concrete");
    addFromBlock(BlockType::BROWN_CONCRETE, "brown_concrete");
    addFromBlock(BlockType::GREEN_CONCRETE, "green_concrete");
    addFromBlock(BlockType::RED_CONCRETE, "red_concrete");
    addFromBlock(BlockType::BLACK_CONCRETE, "black_concrete");

    // Ice
    addFromBlock(BlockType::ICE, "ice");
    addFromBlock(BlockType::PACKED_ICE, "packed_ice");
    addFromBlock(BlockType::BLUE_ICE, "blue_ice");

    // Misc blocks
    addFromBlock(BlockType::SPONGE, "sponge");
    addFromBlock(BlockType::WET_SPONGE, "wet_sponge");
    addFromBlock(BlockType::HAY_BLOCK, "hay_block");
    addFromBlock(BlockType::MELON, "melon");
    addFromBlock(BlockType::PUMPKIN, "pumpkin");
    addFromBlock(BlockType::CARVED_PUMPKIN, "carved_pumpkin");
    addFromBlock(BlockType::CACTUS, "cactus");
    addFromBlock(BlockType::GRASS_BLOCK, "grass_block");
    addFromBlock(BlockType::HONEY_BLOCK, "honey_block");
    addFromBlock(BlockType::HONEYCOMB_BLOCK, "honeycomb_block");
    addFromBlock(BlockType::SLIME_BLOCK, "slime_block");
    addFromBlock(BlockType::MOSS_BLOCK, "moss_block");
    addFromBlock(BlockType::SEA_LANTERN, "sea_lantern");
    addFromBlock(BlockType::CALCITE, "calcite");
    addFromBlock(BlockType::TUFF, "tuff");
    addFromBlock(BlockType::DRIPSTONE_BLOCK, "dripstone_block");
    addFromBlock(BlockType::AMETHYST_BLOCK, "amethyst_block");
    addFromBlock(BlockType::BUDDING_AMETHYST, "budding_amethyst");

    // Armor items (stack size 1)
    auto addArmor = [&](BlockType type, const std::string& name, const std::string& texture) {
        addItem(type, name, 1, texture);
    };

    addArmor(BlockType::LEATHER_HELMET, "leather_helmet", "leather_helmet");
    addArmor(BlockType::LEATHER_CHESTPLATE, "leather_chestplate", "leather_chestplate");
    addArmor(BlockType::LEATHER_LEGGINGS, "leather_leggings", "leather_leggings");
    addArmor(BlockType::LEATHER_BOOTS, "leather_boots", "leather_boots");
    addArmor(BlockType::CHAINMAIL_HELMET, "chainmail_helmet", "chainmail_helmet");
    addArmor(BlockType::CHAINMAIL_CHESTPLATE, "chainmail_chestplate", "chainmail_chestplate");
    addArmor(BlockType::CHAINMAIL_LEGGINGS, "chainmail_leggings", "chainmail_leggings");
    addArmor(BlockType::CHAINMAIL_BOOTS, "chainmail_boots", "chainmail_boots");
    addArmor(BlockType::IRON_HELMET, "iron_helmet", "iron_helmet");
    addArmor(BlockType::IRON_CHESTPLATE, "iron_chestplate", "iron_chestplate");
    addArmor(BlockType::IRON_LEGGINGS, "iron_leggings", "iron_leggings");
    addArmor(BlockType::IRON_BOOTS, "iron_boots", "iron_boots");
    addArmor(BlockType::GOLDEN_HELMET, "golden_helmet", "golden_helmet");
    addArmor(BlockType::GOLDEN_CHESTPLATE, "golden_chestplate", "golden_chestplate");
    addArmor(BlockType::GOLDEN_LEGGINGS, "golden_leggings", "golden_leggings");
    addArmor(BlockType::GOLDEN_BOOTS, "golden_boots", "golden_boots");
    addArmor(BlockType::DIAMOND_HELMET, "diamond_helmet", "diamond_helmet");
    addArmor(BlockType::DIAMOND_CHESTPLATE, "diamond_chestplate", "diamond_chestplate");
    addArmor(BlockType::DIAMOND_LEGGINGS, "diamond_leggings", "diamond_leggings");
    addArmor(BlockType::DIAMOND_BOOTS, "diamond_boots", "diamond_boots");
    addArmor(BlockType::NETHERITE_HELMET, "netherite_helmet", "netherite_helmet");
    addArmor(BlockType::NETHERITE_CHESTPLATE, "netherite_chestplate", "netherite_chestplate");
    addArmor(BlockType::NETHERITE_LEGGINGS, "netherite_leggings", "netherite_leggings");
    addArmor(BlockType::NETHERITE_BOOTS, "netherite_boots", "netherite_boots");

    std::cout << "ItemRegistry: initialized " << m_items.size() << " items\n";
}

// Looks up an item by its BlockType, returns empty item if not found.
const Item& ItemRegistry::getItem(BlockType type) const {
    auto it = m_items.find(type);
    if (it != m_items.end())
        return it->second;

    return s_emptyItem;
}

// Looks up an item by its string name, returns empty item if not found.
const Item& ItemRegistry::getItemByName(const std::string& name) const {
    auto it = m_itemsByName.find(name);
    if (it != m_itemsByName.end())
        return it->second;

    return s_emptyItem;
}

// Returns the max stack size for a given BlockType, default 64.
int ItemRegistry::getMaxStackSize(BlockType type) const {
    auto it = m_items.find(type);
    if (it != m_items.end())
        return it->second.maxStackSize;

    return 64;
}
