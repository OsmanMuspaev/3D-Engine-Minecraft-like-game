#include "InventoryUI.h"
#include "Item.h"
#include <filesystem>
#include <cmath>
#include <algorithm>
#include <iostream>

InventoryUI::InventoryUI() {
    loadFont();
}

void InventoryUI::loadFont() {
    std::string fontPath = "assets/fonts/minecraft.ttf";
    if (!std::filesystem::exists(fontPath)) {
        fontPath = "/System/Library/Fonts/Helvetica.ttc";
    }
    m_fontLoaded = m_font.openFromFile(fontPath);
}

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

void InventoryUI::renderHotbar(sf::RenderWindow& window, const Inventory& inventory,
                               const TextureManager& texMgr, const ItemRegistry& itemReg,
                               unsigned int windowWidth, unsigned int windowHeight) {
    if (!m_texturesLoaded) return;

    float startX = (windowWidth - HOTBAR_W) / 2.0f;
    float startY = windowHeight - HOTBAR_H - 10.0f;

    // 1. Рисуем подложку хотбара (182x22 пикселей)
    sf::Sprite hotbarBg(m_widgetsTex, sf::IntRect({0, 0}, {182, 22}));
    hotbarBg.setScale({HOTBAR_SCALE, HOTBAR_SCALE});
    hotbarBg.setPosition({startX, startY});
    window.draw(hotbarBg);

    // 2. Рисуем рамку выбора активного слота (24x24 пикселя)
    int selIdx = inventory.getSelectedHotbarIndex();
    // Каждый слот смещает рамку на 20 пикселей. Сама рамка шире слота на 1px влево/вверх.
    float selX = startX + (selIdx * 20.0f - 1.0f) * HOTBAR_SCALE;
    float selY = startY - 1.0f * HOTBAR_SCALE;
    
    sf::Sprite selectedSlot(m_widgetsTex, sf::IntRect({0, 22}, {24, 24}));
    selectedSlot.setScale({HOTBAR_SCALE, HOTBAR_SCALE});
    selectedSlot.setPosition({selX, selY});
    window.draw(selectedSlot);

    // Подгружаем атлас предметов, если нужно
    if (!m_itemAtlasLoaded) {
        m_itemAtlasTex.loadFromImage(texMgr.getImage());
        m_itemAtlasLoaded = true;
    }

    // 3. Рисуем предметы в хотбаре
    for (int i = 0; i < Inventory::HOTBAR_SIZE; i++) {
        const ItemStack& stack = inventory.getHotbarSlot(i);
        if (!stack.isEmpty()) {
            // Иконка предмета отступает на 3px от края хотбара, шаг между слотами — 20px
            float ix = startX + (3.0f + i * 20.0f) * HOTBAR_SCALE;
            float iy = startY + 3.0f * HOTBAR_SCALE;
            drawItemStack(window, stack, texMgr, itemReg, {ix, iy}, HOTBAR_SCALE);
        }
    }
}

void InventoryUI::renderInventory(sf::RenderWindow& window, const Inventory& inventory,
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

    // Рисуем задний фон инвентаря
    sf::Sprite invBg(m_inventoryTex, sf::IntRect({0, 0}, {176, 166}));
    invBg.setScale({INV_SCALE, INV_SCALE});
    invBg.setPosition({panelX, panelY});
    window.draw(invBg);

    // В Майнкрафте размер иконки внутри слота — 16x16 пикселей
    const float ITEM_SIZE_PX = 16.0f; 
    const float SLOT_SIZE_PX = 18.0f; // Шаг сетки слотов

    if (!m_itemAtlasLoaded) {
        m_itemAtlasTex.loadFromImage(texMgr.getImage());
        m_itemAtlasLoaded = true;
    }

    // 1. ОСНОВНОЙ ИНВЕНТАРЬ (3 строки по 9 слотов). В текстуре они начинаются с x=8, y=84
    float mainStartX = 8.0f;
    float mainStartY = 84.0f;

    for (int row = 0; row < Inventory::MAIN_ROWS; row++) {
        for (int col = 0; col < Inventory::MAIN_COLS; col++) {
            int slotIndex = Inventory::HOTBAR_SIZE + row * Inventory::MAIN_COLS + col;

            // Экранные координаты самого слота (для кликов мыши)
            float sx = panelX + (mainStartX + col * SLOT_SIZE_PX) * INV_SCALE;
            float sy = panelY + (mainStartY + row * SLOT_SIZE_PX) * INV_SCALE;
            float sSize = SLOT_SIZE_PX * INV_SCALE;

            SlotRect sr;
            sr.bounds = sf::FloatRect({sx, sy}, {sSize, sSize});
            sr.slotIndex = slotIndex;
            m_slotRects.push_back(sr);

            // Отрисовка предмета (смещаем на 1px внутрь слота, так как иконка 16x16, а слот 18x18)
            const ItemStack& stack = inventory.getSlot(slotIndex);
            if (!stack.isEmpty()) {
                float itemX = sx + 1.0f * INV_SCALE;
                float itemY = sy + 1.0f * INV_SCALE;
                drawItemStack(window, stack, texMgr, itemReg, {itemX, itemY}, INV_SCALE);
            }
        }
    }

    // 2. ХОТБАР ВНУТРИ ИНВЕНТАРЯ (9 слотов внизу). В текстуре начинается с x=8, y=142
    float hotbarStartX = 8.0f;
    float hotbarStartY = 142.0f;

    for (int i = 0; i < Inventory::HOTBAR_SIZE; i++) {
        int slotIndex = i;

        float sx = panelX + (hotbarStartX + i * SLOT_SIZE_PX) * INV_SCALE;
        float sy = panelY + hotbarStartY * INV_SCALE;
        float sSize = SLOT_SIZE_PX * INV_SCALE;

        SlotRect sr;
        sr.bounds = sf::FloatRect({sx, sy}, {sSize, sSize});
        sr.slotIndex = slotIndex;
        m_slotRects.push_back(sr);

        const ItemStack& stack = inventory.getSlot(slotIndex);
        if (!stack.isEmpty()) {
            float itemX = sx + 1.0f * INV_SCALE;
            float itemY = sy + 1.0f * INV_SCALE;
            drawItemStack(window, stack, texMgr, itemReg, {itemX, itemY}, INV_SCALE);
        }
    }

    // 3. ПРЕДМЕТ В ВОЗДУХЕ (Перетаскивание мышкой)
    const Inventory::DragState& dragState = inventory.getDragState();
    if (dragState.active && !dragState.stack.isEmpty()) {
        float dragOffset = (ITEM_SIZE_PX * INV_SCALE) / 2.0f;
        drawItemStack(window, dragState.stack, texMgr, itemReg,
                      {static_cast<float>(mousePos.x) - dragOffset, static_cast<float>(mousePos.y) - dragOffset}, INV_SCALE);
    }
}

void InventoryUI::drawSlotBg(sf::RenderWindow& window, const sf::Vector2f& pos, float size, bool selected) {
    // Метод пустой, так как сетка уже идеально нарисована на самой текстуре inventory.png
}

void InventoryUI::drawItemStack(sf::RenderWindow& window, const ItemStack& stack,
                                const TextureManager& texMgr, const ItemRegistry& itemReg,
                                const sf::Vector2f& pos, float scale) {
    if (stack.isEmpty()) return;

    const Item& item = itemReg.getItem(stack.type);
    unsigned int texIndex = texMgr.getTileIndex(item.textureName);
    auto uvs = texMgr.getUV(texIndex);

    // Предмет имеет чистый размер 16x16 пикселей в масштабе
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
    states.texture = &m_itemAtlasTex;
    window.draw(quad, states);

    // Отрисовка количества предметов стака
    if (stack.count > 1 && m_fontLoaded) {
        sf::Text countText(m_font);
        countText.setString(std::to_string(stack.count));
        
        // Масштабируем размер шрифта аккуратно
        countText.setCharacterSize(static_cast<unsigned int>(10 * scale));
        countText.setFillColor(sf::Color::White);
        countText.setOutlineColor(sf::Color(62, 62, 62)); // Темно-серый майнкрафтовский аутлайн
        countText.setOutlineThickness(1.0f);
        
        // Позиционируем в правый нижний угол слота
        countText.setPosition({pos.x + drawSize - 11 * scale, pos.y + drawSize - 12 * scale});
        window.draw(countText);
    }
}

bool InventoryUI::handleMouseInput(const sf::Vector2i& mousePos, bool pressed, bool released,
                                   Inventory& inventory, unsigned int windowWidth, unsigned int windowHeight) {
    if (!m_inventoryOpen || !m_texturesLoaded) return false;

    Inventory::DragState& dragState = const_cast<Inventory::DragState&>(inventory.getDragState());

    int hoveredSlot = -1;
    sf::Vector2f mf(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    for (const auto& sr : m_slotRects) {
        if (sr.bounds.contains(mf)) {
            hoveredSlot = sr.slotIndex;
            break;
        }
    }

    if (pressed && hoveredSlot >= 0) {
        if (!dragState.active) {
            ItemStack& slot = inventory.getSlot(hoveredSlot);
            if (!slot.isEmpty()) {
                dragState.active = true;
                dragState.stack = slot;
                dragState.sourceIndex = hoveredSlot;
                slot = ItemStack();
            }
        } else {
            ItemStack& slot = inventory.getSlot(hoveredSlot);
            if (slot.isEmpty()) {
                slot = dragState.stack;
                dragState.active = false;
                dragState.stack = ItemStack();
                dragState.sourceIndex = -1;
            } else if (slot.canStackWith(dragState.stack)) {
                int space = slot.getFreeSpace();
                int move = std::min(dragState.stack.count, space);
                slot.add(move);
                dragState.stack.remove(move);
                if (dragState.stack.isEmpty()) {
                    dragState.active = false;
                    dragState.sourceIndex = -1;
                }
            } else {
                std::swap(slot, dragState.stack);
                dragState.sourceIndex = hoveredSlot;
            }
        }
        return true;
    }

    if (released && dragState.active) {
        if (hoveredSlot >= 0) {
            ItemStack& slot = inventory.getSlot(hoveredSlot);
            if (slot.isEmpty()) {
                slot = dragState.stack;
            } else if (slot.canStackWith(dragState.stack)) {
                int space = slot.getFreeSpace();
                int move = std::min(dragState.stack.count, space);
                slot.add(move);
                dragState.stack.remove(move);
                if (!dragState.stack.isEmpty() && dragState.sourceIndex >= 0) {
                    inventory.getSlot(dragState.sourceIndex) = dragState.stack;
                }
            } else {
                std::swap(slot, dragState.stack);
                if (!dragState.stack.isEmpty() && dragState.sourceIndex >= 0) {
                    inventory.getSlot(dragState.sourceIndex) = dragState.stack;
                }
            }
        } else if (dragState.sourceIndex >= 0) {
            inventory.getSlot(dragState.sourceIndex) = dragState.stack;
        }
        dragState.active = false;
        dragState.stack = ItemStack();
        dragState.sourceIndex = -1;
        return true;
    }

    return false;
}