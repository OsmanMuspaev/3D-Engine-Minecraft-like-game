#include "CraftingSystem.h"
#include <iostream>

std::vector<CraftingRecipe> CraftingSystem::s_recipes;
bool CraftingSystem::s_recipesInitialized = false;

// Initializes the grid and recipe list.
CraftingSystem::CraftingSystem() {
    m_grid.fill(ItemStack());
    initRecipes();
}

// Returns a mutable reference to the grid slot at the given index.
ItemStack& CraftingSystem::getGridSlot(int index) {
    return m_grid[index];
}

// Returns a const reference to the grid slot at the given index.
const ItemStack& CraftingSystem::getGridSlot(int index) const {
    return m_grid[index];
}

// Returns a mutable reference to the result slot.
ItemStack& CraftingSystem::getResultSlot() {
    return m_result;
}

// Returns a const reference to the result slot.
const ItemStack& CraftingSystem::getResultSlot() const {
    return m_result;
}

// Populates the static recipe list with all 2x2 crafting recipes (once).
void CraftingSystem::initRecipes() {
    if (s_recipesInitialized) return;
    s_recipesInitialized = true;

    s_recipes.push_back({{BlockType::OAK_PLANKS, BlockType::OAK_PLANKS,
                          BlockType::OAK_PLANKS, BlockType::OAK_PLANKS},
                         BlockType::CRAFTING_TABLE, 1});

    s_recipes.push_back({{BlockType::COBBLESTONE, BlockType::COBBLESTONE,
                          BlockType::COBBLESTONE, BlockType::COBBLESTONE},
                         BlockType::FURNACE, 1});

    s_recipes.push_back({{BlockType::STONE, BlockType::STONE,
                          BlockType::STONE, BlockType::STONE},
                         BlockType::STONE_BRICKS, 4});

    s_recipes.push_back({{BlockType::IRON_BLOCK, BlockType::IRON_BLOCK,
                          BlockType::IRON_BLOCK, BlockType::IRON_BLOCK},
                         BlockType::ANVIL, 1});

    s_recipes.push_back({{BlockType::DIAMOND_BLOCK, BlockType::DIAMOND_BLOCK,
                          BlockType::DIAMOND_BLOCK, BlockType::DIAMOND_BLOCK},
                         BlockType::ENCHANTING_TABLE, 1});

    s_recipes.push_back({{BlockType::OAK_PLANKS, BlockType::OAK_PLANKS,
                          BlockType::OAK_PLANKS, BlockType::OAK_PLANKS},
                         BlockType::CHEST, 1});

    s_recipes.push_back({{BlockType::SAND, BlockType::SAND,
                          BlockType::SAND, BlockType::SAND},
                         BlockType::SANDSTONE, 1});

    s_recipes.push_back({{BlockType::BRICKS, BlockType::BRICKS,
                          BlockType::BRICKS, BlockType::BRICKS},
                         BlockType::BRICKS, 4});

    std::cout << "CraftingSystem: " << s_recipes.size() << " recipes initialized\n";
}

// Returns the list of all registered crafting recipes.
const std::vector<CraftingRecipe>& CraftingSystem::getRecipes() {
    return s_recipes;
}

// Checks the grid against shapeless and shaped recipes, updating the result slot.
void CraftingSystem::updateResult() {
    // Count non-empty slots and record the first filled type
    int filledCount = 0;
    int firstIndex = -1;
    BlockType firstType = BlockType::AIR;

    for (int i = 0; i < GRID_SIZE; i++) {
        if (!m_grid[i].isEmpty()) {
            filledCount++;
            if (firstIndex < 0) {
                firstIndex = i;
                firstType = m_grid[i].type;
            }
        }
    }

    if (filledCount == 0) {
        m_result = ItemStack();
        return;
    }

    // Shapeless recipes: single log -> 4 planks, single cobblestone -> stone
    if (filledCount == 1) {
        if (firstType == BlockType::OAK_LOG) {
            m_result = ItemStack(BlockType::OAK_PLANKS, 4);
            return;
        }
        if (firstType == BlockType::SPRUCE_LOG) {
            m_result = ItemStack(BlockType::SPRUCE_PLANKS, 4);
            return;
        }
        if (firstType == BlockType::BIRCH_LOG) {
            m_result = ItemStack(BlockType::BIRCH_PLANKS, 4);
            return;
        }
        if (firstType == BlockType::JUNGLE_LOG) {
            m_result = ItemStack(BlockType::JUNGLE_PLANKS, 4);
            return;
        }
        if (firstType == BlockType::ACACIA_LOG) {
            m_result = ItemStack(BlockType::ACACIA_PLANKS, 4);
            return;
        }
        if (firstType == BlockType::DARK_OAK_LOG) {
            m_result = ItemStack(BlockType::DARK_OAK_PLANKS, 4);
            return;
        }
        if (firstType == BlockType::COBBLESTONE) {
            m_result = ItemStack(BlockType::STONE, 1);
            return;
        }
    }

    // Exact 2x2 pattern matching
    for (const auto& recipe : s_recipes) {
        bool match = true;
        for (int i = 0; i < GRID_SIZE; i++) {
            BlockType expected = recipe.pattern[i];
            BlockType actual = m_grid[i].type;

            if (expected == BlockType::AIR && actual != BlockType::AIR) {
                match = false;
                break;
            }
            if (expected != BlockType::AIR && actual != expected) {
                match = false;
                break;
            }
        }
        if (match) {
            m_result = ItemStack(recipe.result, recipe.resultCount);
            return;
        }
    }

    m_result = ItemStack();
}

// Consumes one item from each filled grid slot, leaving the result in place.
void CraftingSystem::craft() {
    if (m_result.isEmpty()) return;

    for (int i = 0; i < GRID_SIZE; i++) {
        if (!m_grid[i].isEmpty()) {
            m_grid[i].remove(1);
        }
    }
}
