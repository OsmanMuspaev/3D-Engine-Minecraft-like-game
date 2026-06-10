#pragma once
#include "Item.h"
#include <vector>
#include <array>

struct CraftingRecipe {
    std::array<BlockType, 4> pattern;
    BlockType result;
    int resultCount = 1;
};

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

    // Checks the grid against recipes and updates the result slot.
    void updateResult();
    // Consumes one item from each filled grid slot.
    void craft();

    // Returns the list of all registered crafting recipes.
    static const std::vector<CraftingRecipe>& getRecipes();

private:
    std::array<ItemStack, GRID_SIZE> m_grid;
    ItemStack m_result;

    static std::vector<CraftingRecipe> s_recipes;
    static bool s_recipesInitialized;
    // Populates the static recipe list with all 2x2 crafting recipes (once).
    static void initRecipes();
};
