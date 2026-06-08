#include "InventoryUI.h"
#include "Item.h"
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <iostream>

// Initialize the UI and load the default font.
InventoryUI::InventoryUI() {
    loadFont();
}

// Load a font file, falling back to a system font if the primary is missing.
void InventoryUI::loadFont() {
    std::string fontPath = "assets/fonts/minecraft.ttf";
    if (!std::filesystem::exists(fontPath)) {
        fontPath = "assets/minecraft-rus-regular1.ttf";
        if (!std::filesystem::exists(fontPath))
            fontPath = "/System/Library/Fonts/Helvetica.ttc";
    }
    m_fontLoaded = m_font.openFromFile(fontPath);
}

// Load all GUI textures from the given assets path.
void InventoryUI::loadTextures(const std::string& assetsPath) {
    std::string guiPath = assetsPath + "/minecraft/textures/gui";

    std::string widgetsFile = guiPath + "/widgets.png";
    std::string iconsFile = guiPath + "/icons.png";
    std::string invFile = guiPath + "/container/inventory.png";

    if (!std::filesystem::exists(widgetsFile) ||
        !std::filesystem::exists(iconsFile) ||
        !std::filesystem::exists(invFile)) {
        std::cout << "ERROR: One or more GUI textures are missing in " << guiPath << "\n";
        m_texturesLoaded = false;
        return;
    }

    if (m_widgetsTex.loadFromFile(widgetsFile) &&
        m_iconsTex.loadFromFile(iconsFile) &&
        m_inventoryTex.loadFromFile(invFile)) {
        m_texturesLoaded = true;
        std::cout << "InventoryUI: Textures loaded successfully.\n";
    }
}

// Draw the crosshair centered on screen.
void InventoryUI::renderCrosshair(sf::RenderWindow& window, unsigned int windowWidth, unsigned int windowHeight) {
    if (!m_texturesLoaded) return;

    const int CW = 16, CH = 16;
    const float SCALE = 2.0f;

    sf::Sprite crosshair(m_iconsTex, sf::IntRect({0, 0}, {CW, CH}));
    crosshair.setScale({SCALE, SCALE});
    crosshair.setOrigin({CW / 2.0f, CH / 2.0f});

    auto size = window.getSize();
    crosshair.setPosition({size.x * 0.5f, size.y * 0.5f});
    window.draw(crosshair);
}

// Render the hotbar background, selection highlight, and item icons.
void InventoryUI::renderHotbar(sf::RenderWindow& window, const Inventory& inventory,
                               const TextureManager& texMgr, const ItemRegistry& itemReg,
                               unsigned int windowWidth, unsigned int windowHeight) {
    if (!m_texturesLoaded) return;

    float startX = (windowWidth - HOTBAR_W) / 2.0f;
    float startY = windowHeight - HOTBAR_H - 10.0f;

    sf::Sprite hotbarBg(m_widgetsTex, sf::IntRect({0, 0}, {182, 22}));
    hotbarBg.setScale({HOTBAR_SCALE, HOTBAR_SCALE});
    hotbarBg.setPosition({startX, startY});
    window.draw(hotbarBg);

    int selIdx = inventory.getSelectedHotbarIndex();
    float selX = startX + (selIdx * 20.0f - 1.0f) * HOTBAR_SCALE;
    float selY = startY - 1.0f * HOTBAR_SCALE;

    sf::Sprite selectedSlot(m_widgetsTex, sf::IntRect({0, 22}, {24, 24}));
    selectedSlot.setScale({HOTBAR_SCALE, HOTBAR_SCALE});
    selectedSlot.setPosition({selX, selY});
    window.draw(selectedSlot);

    if (!m_blockAtlasLoaded) {
        (void)m_blockAtlasTex.loadFromImage(texMgr.getImage());
        m_blockAtlasLoaded = true;
    }
    if (!m_itemAtlasLoaded) {
        (void)m_itemAtlasTex.loadFromImage(texMgr.getItemImage());
        m_itemAtlasLoaded = true;
    }

    for (int i = 0; i < Inventory::HOTBAR_SIZE; i++) {
        const ItemStack& stack = inventory.getHotbarSlot(i);
        if (!stack.isEmpty()) {
            float ix = startX + (3.0f + i * 20.0f) * HOTBAR_SCALE;
            float iy = startY + 3.0f * HOTBAR_SCALE;
            drawItemStack(window, stack, texMgr, itemReg, {ix, iy}, HOTBAR_SCALE);
        }
    }
}

// Render the full inventory panel: main grid, hotbar, crafting, armor, and drag item.
void InventoryUI::renderInventory(sf::RenderWindow& window, const Inventory& inventory,
                                  const CraftingSystem& crafting,
                                  const TextureManager& texMgr, const ItemRegistry& itemReg,
                                  unsigned int windowWidth, unsigned int windowHeight,
                                  const sf::Vector2i& mousePos, bool mousePressed, bool mouseReleased) {
    if (!m_inventoryOpen || !m_texturesLoaded) return;

    m_slotRects.clear();

    const float INV_SCALE = 2.0f;
    const float INV_W = 176.0f * INV_SCALE;
    const float INV_H = 166.0f * INV_SCALE;
    float panelX = (windowWidth - INV_W) / 2.0f;
    float panelY = (windowHeight - INV_H) / 2.0f;

    sf::Sprite invBg(m_inventoryTex, sf::IntRect({0, 0}, {176, 166}));
    invBg.setScale({INV_SCALE, INV_SCALE});
    invBg.setPosition({panelX, panelY});
    window.draw(invBg);

    const float ITEM_SIZE_PX = 16.0f;
    const float SLOT_SIZE_PX = 18.0f;

    if (!m_blockAtlasLoaded) {
        (void)m_blockAtlasTex.loadFromImage(texMgr.getImage());
        m_blockAtlasLoaded = true;
    }
    if (!m_itemAtlasLoaded) {
        (void)m_itemAtlasTex.loadFromImage(texMgr.getItemImage());
        m_itemAtlasLoaded = true;
    }

    auto addSlot = [&](float texX, float texY, int slotIndex) {
        float sx = panelX + texX * INV_SCALE;
        float sy = panelY + texY * INV_SCALE;
        float sSize = SLOT_SIZE_PX * INV_SCALE;

        SlotRect sr;
        sr.bounds = sf::FloatRect({sx, sy}, {sSize, sSize});
        sr.slotIndex = slotIndex;
        m_slotRects.push_back(sr);

        return sf::Vector2f(sx, sy);
    };

    // Main inventory grid (3 rows x 9 cols)
    for (int row = 0; row < Inventory::MAIN_ROWS; row++) {
        for (int col = 0; col < Inventory::MAIN_COLS; col++) {
            int slotIndex = Inventory::HOTBAR_SIZE + row * Inventory::MAIN_COLS + col;
            float texX = 8.0f + col * SLOT_SIZE_PX;
            float texY = 84.0f + row * SLOT_SIZE_PX;
            sf::Vector2f pos = addSlot(texX, texY, slotIndex);

            const ItemStack& stack = inventory.getSlot(slotIndex);
            if (!stack.isEmpty()) {
                float itemX = pos.x + 1.0f * INV_SCALE;
                float itemY = pos.y + 1.0f * INV_SCALE;
                drawItemStack(window, stack, texMgr, itemReg, {itemX, itemY}, INV_SCALE);
            }
        }
    }

    // Hotbar slots within the inventory panel
    for (int i = 0; i < Inventory::HOTBAR_SIZE; i++) {
        int slotIndex = i;
        float texX = 8.0f + i * SLOT_SIZE_PX;
        float texY = 142.0f;
        sf::Vector2f pos = addSlot(texX, texY, slotIndex);

        const ItemStack& stack = inventory.getSlot(slotIndex);
        if (!stack.isEmpty()) {
            float itemX = pos.x + 1.0f * INV_SCALE;
            float itemY = pos.y + 1.0f * INV_SCALE;
            drawItemStack(window, stack, texMgr, itemReg, {itemX, itemY}, INV_SCALE);
        }
    }

    // 2x2 crafting grid
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 2; col++) {
            int craftIndex = row * 2 + col;
            float texX = 98.0f + col * SLOT_SIZE_PX;
            float texY = 18.0f + row * SLOT_SIZE_PX;
            int slotIndex = Inventory::TOTAL_SIZE + craftIndex;
            sf::Vector2f pos = addSlot(texX, texY, slotIndex);

            const ItemStack& stack = crafting.getGridSlot(craftIndex);
            if (!stack.isEmpty()) {
                float itemX = pos.x + 1.0f * INV_SCALE;
                float itemY = pos.y + 1.0f * INV_SCALE;
                drawItemStack(window, stack, texMgr, itemReg, {itemX, itemY}, INV_SCALE);
            }
        }
    }

    // Crafting result slot
    {
        float texX = 152.0f;
        float texY = 28.0f;
        int resultIndex = Inventory::TOTAL_SIZE + CraftingSystem::GRID_SIZE;
        sf::Vector2f pos = addSlot(texX, texY, resultIndex);

        const ItemStack& stack = crafting.getResultSlot();
        if (!stack.isEmpty()) {
            float itemX = pos.x + 1.0f * INV_SCALE;
            float itemY = pos.y + 1.0f * INV_SCALE;
            drawItemStack(window, stack, texMgr, itemReg, {itemX, itemY}, INV_SCALE);
        }
    }

    // Armor slots on the left side
    {
        static constexpr int ARMOR_SLOT_START = Inventory::TOTAL_SIZE + CraftingSystem::GRID_SIZE + 1;
        static constexpr float armorTexCoords[4][2] = {
            {8.0f, 8.0f},
            {8.0f, 26.0f},
            {8.0f, 44.0f},
            {8.0f, 62.0f}
        };
        for (int i = 0; i < Inventory::ARMOR_SLOTS; i++) {
            int slotIndex = ARMOR_SLOT_START + i;
            sf::Vector2f pos = addSlot(armorTexCoords[i][0], armorTexCoords[i][1], slotIndex);

            const ItemStack& stack = inventory.getArmorSlot(i);
            if (!stack.isEmpty()) {
                float itemX = pos.x + 1.0f * INV_SCALE;
                float itemY = pos.y + 1.0f * INV_SCALE;
                drawItemStack(window, stack, texMgr, itemReg, {itemX, itemY}, INV_SCALE);
            }
        }
    }

    // Item currently being dragged by the cursor
    const Inventory::DragState& dragState = inventory.getDragState();
    if (dragState.active && !dragState.stack.isEmpty()) {
        float dragOffset = (ITEM_SIZE_PX * INV_SCALE) / 2.0f;
        drawItemStack(window, dragState.stack, texMgr, itemReg,
                      {static_cast<float>(mousePos.x) - dragOffset, static_cast<float>(mousePos.y) - dragOffset}, INV_SCALE);
    }
}

// Placeholder: slot background is already drawn by the inventory texture.
void InventoryUI::drawSlotBg(sf::RenderWindow& window, const sf::Vector2f& pos, float size, bool selected) {
}

// Draw an item icon with its stack count inside a slot.
void InventoryUI::drawItemStack(sf::RenderWindow& window, const ItemStack& stack,
                                const TextureManager& texMgr, const ItemRegistry& itemReg,
                                const sf::Vector2f& pos, float scale) {
    if (stack.isEmpty()) return;

    const Item& item = itemReg.getItem(stack.type);
    bool isArmorItem = Inventory::isArmor(stack.type);

    unsigned int texIndex;
    std::array<sf::Vector2f, 4> uvs;
    sf::Texture* atlasTex;

    if (isArmorItem) {
        texIndex = texMgr.getItemTileIndex(item.textureName);
        uvs = texMgr.getItemUV(texIndex);
        atlasTex = &m_itemAtlasTex;
    } else {
        texIndex = texMgr.getTileIndex(item.textureName);
        uvs = texMgr.getUV(texIndex);
        atlasTex = &m_blockAtlasTex;
    }

    float drawSize = 16.0f * scale;

    float u0 = uvs[0].x;
    float v0 = uvs[0].y;
    float u1 = uvs[2].x;
    float v1 = uvs[2].y;

    sf::VertexArray quad(sf::PrimitiveType::Triangles, 6);
    quad[0].position = {pos.x, pos.y + drawSize};
    quad[1].position = {pos.x + drawSize, pos.y + drawSize};
    quad[2].position = {pos.x + drawSize, pos.y};
    quad[3].position = {pos.x, pos.y + drawSize};
    quad[4].position = {pos.x + drawSize, pos.y};
    quad[5].position = {pos.x, pos.y};

    quad[0].texCoords = {u0, v1};
    quad[1].texCoords = {u1, v1};
    quad[2].texCoords = {u1, v0};
    quad[3].texCoords = {u0, v1};
    quad[4].texCoords = {u1, v0};
    quad[5].texCoords = {u0, v0};

    for (int i = 0; i < 6; i++) {
        quad[i].color = sf::Color::White;
    }

    sf::RenderStates states;
    states.texture = atlasTex;
    window.draw(quad, states);

    // Draw stack count in the bottom-right corner
    if (stack.count > 1 && m_fontLoaded) {
        sf::Text countText(m_font);
        countText.setString(std::to_string(stack.count));

        countText.setCharacterSize(static_cast<unsigned int>(10 * scale));
        countText.setFillColor(sf::Color::White);
        countText.setOutlineColor(sf::Color(62, 62, 62));
        countText.setOutlineThickness(1.0f);

        countText.setPosition({pos.x + drawSize - 11 * scale, pos.y + drawSize - 12 * scale});
        window.draw(countText);
    }
}

// Validate whether an armor piece matches its target slot index.
static bool isValidArmorSlotPlacement(int armorIndex, BlockType type) {
    switch (armorIndex) {
        case Inventory::ARMOR_HELMET:
            return type == BlockType::LEATHER_HELMET ||
                   type == BlockType::CHAINMAIL_HELMET ||
                   type == BlockType::IRON_HELMET ||
                   type == BlockType::GOLDEN_HELMET ||
                   type == BlockType::DIAMOND_HELMET ||
                   type == BlockType::NETHERITE_HELMET;
        case Inventory::ARMOR_CHESTPLATE:
            return type == BlockType::LEATHER_CHESTPLATE ||
                   type == BlockType::CHAINMAIL_CHESTPLATE ||
                   type == BlockType::IRON_CHESTPLATE ||
                   type == BlockType::GOLDEN_CHESTPLATE ||
                   type == BlockType::DIAMOND_CHESTPLATE ||
                   type == BlockType::NETHERITE_CHESTPLATE;
        case Inventory::ARMOR_LEGGINGS:
            return type == BlockType::LEATHER_LEGGINGS ||
                   type == BlockType::CHAINMAIL_LEGGINGS ||
                   type == BlockType::IRON_LEGGINGS ||
                   type == BlockType::GOLDEN_LEGGINGS ||
                   type == BlockType::DIAMOND_LEGGINGS ||
                   type == BlockType::NETHERITE_LEGGINGS;
        case Inventory::ARMOR_BOOTS:
            return type == BlockType::LEATHER_BOOTS ||
                   type == BlockType::CHAINMAIL_BOOTS ||
                   type == BlockType::IRON_BOOTS ||
                   type == BlockType::GOLDEN_BOOTS ||
                   type == BlockType::DIAMOND_BOOTS ||
                   type == BlockType::NETHERITE_BOOTS;
        default:
            return false;
    }
}

// Process mouse clicks and releases for inventory slot interaction.
bool InventoryUI::handleMouseInput(const sf::Vector2i& mousePos, bool leftPressed, bool leftReleased,
                                   bool rightPressed, bool rightReleased,
                                   Inventory& inventory, CraftingSystem& crafting,
                                   unsigned int windowWidth, unsigned int windowHeight) {
    if (!m_inventoryOpen || !m_texturesLoaded) return false;

    Inventory::DragState& dragState = const_cast<Inventory::DragState&>(inventory.getDragState());

    // Determine which slot the mouse is hovering over
    int hoveredSlot = -1;
    sf::Vector2f mf(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    for (const auto& sr : m_slotRects) {
        if (sr.bounds.contains(mf)) {
            hoveredSlot = sr.slotIndex;
            break;
        }
    }

    static constexpr int ARMOR_SLOT_START = Inventory::TOTAL_SIZE + CraftingSystem::GRID_SIZE + 1;

    auto isCraftGrid = [](int idx) -> bool {
        return idx >= Inventory::TOTAL_SIZE && idx < Inventory::TOTAL_SIZE + CraftingSystem::GRID_SIZE;
    };
    auto isCraftResult = [](int idx) -> bool {
        return idx == Inventory::TOTAL_SIZE + CraftingSystem::GRID_SIZE;
    };
    auto isArmorSlot = [](int idx) -> bool {
        return idx >= ARMOR_SLOT_START && idx < ARMOR_SLOT_START + Inventory::ARMOR_SLOTS;
    };
    auto getArmorIndex = [](int idx) -> int {
        return idx - ARMOR_SLOT_START;
    };
    auto getCraftGridIndex = [](int idx) -> int {
        return idx - Inventory::TOTAL_SIZE;
    };

    auto getSlotRef = [&](int idx) -> ItemStack* {
        if (idx >= 0 && idx < Inventory::TOTAL_SIZE) {
            return &inventory.getSlot(idx);
        }
        if (isCraftGrid(idx)) {
            return &crafting.getGridSlot(getCraftGridIndex(idx));
        }
        if (isArmorSlot(idx)) {
            return &inventory.getArmorSlot(getArmorIndex(idx));
        }
        return nullptr;
    };

    auto refreshCraft = [&]() {
        crafting.updateResult();
    };

    // Left-click on crafting result slot: craft the item
    if (leftPressed && isCraftResult(hoveredSlot) && !crafting.getResultSlot().isEmpty()) {
        if (!dragState.active) {
            ItemStack result = crafting.getResultSlot();
            int stackSize = result.count;

            crafting.craft();
            dragState.active = true;
            dragState.stack = ItemStack(result.type, stackSize);
            dragState.sourceIndex = -1;

            refreshCraft();

            return true;
        } else {
            ItemStack result = crafting.getResultSlot();
            if (dragState.stack.canStackWith(result)) {
                int space = dragState.stack.getFreeSpace();
                int count = result.count;
                if (space >= count) {
                    crafting.craft();
                    dragState.stack.add(count);
                    refreshCraft();
                    return true;
                }
            }
        }
    }

    // Left-click on a regular slot: pick up, place, merge, or swap
    if (leftPressed && hoveredSlot >= 0 && !isCraftResult(hoveredSlot)) {
        ItemStack* slot = getSlotRef(hoveredSlot);
        if (!slot) return false;

        if (!dragState.active) {
            // Pick up entire stack
            if (!slot->isEmpty()) {
                dragState.active = true;
                dragState.stack = *slot;
                dragState.sourceIndex = hoveredSlot;
                *slot = ItemStack();
                if (isCraftGrid(hoveredSlot)) refreshCraft();
            }
        } else {
            // Validate armor slot placement before proceeding
            if (isArmorSlot(hoveredSlot)) {
                int armorIdx = getArmorIndex(hoveredSlot);
                if (!isValidArmorSlotPlacement(armorIdx, dragState.stack.type)) {
                    return false;
                }
            }

            if (slot->isEmpty()) {
                // Place held stack into empty slot
                *slot = dragState.stack;
                dragState.active = false;
                dragState.stack = ItemStack();
                dragState.sourceIndex = -1;
            } else if (slot->canStackWith(dragState.stack)) {
                // Merge into existing compatible stack
                int space = slot->getFreeSpace();
                if (space > 0) {
                    int move = std::min(dragState.stack.count, space);
                    slot->add(move);
                    dragState.stack.remove(move);
                } else {
                    // Stack is full: swap
                    std::swap(*slot, dragState.stack);
                    dragState.sourceIndex = hoveredSlot;
                }
                if (dragState.stack.isEmpty()) {
                    dragState.active = false;
                    dragState.sourceIndex = -1;
                }
            } else {
                // Incompatible types: swap
                std::swap(*slot, dragState.stack);
                dragState.sourceIndex = hoveredSlot;
            }
            if (isCraftGrid(hoveredSlot)) refreshCraft();
        }
        return true;
    }

    // Right-click on a regular slot: pick up half or place one item
    if (rightPressed && hoveredSlot >= 0 && !isCraftResult(hoveredSlot)) {
        ItemStack* slot = getSlotRef(hoveredSlot);
        if (!slot) return false;

        if (!dragState.active) {
            // Pick up half of the stack
            if (!slot->isEmpty() && slot->count > 0) {
                int half = (slot->count + 1) / 2;
                dragState.active = true;
                dragState.stack = ItemStack(slot->type, half);
                dragState.sourceIndex = hoveredSlot;
                slot->count -= half;
                if (slot->count == 0) slot->type = BlockType::AIR;
                if (isCraftGrid(hoveredSlot)) refreshCraft();
            }
        } else {
            // Validate armor slot placement before proceeding
            if (isArmorSlot(hoveredSlot)) {
                int armorIdx = getArmorIndex(hoveredSlot);
                if (!isValidArmorSlotPlacement(armorIdx, dragState.stack.type)) {
                    return false;
                }
            }

            if (slot->isEmpty()) {
                // Place one item into empty slot
                *slot = ItemStack(dragState.stack.type, 1);
                dragState.stack.remove(1);
                if (dragState.stack.isEmpty()) {
                    dragState.active = false;
                    dragState.sourceIndex = -1;
                }
            } else if (slot->canStackWith(dragState.stack)) {
                // Add one item to compatible stack if there is room
                int space = slot->getFreeSpace();
                if (space > 0) {
                    slot->add(1);
                    dragState.stack.remove(1);
                    if (dragState.stack.isEmpty()) {
                        dragState.active = false;
                        dragState.sourceIndex = -1;
                    }
                }
            }
            if (isCraftGrid(hoveredSlot)) refreshCraft();
        }
        return true;
    }

    // Release outside any slot: return dragged items to their source
    if ((leftReleased || rightReleased) && dragState.active && hoveredSlot < 0) {
        if (dragState.sourceIndex >= 0) {
            ItemStack* slot = getSlotRef(dragState.sourceIndex);
            if (slot) {
                *slot = dragState.stack;
                if (isCraftGrid(dragState.sourceIndex)) refreshCraft();
            }
        }
        dragState.active = false;
        dragState.stack = ItemStack();
        dragState.sourceIndex = -1;
        return true;
    }

    return false;
}
