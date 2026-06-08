#pragma once
#include "Item.h"
#include <vector>
#include <array>

// A 2x2 crafting recipe with an input pattern and output item.
struct CraftingRecipe {
    std::array<BlockType, 4> pattern; // 2x2 grid, row-major
    BlockType result;
    int resultCount = 1;
};

// Manages a 2x2 crafting grid and matches items against known recipes.
class CraftingSystem {
public:
    static constexpr int GRID_SIZE = 4;
    static constexpr int RESULT_SLOTS = 1;
    static constexpr int TOTAL_CRAFT = GRID_SIZE + RESULT_SLOTS;

    CraftingSystem();

    ItemStack& getGridSlot(int index);
    const ItemStack& getGridSlot(int index) const;
    ItemStack& getResultSlot();
    const ItemStack& getResultSlot() const;

    void updateResult();
    void craft();

    static const std::vector<CraftingRecipe>& getRecipes();

private:
    std::array<ItemStack, GRID_SIZE> m_grid;
    ItemStack m_result;

    static std::vector<CraftingRecipe> s_recipes;
    static bool s_recipesInitialized;
    static void initRecipes();
};
