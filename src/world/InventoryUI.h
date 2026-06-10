#pragma once
#include <SFML/Graphics.hpp>
#include "Inventory.h"
#include "TextureManager.h"
#include "Item.h"
#include "CraftingSystem.h"
#include "../core/Camera.h"

class InventoryUI {
public:
    InventoryUI();

    void loadTextures(const std::string& assetsPath);

    void renderCrosshair(sf::RenderWindow& window, unsigned int windowWidth, unsigned int windowHeight);

    void renderHotbar(sf::RenderWindow& window, const Inventory& inventory,
                      const TextureManager& texMgr, const ItemRegistry& itemReg,
                      unsigned int windowWidth, unsigned int windowHeight);

    void renderInventory(sf::RenderWindow& window, const Inventory& inventory,
                         const CraftingSystem& crafting,
                         const TextureManager& texMgr, const ItemRegistry& itemReg,
                         unsigned int windowWidth, unsigned int windowHeight,
                         const sf::Vector2i& mousePos, bool mousePressed, bool mouseReleased);

    bool handleMouseInput(const sf::Vector2i& mousePos, bool leftPressed, bool leftReleased,
                          bool rightPressed, bool rightReleased,
                          Inventory& inventory, CraftingSystem& crafting,
                          unsigned int windowWidth, unsigned int windowHeight);

    bool isInventoryOpen() const { return m_inventoryOpen; }
    void toggleInventory() { m_inventoryOpen = !m_inventoryOpen; }
    void setInventoryOpen(bool open) { m_inventoryOpen = open; }

private:
    bool m_inventoryOpen = false;
    sf::Font m_font;
    bool m_fontLoaded = false;

    sf::Texture m_widgetsTex;
    sf::Texture m_iconsTex;
    sf::Texture m_inventoryTex;
    bool m_texturesLoaded = false;

    sf::Texture m_blockAtlasTex;
    sf::Texture m_itemAtlasTex;
    bool m_blockAtlasLoaded = false;
    bool m_itemAtlasLoaded = false;

    struct SlotRect {
        sf::FloatRect bounds;
        int slotIndex;
    };
    std::vector<SlotRect> m_slotRects;

    static constexpr float HOTBAR_SCALE = 2.0f;
    static constexpr float HOTBAR_W = 182.0f * HOTBAR_SCALE;
    static constexpr float HOTBAR_H = 22.0f * HOTBAR_SCALE;
    static constexpr float SLOT_SIZE = 18.0f * HOTBAR_SCALE;
    static constexpr float SLOT_GAP = 2.0f;
    static constexpr float SELECTED_SIZE = 24.0f * HOTBAR_SCALE;

    void drawSlotBg(sf::RenderWindow& window, const sf::Vector2f& pos, float size, bool selected);
    void drawItemStack(sf::RenderWindow& window, const ItemStack& stack,
                       const TextureManager& texMgr, const ItemRegistry& itemReg,
                       const sf::Vector2f& pos, float size);

    // Loads a font file, falling back to a system font if the primary is missing.
    void loadFont();
};
