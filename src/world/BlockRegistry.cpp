#include "BlockRegistry.h"
#include <iostream>

static const std::array<int, 6> NO_TINT = {TINT_NONE, TINT_NONE, TINT_NONE, TINT_NONE, TINT_NONE, TINT_NONE};
static const std::array<int, 6> ALL_GRASS = {TINT_GRASS, TINT_GRASS, TINT_GRASS, TINT_GRASS, TINT_GRASS, TINT_GRASS};
static const std::array<int, 6> ALL_FOLIAGE = {TINT_FOLIAGE, TINT_FOLIAGE, TINT_FOLIAGE, TINT_FOLIAGE, TINT_FOLIAGE, TINT_FOLIAGE};
static const std::array<int, 6> ALL_WATER = {TINT_WATER, TINT_WATER, TINT_WATER, TINT_WATER, TINT_WATER, TINT_WATER};
static const std::array<int, 6> TOP_GRASS_TINT = {TINT_NONE, TINT_NONE, TINT_GRASS, TINT_NONE, TINT_NONE, TINT_NONE};
static const std::array<int, 6> SIDE_GRASS_TINT = {TINT_GRASS, TINT_GRASS, TINT_NONE, TINT_NONE, TINT_GRASS, TINT_GRASS};

// Returns the singleton block registry instance
BlockRegistry& BlockRegistry::instance() {
    static BlockRegistry reg;
    return reg;
}

// Creates a BlockDef with the same texture on all six faces
static BlockDef makeDef(const std::string& name, const std::string& allFaces,
                        bool solid, bool transparent, int tint = TINT_NONE) {
    BlockDef d;
    d.name = name;
    d.textures = {allFaces, allFaces, allFaces, allFaces, allFaces, allFaces};
    d.tintIndex = {tint, tint, tint, tint, tint, tint};
    d.solid = solid;
    d.transparent = transparent;
    return d;
}

// Creates a BlockDef with per-face textures and per-face tint indices
static BlockDef makeDefFaces(const std::string& name,
                             const std::string& px, const std::string& nx,
                             const std::string& py, const std::string& ny,
                             const std::string& pz, const std::string& nz,
                             bool solid, bool transparent,
                             const std::array<int, 6>& tints = NO_TINT) {
    BlockDef d;
    d.name = name;
    d.textures = {px, nx, py, ny, pz, nz};
    d.tintIndex = tints;
    d.solid = solid;
    d.transparent = transparent;
    return d;
}

// Initializes all block definitions
void BlockRegistry::init() {
    m_defs.clear();

    // Helper: uniform texture block
    auto uni = [&](BlockType type, const std::string& name, const std::string& tex,
                   bool solid = true, bool transparent = false, int tint = TINT_NONE) {
        m_defs[type] = makeDef(name, tex, solid, transparent, tint);
    };

    // Helper: log block with side/top textures
    auto log = [&](BlockType type, const std::string& name,
                   const std::string& sideTex, const std::string& topTex) {
        m_defs[type] = makeDefFaces(name, sideTex, sideTex, topTex, topTex, sideTex, sideTex, true, false);
    };

    // Helper: plank block (uniform texture)
    auto plank = [&](BlockType type, const std::string& name, const std::string& tex) {
        uni(type, name, tex);
    };

    // Original block types
    m_defs[BlockType::GRASS] = makeDefFaces("grass",
        "grass_block_side", "grass_block_side",
        "grass_block_top", "dirt",
        "grass_block_side", "grass_block_side",
        true, false, TOP_GRASS_TINT);

    m_defs[BlockType::DIRT] = makeDef("dirt", "dirt", true, false);
    m_defs[BlockType::STONE] = makeDef("stone", "stone", true, false);

    m_defs[BlockType::WOOD] = makeDefFaces("oak_log",
        "oak_log", "oak_log", "oak_log_top", "oak_log_top", "oak_log", "oak_log",
        true, false);

    m_defs[BlockType::LEAVES] = makeDef("oak_leaves", "oak_leaves", true, true, TINT_FOLIAGE);

    m_defs[BlockType::AIR] = makeDef("air", "", false, true);
    m_defs[BlockType::SAND] = makeDef("sand", "sand", true, false);

    m_defs[BlockType::SNOW] = makeDefFaces("snow",
        "snow", "snow", "snow", "dirt", "snow", "snow",
        true, false);

    m_defs[BlockType::BEDROCK] = makeDef("bedrock", "bedrock", true, false);
    m_defs[BlockType::WATER] = makeDef("water", "water_still", false, true, TINT_WATER);

    m_defs[BlockType::SANDSTONE] = makeDefFaces("sandstone",
        "sandstone", "sandstone", "sandstone_top", "sandstone_bottom", "sandstone", "sandstone",
        true, false);

    m_defs[BlockType::SNOW_BLOCK] = makeDef("snow_block", "snow", true, false);

    // Earth / ground blocks
    uni(BlockType::COBBLESTONE, "cobblestone", "cobblestone");
    uni(BlockType::GRAVEL, "gravel", "gravel");
    uni(BlockType::CLAY, "clay", "clay");
    uni(BlockType::COARSE_DIRT, "coarse_dirt", "coarse_dirt");

    m_defs[BlockType::PODZOL] = makeDefFaces("podzol",
        "podzol_side", "podzol_side", "podzol_top", "dirt", "podzol_side", "podzol_side",
        true, false);

    m_defs[BlockType::MYCELIUM] = makeDefFaces("mycelium",
        "mycelium_side", "mycelium_side", "mycelium_top", "dirt", "mycelium_side", "mycelium_side",
        true, false);

    uni(BlockType::GRANITE, "granite", "granite");
    uni(BlockType::DIORITE, "diorite", "diorite");
    uni(BlockType::ANDESITE, "andesite", "andesite");
    uni(BlockType::MUD, "mud", "mud");
    uni(BlockType::PACKED_MUD, "packed_mud", "packed_mud");
    uni(BlockType::MUD_BRICKS, "mud_bricks", "mud_bricks");
    uni(BlockType::DEEPSLATE, "deepslate", "deepslate");
    uni(BlockType::COBBLED_DEEPSLATE, "cobbled_deepslate", "cobbled_deepslate");
    uni(BlockType::DEEPSLATE_BRICKS, "deepslate_bricks", "deepslate_bricks");
    uni(BlockType::DEEPSLATE_TILES, "deepslate_tiles", "deepslate_tiles");
    uni(BlockType::POLISHED_DEEPSLATE, "polished_deepslate", "polished_deepslate");
    uni(BlockType::STONE_BRICKS, "stone_bricks", "stone_bricks");
    uni(BlockType::MOSSY_STONE_BRICKS, "mossy_stone_bricks", "mossy_stone_bricks");
    uni(BlockType::CRACKED_STONE_BRICKS, "cracked_stone_bricks", "cracked_stone_bricks");
    uni(BlockType::CHISELED_STONE_BRICKS, "chiseled_stone_bricks", "chiseled_stone_bricks");
    uni(BlockType::MOSSY_COBBLESTONE, "mossy_cobblestone", "mossy_cobblestone");
    uni(BlockType::SMOOTH_STONE, "smooth_stone", "smooth_stone");

    // Ores
    uni(BlockType::COAL_ORE, "coal_ore", "coal_ore");
    uni(BlockType::IRON_ORE, "iron_ore", "iron_ore");
    uni(BlockType::COPPER_ORE, "copper_ore", "copper_ore");
    uni(BlockType::GOLD_ORE, "gold_ore", "gold_ore");
    uni(BlockType::DIAMOND_ORE, "diamond_ore", "diamond_ore");
    uni(BlockType::EMERALD_ORE, "emerald_ore", "emerald_ore");
    uni(BlockType::LAPIS_ORE, "lapis_ore", "lapis_ore");
    uni(BlockType::REDSTONE_ORE, "redstone_ore", "redstone_ore");
    uni(BlockType::DEEPSLATE_COAL_ORE, "deepslate_coal_ore", "deepslate_coal_ore");
    uni(BlockType::DEEPSLATE_IRON_ORE, "deepslate_iron_ore", "deepslate_iron_ore");
    uni(BlockType::DEEPSLATE_COPPER_ORE, "deepslate_copper_ore", "deepslate_copper_ore");
    uni(BlockType::DEEPSLATE_GOLD_ORE, "deepslate_gold_ore", "deepslate_gold_ore");
    uni(BlockType::DEEPSLATE_DIAMOND_ORE, "deepslate_diamond_ore", "deepslate_diamond_ore");
    uni(BlockType::DEEPSLATE_EMERALD_ORE, "deepslate_emerald_ore", "deepslate_emerald_ore");
    uni(BlockType::DEEPSLATE_LAPIS_ORE, "deepslate_lapis_ore", "deepslate_lapis_ore");
    uni(BlockType::DEEPSLATE_REDSTONE_ORE, "deepslate_redstone_ore", "deepslate_redstone_ore");

    // Mineral blocks
    uni(BlockType::IRON_BLOCK, "iron_block", "iron_block");
    uni(BlockType::GOLD_BLOCK, "gold_block", "gold_block");
    uni(BlockType::DIAMOND_BLOCK, "diamond_block", "diamond_block");
    uni(BlockType::EMERALD_BLOCK, "emerald_block", "emerald_block");
    uni(BlockType::LAPIS_BLOCK, "lapis_block", "lapis_block");
    uni(BlockType::REDSTONE_BLOCK, "redstone_block", "redstone_block");
    uni(BlockType::COAL_BLOCK, "coal_block", "coal_block");
    uni(BlockType::NETHERITE_BLOCK, "netherite_block", "netherite_block");
    uni(BlockType::COPPER_BLOCK, "copper_block", "copper_block");
    uni(BlockType::RAW_IRON_BLOCK, "raw_iron_block", "raw_iron_block");
    uni(BlockType::RAW_GOLD_BLOCK, "raw_gold_block", "raw_gold_block");
    uni(BlockType::RAW_COPPER_BLOCK, "raw_copper_block", "raw_copper_block");

    // Planks
    plank(BlockType::OAK_PLANKS, "oak_planks", "oak_planks");
    plank(BlockType::SPRUCE_PLANKS, "spruce_planks", "spruce_planks");
    plank(BlockType::BIRCH_PLANKS, "birch_planks", "birch_planks");
    plank(BlockType::JUNGLE_PLANKS, "jungle_planks", "jungle_planks");
    plank(BlockType::ACACIA_PLANKS, "acacia_planks", "acacia_planks");
    plank(BlockType::DARK_OAK_PLANKS, "dark_oak_planks", "dark_oak_planks");
    plank(BlockType::MANGROVE_PLANKS, "mangrove_planks", "mangrove_planks");
    plank(BlockType::CRIMSON_PLANKS, "crimson_planks", "crimson_planks");
    plank(BlockType::WARPED_PLANKS, "warped_planks", "warped_planks");

    // Logs
    log(BlockType::OAK_LOG, "oak_log", "oak_log", "oak_log_top");
    log(BlockType::SPRUCE_LOG, "spruce_log", "spruce_log", "spruce_log_top");
    log(BlockType::BIRCH_LOG, "birch_log", "birch_log", "birch_log_top");
    log(BlockType::JUNGLE_LOG, "jungle_log", "jungle_log", "jungle_log_top");
    log(BlockType::ACACIA_LOG, "acacia_log", "acacia_log", "acacia_log_top");
    log(BlockType::DARK_OAK_LOG, "dark_oak_log", "dark_oak_log", "dark_oak_log_top");
    log(BlockType::MANGROVE_LOG, "mangrove_log", "mangrove_log", "mangrove_log_top");
    log(BlockType::CRIMSON_STEM, "crimson_stem", "crimson_stem", "crimson_stem_top");
    log(BlockType::WARPED_STEM, "warped_stem", "warped_stem", "warped_stem_top");

    // Stripped logs
    log(BlockType::STRIPPED_OAK_LOG, "stripped_oak_log", "stripped_oak_log", "stripped_oak_log_top");
    log(BlockType::STRIPPED_SPRUCE_LOG, "stripped_spruce_log", "stripped_spruce_log", "stripped_spruce_log_top");
    log(BlockType::STRIPPED_BIRCH_LOG, "stripped_birch_log", "stripped_birch_log", "stripped_birch_log_top");
    log(BlockType::STRIPPED_JUNGLE_LOG, "stripped_jungle_log", "stripped_jungle_log", "stripped_jungle_log_top");
    log(BlockType::STRIPPED_ACACIA_LOG, "stripped_acacia_log", "stripped_acacia_log", "stripped_acacia_log_top");
    log(BlockType::STRIPPED_DARK_OAK_LOG, "stripped_dark_oak_log", "stripped_dark_oak_log", "stripped_dark_oak_log_top");
    log(BlockType::STRIPPED_MANGROVE_LOG, "stripped_mangrove_log", "stripped_mangrove_log", "stripped_mangrove_log_top");

    // Leaves
    auto leaf = [&](BlockType type, const std::string& name, const std::string& tex) {
        m_defs[type] = makeDef(name, tex, true, true, TINT_FOLIAGE);
    };
    leaf(BlockType::OAK_LEAVES, "oak_leaves", "oak_leaves");
    leaf(BlockType::SPRUCE_LEAVES, "spruce_leaves", "spruce_leaves");
    leaf(BlockType::BIRCH_LEAVES, "birch_leaves", "birch_leaves");
    leaf(BlockType::JUNGLE_LEAVES, "jungle_leaves", "jungle_leaves");
    leaf(BlockType::ACACIA_LEAVES, "acacia_leaves", "acacia_leaves");
    leaf(BlockType::DARK_OAK_LEAVES, "dark_oak_leaves", "dark_oak_leaves");
    leaf(BlockType::MANGROVE_LEAVES, "mangrove_leaves", "mangrove_leaves");
    leaf(BlockType::AZALEA_LEAVES, "azalea_leaves", "azalea_leaves");
    leaf(BlockType::FLOWERING_AZALEA_LEAVES, "flowering_azalea_leaves", "flowering_azalea_leaves");

    // Nether blocks
    uni(BlockType::NETHERRACK, "netherrack", "netherrack");
    uni(BlockType::NETHER_BRICKS, "nether_bricks", "nether_bricks");
    uni(BlockType::RED_NETHER_BRICKS, "red_nether_bricks", "red_nether_bricks");

    m_defs[BlockType::CRIMSON_NYLIUM] = makeDefFaces("crimson_nylium",
        "crimson_nylium_side", "crimson_nylium_side", "crimson_nylium", "netherrack",
        "crimson_nylium_side", "crimson_nylium_side", true, false);

    m_defs[BlockType::WARPED_NYLIUM] = makeDefFaces("warped_nylium",
        "warped_nylium_side", "warped_nylium_side", "warped_nylium", "netherrack",
        "warped_nylium_side", "warped_nylium_side", true, false);

    uni(BlockType::SOUL_SAND, "soul_sand", "soul_sand");
    uni(BlockType::SOUL_SOIL, "soul_soil", "soul_soil");
    uni(BlockType::BLACKSTONE, "blackstone", "blackstone");

    log(BlockType::BASALT, "basalt", "basalt_side", "basalt_top");

    uni(BlockType::POLISHED_BLACKSTONE, "polished_blackstone", "polished_blackstone");
    uni(BlockType::POLISHED_BLACKSTONE_BRICKS, "polished_blackstone_bricks", "polished_blackstone_bricks");

    m_defs[BlockType::POLISHED_BASALT] = makeDefFaces("polished_basalt",
        "polished_basalt_side", "polished_basalt_side",
        "polished_basalt_top", "polished_basalt_top",
        "polished_basalt_side", "polished_basalt_side", true, false);

    uni(BlockType::GLOWSTONE, "glowstone", "glowstone");
    uni(BlockType::NETHER_QUARTZ_ORE, "nether_quartz_ore", "nether_quartz_ore");
    uni(BlockType::NETHER_GOLD_ORE, "nether_gold_ore", "nether_gold_ore");
    uni(BlockType::ANCIENT_DEBRIS, "ancient_debris_side", "ancient_debris_side");
    uni(BlockType::MAGMA, "magma", "magma");
    uni(BlockType::CRYING_OBSIDIAN, "crying_obsidian", "crying_obsidian");

    // Building blocks
    uni(BlockType::BRICKS, "bricks", "bricks");

    m_defs[BlockType::RED_SANDSTONE] = makeDefFaces("red_sandstone",
        "red_sandstone", "red_sandstone", "red_sandstone_top", "red_sandstone_bottom",
        "red_sandstone", "red_sandstone", true, false);

    uni(BlockType::QUARTZ_BLOCK, "quartz_block", "quartz_block_top");
    log(BlockType::QUARTZ_PILLAR, "quartz_pillar", "quartz_pillar", "quartz_pillar_top");
    uni(BlockType::QUARTZ_BRICKS, "quartz_bricks", "quartz_bricks");
    uni(BlockType::PURPUR_BLOCK, "purpur_block", "purpur_block");
    log(BlockType::PURPUR_PILLAR, "purpur_pillar", "purpur_pillar", "purpur_pillar_top");
    uni(BlockType::PRISMARINE, "prismarine", "prismarine");
    uni(BlockType::DARK_PRISMARINE, "dark_prismarine", "dark_prismarine");
    uni(BlockType::PRISMARINE_BRICKS, "prismarine_bricks", "prismarine_bricks");
    uni(BlockType::END_STONE, "end_stone", "end_stone");
    uni(BlockType::END_STONE_BRICKS, "end_stone_bricks", "end_stone_bricks");

    // Utility / redstone blocks
    m_defs[BlockType::FURNACE] = makeDefFaces("furnace",
        "furnace_side", "furnace_front", "furnace_top", "furnace_top",
        "furnace_side", "furnace_side", true, false);

    m_defs[BlockType::DISPENSER] = makeDefFaces("dispenser",
        "furnace_side", "dispenser_front", "furnace_top", "furnace_top",
        "furnace_side", "furnace_side", true, false);

    m_defs[BlockType::DROPPER] = makeDefFaces("dropper",
        "furnace_side", "dropper_front", "furnace_top", "furnace_top",
        "furnace_side", "furnace_side", true, false);

    m_defs[BlockType::OBSERVER] = makeDefFaces("observer",
        "observer_side", "observer_front", "observer_top", "observer_top",
        "observer_side", "observer_side", true, false);

    m_defs[BlockType::CRAFTING_TABLE] = makeDefFaces("crafting_table",
        "crafting_table_side", "crafting_table_front", "crafting_table_top", "oak_planks",
        "crafting_table_side", "crafting_table_side", true, false);

    m_defs[BlockType::BOOKSHELF] = makeDefFaces("bookshelf",
        "bookshelf", "bookshelf", "oak_planks", "oak_planks",
        "bookshelf", "bookshelf", true, false);

    m_defs[BlockType::NOTE_BLOCK] = makeDef("note_block", "note_block", true, false);

    m_defs[BlockType::JUKEBOX] = makeDefFaces("jukebox",
        "jukebox_side", "jukebox_side", "jukebox_top", "oak_planks",
        "jukebox_side", "jukebox_side", true, false);

    m_defs[BlockType::TNT] = makeDefFaces("tnt",
        "tnt_side", "tnt_side", "tnt_top", "tnt_bottom",
        "tnt_side", "tnt_side", true, false);

    uni(BlockType::OBSIDIAN, "obsidian", "obsidian");
    uni(BlockType::BEACON, "beacon", "beacon");

    m_defs[BlockType::CHEST] = makeDefFaces("chest",
        "oak_planks", "oak_planks", "oak_planks", "oak_planks",
        "oak_planks", "oak_planks", true, false);

    uni(BlockType::ENDER_CHEST, "ender_chest", "obsidian");
    uni(BlockType::ANVIL, "anvil", "anvil_top");
    uni(BlockType::ENCHANTING_TABLE, "enchanting_table", "enchanting_table_top");

    m_defs[BlockType::REDSTONE_LAMP] = makeDef("redstone_lamp", "redstone_lamp", true, false);

    m_defs[BlockType::PISTON] = makeDefFaces("piston",
        "piston_side", "piston_front", "piston_top", "piston_top",
        "piston_side", "piston_side", true, false);

    m_defs[BlockType::STICKY_PISTON] = makeDefFaces("sticky_piston",
        "piston_side", "piston_top_sticky", "piston_top", "piston_top",
        "piston_side", "piston_side", true, false);

    // Glass blocks
    auto glassBlock = [&](BlockType type, const std::string& name, const std::string& tex) {
        m_defs[type] = makeDef(name, tex, true, true);
    };
    glassBlock(BlockType::GLASS, "glass", "glass");
    glassBlock(BlockType::TINTED_GLASS, "tinted_glass", "tinted_glass");
    glassBlock(BlockType::WHITE_STAINED_GLASS, "white_stained_glass", "white_stained_glass");
    glassBlock(BlockType::ORANGE_STAINED_GLASS, "orange_stained_glass", "orange_stained_glass");
    glassBlock(BlockType::MAGENTA_STAINED_GLASS, "magenta_stained_glass", "magenta_stained_glass");
    glassBlock(BlockType::LIGHT_BLUE_STAINED_GLASS, "light_blue_stained_glass", "light_blue_stained_glass");
    glassBlock(BlockType::YELLOW_STAINED_GLASS, "yellow_stained_glass", "yellow_stained_glass");
    glassBlock(BlockType::LIME_STAINED_GLASS, "lime_stained_glass", "lime_stained_glass");
    glassBlock(BlockType::PINK_STAINED_GLASS, "pink_stained_glass", "pink_stained_glass");
    glassBlock(BlockType::GRAY_STAINED_GLASS, "gray_stained_glass", "gray_stained_glass");
    glassBlock(BlockType::LIGHT_GRAY_STAINED_GLASS, "light_gray_stained_glass", "light_gray_stained_glass");
    glassBlock(BlockType::CYAN_STAINED_GLASS, "cyan_stained_glass", "cyan_stained_glass");
    glassBlock(BlockType::PURPLE_STAINED_GLASS, "purple_stained_glass", "purple_stained_glass");
    glassBlock(BlockType::BLUE_STAINED_GLASS, "blue_stained_glass", "blue_stained_glass");
    glassBlock(BlockType::BROWN_STAINED_GLASS, "brown_stained_glass", "brown_stained_glass");
    glassBlock(BlockType::GREEN_STAINED_GLASS, "green_stained_glass", "green_stained_glass");
    glassBlock(BlockType::RED_STAINED_GLASS, "red_stained_glass", "red_stained_glass");
    glassBlock(BlockType::BLACK_STAINED_GLASS, "black_stained_glass", "black_stained_glass");

    // Terracotta blocks
    auto terracottaBlock = [&](BlockType type, const std::string& name, const std::string& tex) {
        m_defs[type] = makeDef(name, tex, true, false);
    };
    terracottaBlock(BlockType::TERRACOTTA, "terracotta", "terracotta");
    terracottaBlock(BlockType::WHITE_TERRACOTTA, "white_terracotta", "white_terracotta");
    terracottaBlock(BlockType::ORANGE_TERRACOTTA, "orange_terracotta", "orange_terracotta");
    terracottaBlock(BlockType::MAGENTA_TERRACOTTA, "magenta_terracotta", "magenta_terracotta");
    terracottaBlock(BlockType::LIGHT_BLUE_TERRACOTTA, "light_blue_terracotta", "light_blue_terracotta");
    terracottaBlock(BlockType::YELLOW_TERRACOTTA, "yellow_terracotta", "yellow_terracotta");
    terracottaBlock(BlockType::LIME_TERRACOTTA, "lime_terracotta", "lime_terracotta");
    terracottaBlock(BlockType::PINK_TERRACOTTA, "pink_terracotta", "pink_terracotta");
    terracottaBlock(BlockType::GRAY_TERRACOTTA, "gray_terracotta", "gray_terracotta");
    terracottaBlock(BlockType::LIGHT_GRAY_TERRACOTTA, "light_gray_terracotta", "light_gray_terracotta");
    terracottaBlock(BlockType::CYAN_TERRACOTTA, "cyan_terracotta", "cyan_terracotta");
    terracottaBlock(BlockType::PURPLE_TERRACOTTA, "purple_terracotta", "purple_terracotta");
    terracottaBlock(BlockType::BLUE_TERRACOTTA, "blue_terracotta", "blue_terracotta");
    terracottaBlock(BlockType::BROWN_TERRACOTTA, "brown_terracotta", "brown_terracotta");
    terracottaBlock(BlockType::GREEN_TERRACOTTA, "green_terracotta", "green_terracotta");
    terracottaBlock(BlockType::RED_TERRACOTTA, "red_terracotta", "red_terracotta");
    terracottaBlock(BlockType::BLACK_TERRACOTTA, "black_terracotta", "black_terracotta");

    // Wool blocks
    auto woolBlock = [&](BlockType type, const std::string& name, const std::string& tex) {
        m_defs[type] = makeDef(name, tex, true, false);
    };
    woolBlock(BlockType::WHITE_WOOL, "white_wool", "white_wool");
    woolBlock(BlockType::ORANGE_WOOL, "orange_wool", "orange_wool");
    woolBlock(BlockType::MAGENTA_WOOL, "magenta_wool", "magenta_wool");
    woolBlock(BlockType::LIGHT_BLUE_WOOL, "light_blue_wool", "light_blue_wool");
    woolBlock(BlockType::YELLOW_WOOL, "yellow_wool", "yellow_wool");
    woolBlock(BlockType::LIME_WOOL, "lime_wool", "lime_wool");
    woolBlock(BlockType::PINK_WOOL, "pink_wool", "pink_wool");
    woolBlock(BlockType::GRAY_WOOL, "gray_wool", "gray_wool");
    woolBlock(BlockType::LIGHT_GRAY_WOOL, "light_gray_wool", "light_gray_wool");
    woolBlock(BlockType::CYAN_WOOL, "cyan_wool", "cyan_wool");
    woolBlock(BlockType::PURPLE_WOOL, "purple_wool", "purple_wool");
    woolBlock(BlockType::BLUE_WOOL, "blue_wool", "blue_wool");
    woolBlock(BlockType::BROWN_WOOL, "brown_wool", "brown_wool");
    woolBlock(BlockType::GREEN_WOOL, "green_wool", "green_wool");
    woolBlock(BlockType::RED_WOOL, "red_wool", "red_wool");
    woolBlock(BlockType::BLACK_WOOL, "black_wool", "black_wool");

    // Concrete blocks
    auto concreteBlock = [&](BlockType type, const std::string& name, const std::string& tex) {
        m_defs[type] = makeDef(name, tex, true, false);
    };
    concreteBlock(BlockType::WHITE_CONCRETE, "white_concrete", "white_concrete");
    concreteBlock(BlockType::ORANGE_CONCRETE, "orange_concrete", "orange_concrete");
    concreteBlock(BlockType::MAGENTA_CONCRETE, "magenta_concrete", "magenta_concrete");
    concreteBlock(BlockType::LIGHT_BLUE_CONCRETE, "light_blue_concrete", "light_blue_concrete");
    concreteBlock(BlockType::YELLOW_CONCRETE, "yellow_concrete", "yellow_concrete");
    concreteBlock(BlockType::LIME_CONCRETE, "lime_concrete", "lime_concrete");
    concreteBlock(BlockType::PINK_CONCRETE, "pink_concrete", "pink_concrete");
    concreteBlock(BlockType::GRAY_CONCRETE, "gray_concrete", "gray_concrete");
    concreteBlock(BlockType::LIGHT_GRAY_CONCRETE, "light_gray_concrete", "light_gray_concrete");
    concreteBlock(BlockType::CYAN_CONCRETE, "cyan_concrete", "cyan_concrete");
    concreteBlock(BlockType::PURPLE_CONCRETE, "purple_concrete", "purple_concrete");
    concreteBlock(BlockType::BLUE_CONCRETE, "blue_concrete", "blue_concrete");
    concreteBlock(BlockType::BROWN_CONCRETE, "brown_concrete", "brown_concrete");
    concreteBlock(BlockType::GREEN_CONCRETE, "green_concrete", "green_concrete");
    concreteBlock(BlockType::RED_CONCRETE, "red_concrete", "red_concrete");
    concreteBlock(BlockType::BLACK_CONCRETE, "black_concrete", "black_concrete");

    // Ice blocks
    uni(BlockType::ICE, "ice", "ice");
    uni(BlockType::PACKED_ICE, "packed_ice", "packed_ice");
    uni(BlockType::BLUE_ICE, "blue_ice", "blue_ice");

    // Misc blocks
    uni(BlockType::SPONGE, "sponge", "sponge");
    uni(BlockType::WET_SPONGE, "wet_sponge", "wet_sponge");

    m_defs[BlockType::HAY_BLOCK] = makeDefFaces("hay_block",
        "hay_block_side", "hay_block_side", "hay_block_top", "hay_block_top",
        "hay_block_side", "hay_block_side", true, false);

    m_defs[BlockType::MELON] = makeDefFaces("melon",
        "melon_side", "melon_side", "melon_top", "melon_top",
        "melon_side", "melon_side", true, false);

    m_defs[BlockType::PUMPKIN] = makeDefFaces("pumpkin",
        "pumpkin_side", "pumpkin_side", "pumpkin_top", "pumpkin_top",
        "pumpkin_side", "pumpkin_side", true, false);

    m_defs[BlockType::CARVED_PUMPKIN] = makeDefFaces("carved_pumpkin",
        "pumpkin_side", "carved_pumpkin", "pumpkin_top", "pumpkin_top",
        "pumpkin_side", "pumpkin_side", true, false);

    uni(BlockType::CACTUS, "cactus", "cactus_side");

    m_defs[BlockType::FARMLAND] = makeDefFaces("farmland",
        "dirt", "dirt", "farmland", "dirt", "dirt", "dirt", true, false);

    m_defs[BlockType::GRASS_BLOCK] = makeDefFaces("grass_block",
        "grass_block_side", "grass_block_side",
        "grass_block_top", "dirt",
        "grass_block_side", "grass_block_side",
        true, false, TOP_GRASS_TINT);

    uni(BlockType::HONEY_BLOCK, "honey_block", "honey_block_side");
    uni(BlockType::HONEYCOMB_BLOCK, "honeycomb_block", "honeycomb_block");
    uni(BlockType::SLIME_BLOCK, "slime_block", "slime_block");
    uni(BlockType::MOSS_BLOCK, "moss_block", "moss_block");
    uni(BlockType::SEA_LANTERN, "sea_lantern", "sea_lantern");
    uni(BlockType::CALCITE, "calcite", "calcite");
    uni(BlockType::TUFF, "tuff", "tuff");
    uni(BlockType::DRIPSTONE_BLOCK, "dripstone_block", "dripstone_block");
    uni(BlockType::AMETHYST_BLOCK, "amethyst_block", "amethyst_block");
    uni(BlockType::BUDDING_AMETHYST, "budding_amethyst", "budding_amethyst");

    std::cout << "BlockRegistry: initialized " << m_defs.size() << " block types\n";
}

// Looks up the BlockDef for a given block type, returns empty def if not found
const BlockDef& BlockRegistry::getDef(BlockType type) const {
    auto it = m_defs.find(type);
    if (it != m_defs.end()) {
        return it->second;
    }
    static const BlockDef empty = {"unknown", {"stone","stone","stone","stone","stone","stone"}, NO_TINT, true, false};
    return empty;
}

// Returns the six atlas tile indices for the block's face textures
std::array<unsigned int, 6> BlockRegistry::getBlockTextures(
    const TextureManager& texMgr, BlockType type) const
{
    const auto& def = getDef(type);
    std::array<unsigned int, 6> texs;
    for (int i = 0; i < 6; i++) {
        texs[i] = texMgr.getTileIndex(def.textures[i]);
    }
    return texs;
}
