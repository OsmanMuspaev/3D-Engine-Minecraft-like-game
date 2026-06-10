#include "MenuManager.h"
#include "MenuWidget.h"
#include "../core/AssetPath.h"
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace fs = std::filesystem;

static const float BTN_WIDTH = 300.0f;
static const float BTN_HEIGHT = 44.0f;
static const float BTN_SPACING = 12.0f;

// Исправлено: Спрайты явно инициализируются дефолтным конструктором в списке инициализации
MenuManager::MenuManager() {}

// Исправлено: Добавлен аргумент Game* game
void MenuManager::init(Game* game, unsigned int windowWidth, unsigned int windowHeight) {
    m_game = game;
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;

    loadResources();
    createButtons();
    refreshWorldList();

    float cx = m_windowWidth / 2.0f;
    m_worldNameInput.emplace("World Name", m_font, cx - 150.0f, m_windowHeight / 2.0f - 30.0f, 300.0f);
}

void MenuManager::loadResources() {
    m_fontLoaded = m_font.openFromFile(AssetPath::resolve("minecraft-rus-regular1.ttf"));
    if (!m_fontLoaded) {
        m_fontLoaded = m_font.openFromFile("/System/Library/Fonts/Helvetica.ttc");
    }

    if (m_dirtTexture.loadFromFile(AssetPath::resolve("minecraft/textures/gui/options_background.png")) &&
        m_guiTexture.loadFromFile(AssetPath::resolve("minecraft/textures/gui/widgets.png"))) {

        m_texturesLoaded = true;

        m_dirtTexture.setSmooth(false);
        m_guiTexture.setSmooth(false);

        m_dirtTexture.setRepeated(true);
        m_bgSprite.emplace(m_dirtTexture);

        sf::Vector2i bgSize(static_cast<int>(m_windowWidth / 2), static_cast<int>(m_windowHeight / 2));
        m_bgSprite->setTextureRect(sf::IntRect({0, 0}, bgSize));
        m_bgSprite->setScale({2.0f, 2.0f});

        m_bgSprite->setColor(sf::Color(90, 90, 90));

        m_btnSprite.emplace(m_guiTexture);
    } else {
        std::cerr << "Ошибка: Не удалось загрузить текстуры меню!" << std::endl;
    }
}

void MenuManager::createButtons() {
    float cx = m_windowWidth / 2.0f;

    m_mainMenuButtons.clear();
    m_singleplayerMenuButtons.clear();

    float startY = m_windowHeight / 2.0f - 40.0f;
    m_mainMenuButtons.push_back({"Singleplayer", {cx - BTN_WIDTH / 2.0f, startY}, {BTN_WIDTH, BTN_HEIGHT}, 1});
    m_mainMenuButtons.push_back({"Multiplayer", {cx - BTN_WIDTH / 2.0f, startY + BTN_HEIGHT + BTN_SPACING}, {BTN_WIDTH, BTN_HEIGHT}, 2});
    m_mainMenuButtons.push_back({"Settings", {cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING) * 2}, {BTN_WIDTH, BTN_HEIGHT}, 3});
    m_mainMenuButtons.push_back({"Quit", {cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING) * 3}, {BTN_WIDTH, BTN_HEIGHT}, 4});
}

void MenuManager::refreshWorldList() {
    m_worldList.clear();

    if (!fs::exists("saves")) {
        fs::create_directory("saves");
    }

    for (const auto& entry : fs::directory_iterator("saves")) {
        if (entry.is_directory()) {
            m_worldList.push_back(entry.path().filename().string());
        }
    }
}

void MenuManager::setState(MenuState newState) {
    m_state = newState;
    if (m_state == MenuState::SingleplayerMenu) {
        refreshWorldList();
    }
}

void MenuManager::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (!m_fontLoaded) return;

    if (m_showCreateDialog && m_worldNameInput) {
        m_worldNameInput->handleEvent(event);
        if (const auto* btnEvt = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (btnEvt->button == sf::Mouse::Button::Left) {
                m_mouseClicked = true;
                sf::Vector2f mp = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                float cx = m_windowWidth / 2.0f;
                float cy = m_windowHeight / 2.0f;
                float btnY = cy + 100.0f - 55.0f;
                float btnW = 120.0f;
                float gap = 20.0f;
                sf::FloatRect createBounds({cx - btnW - gap / 2.0f, btnY}, {btnW, BTN_HEIGHT});
                sf::FloatRect cancelBounds({cx + gap / 2.0f, btnY}, {btnW, BTN_HEIGHT});
                if (createBounds.contains(mp) && !m_worldNameInput->text.empty()) {
                    m_newWorldName = m_worldNameInput->text;
                    m_showCreateDialog = false;
                    m_wantsCreateWorld = true;
                    setState(MenuState::Playing);
                } else if (cancelBounds.contains(mp)) {
                    m_showCreateDialog = false;
                }
            }
        }
        if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
            if (key->code == sf::Keyboard::Key::Enter && !m_worldNameInput->text.empty()) {
                m_newWorldName = m_worldNameInput->text;
                m_showCreateDialog = false;
                m_wantsCreateWorld = true;
                setState(MenuState::Playing);
            }
            if (key->code == sf::Keyboard::Key::Escape) {
                m_showCreateDialog = false;
            }
        }
        return;
    }

    if (const auto* btn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (btn->button == sf::Mouse::Button::Left) {
            m_mouseClicked = true;

            if (m_state == MenuState::SingleplayerMenu) {
                sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
                float cx = m_windowWidth / 2.0f;
                float listX = cx - 250.0f;
                float listY = 80.0f;
                float listW = 500.0f;
                float listH = m_windowHeight - 200.0f;

                float entryY = listY + 8.0f;
                float entryH = 40.0f;

                for (size_t i = 0; i < m_worldList.size() && (entryY + entryH < listY + listH); ++i) {
                    sf::FloatRect entryBounds({listX + 6.0f, entryY}, {listW - 12.0f, entryH});
                    if (entryBounds.contains(mousePos)) {
                        m_selectedWorld = m_worldList[i];
                        break;
                    }
                    entryY += entryH + 4.0f;
                }
            }
        }
    }

    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            if (m_state == MenuState::SingleplayerMenu ||
                m_state == MenuState::MultiplayerMenu ||
                m_state == MenuState::SettingsMenu) {
                
                setState(MenuState::MainMenu);
            } else if (m_state == MenuState::NetworkMenu) {
                setState(MenuState::PauseMenu);
            }
        }
    }
}

void MenuManager::update(float dt, const sf::RenderWindow& window) {
    // Логика обновления
}

void MenuManager::draw(sf::RenderWindow& window) {
    if (!m_fontLoaded) return;

    sf::Vector2u actualSize = window.getSize();
    m_windowWidth = actualSize.x;
    m_windowHeight = actualSize.y;

    sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

    if (m_texturesLoaded) {
        window.draw(*m_bgSprite);
    } else {
        window.clear(sf::Color(30, 30, 30));
    }

    switch (m_state) {
        case MenuState::MainMenu:
            drawMainMenu(window, mousePos);
            break;
        case MenuState::SingleplayerMenu:
            drawSingleplayerMenu(window, mousePos);
            break;
        case MenuState::PauseMenu:
            drawPauseMenu(window, mousePos);
            break;
        case MenuState::NetworkMenu:
            drawNetworkMenu(window, mousePos);
            break;
        default:
            break;
    }

    if (m_showCreateDialog) {
        drawCreateDialog(window, mousePos);
    }

    m_mouseClicked = false;
}

// Исправлено: Сигнатура полностью соответствует const MenuButton& btn в .h файле
void MenuManager::drawMinecraftButton(sf::RenderWindow& window, const MenuButton& btn, const sf::Vector2f& mousePos) {
    sf::FloatRect bounds(btn.position, btn.size);
    bool isHovered = bounds.contains(mousePos);

    if (m_texturesLoaded) {
        sf::IntRect srcRect = isHovered ? sf::IntRect({0, 86}, {200, 20}) : sf::IntRect({0, 66}, {200, 20});

        m_btnSprite->setTextureRect(srcRect);
        m_btnSprite->setPosition(btn.position);
        m_btnSprite->setScale({btn.size.x / 200.0f, btn.size.y / 20.0f});

        window.draw(*m_btnSprite);
    }

    // Исправлено под SFML 3: Шрифт m_font передан первым аргументом конструктора Text
    sf::Text text(m_font, btn.text, 18);
    text.setFillColor(isHovered ? sf::Color(255, 255, 160) : sf::Color(220, 220, 220));

    sf::Text shadow = text;
    shadow.setFillColor(sf::Color(40, 40, 40));

    sf::FloatRect textRect = text.getLocalBounds();
    sf::Vector2f origin = {textRect.position.x + textRect.size.x / 2.0f, textRect.position.y + textRect.size.y / 2.0f};
    sf::Vector2f pos = {btn.position.x + btn.size.x / 2.0f, btn.position.y + btn.size.y / 2.0f};

    text.setOrigin(origin);
    text.setPosition(pos);

    shadow.setOrigin(origin);
    // Исправлено под SFML 3: Смена позиции через вектор {x, y}
    shadow.setPosition({pos.x + 2.0f, pos.y + 2.0f});

    window.draw(shadow);
    window.draw(text);

    if (isHovered && m_mouseClicked) {
        switch (btn.actionId) {
            case 1: setState(MenuState::SingleplayerMenu); break;
            case 2: setState(MenuState::MultiplayerMenu); break;
            case 3: setState(MenuState::SettingsMenu); break;
            case 4: m_wantsQuit = true; break;
            case 10:
                if (!m_selectedWorld.empty()) {
                    m_wantsLoadWorld = true;
                    setState(MenuState::Playing);
                }
                break;
            case 11: setState(MenuState::MainMenu); break;
            case 12: {
                m_worldNameInput->text.clear();
                m_worldNameInput->value.setString("");
                m_worldNameInput->setFocus(true);
                m_showCreateDialog = true;
                break;
            }
            case 13:
                if (!m_selectedWorld.empty() && fs::exists("saves/" + m_selectedWorld)) {
                    fs::remove_all("saves/" + m_selectedWorld);
                    m_selectedWorld.clear();
                    refreshWorldList();
                }
                break;
            case 20: m_wantsResume = true; setState(MenuState::Playing); break;
            case 21: m_wantsQuitToMenu = true; setState(MenuState::MainMenu); break;
            case 30: m_wantsOpenToLAN = true; break;
            case 31: m_wantsOpenToTunnel = true; break;
            case 32: setState(MenuState::NetworkMenu); break;
            case 33: setState(MenuState::PauseMenu); break;
            case 14:
                if (m_worldNameInput && !m_worldNameInput->text.empty()) {
                    m_newWorldName = m_worldNameInput->text;
                    m_showCreateDialog = false;
                    m_wantsCreateWorld = true;
                    setState(MenuState::Playing);
                }
                break;
            case 15:
                m_showCreateDialog = false;
                break;
        }
    }
}

void MenuManager::showServerInfo(const std::string& info) {
    m_serverInfoText = info;
}

void MenuManager::setServerStatus(bool running, unsigned short port, const std::string& tunnelUrl, bool tunnelRunning, const std::string& lanAddress) {
    m_serverRunning = running;
    m_serverPort = port;
    m_tunnelUrl = tunnelUrl;
    m_tunnelRunning = tunnelRunning;
    m_lanAddress = lanAddress;
}

void MenuManager::drawMainMenu(sf::RenderWindow& window, const sf::Vector2f& mousePos) {
    // Исправлено под SFML 3: Шрифт m_font на первом месте
    sf::Text title(m_font, "Quadro", 72);
    title.setFillColor(sf::Color::White);

    sf::Text titleShadow = title;
    titleShadow.setFillColor(sf::Color(50, 50, 50));

    sf::FloatRect lb = title.getLocalBounds();
    sf::Vector2f titlePos = {m_windowWidth / 2.0f, (m_windowHeight / 2.0f) - 130.0f};

    title.setOrigin({lb.position.x + lb.size.x / 2.0f, lb.position.y + lb.size.y / 2.0f});
    title.setPosition(titlePos);
    
    titleShadow.setOrigin(title.getOrigin());
    // Исправлено под SFML 3: Передача позиции через вектор
    titleShadow.setPosition({titlePos.x + 4.0f, titlePos.y + 4.0f});

    window.draw(titleShadow);
    window.draw(title);

    for (const auto& btn : m_mainMenuButtons) {
        drawMinecraftButton(window, btn, mousePos);
    }
}

void MenuManager::drawSingleplayerMenu(sf::RenderWindow& window, const sf::Vector2f& mousePos) {
    float cx = m_windowWidth / 2.0f;

    sf::Text title(m_font, "Select World", 24);
    title.setFillColor(sf::Color::White);
    sf::FloatRect lb = title.getLocalBounds();
    title.setOrigin({lb.position.x + lb.size.x / 2.0f, 0.0f});
    title.setPosition({cx, 30.0f});
    window.draw(title);

    float listX = cx - 250.0f;
    float listY = 80.0f;
    float listW = 500.0f;
    float listH = m_windowHeight - 240.0f;

    sf::RectangleShape listBg(sf::Vector2f(listW, listH));
    listBg.setPosition({listX, listY});
    listBg.setFillColor(sf::Color(16, 16, 16));
    listBg.setOutlineColor(sf::Color(128, 128, 128));
    listBg.setOutlineThickness(2.0f);
    window.draw(listBg);

    float entryY = listY + 8.0f;
    float entryH = 40.0f;

    for (size_t i = 0; i < m_worldList.size() && (entryY + entryH < listY + listH); ++i) {
        sf::FloatRect entryBounds({listX + 6.0f, entryY}, {listW - 12.0f, entryH});
        bool isSelected = (m_worldList[i] == m_selectedWorld);
        bool isHovered = entryBounds.contains(mousePos);

        if (isSelected) {
            sf::RectangleShape selectionFrame(sf::Vector2f(listW - 12.0f, entryH));
            selectionFrame.setPosition({listX + 6.0f, entryY});
            selectionFrame.setFillColor(sf::Color(32, 32, 32));
            selectionFrame.setOutlineColor(sf::Color::White);
            selectionFrame.setOutlineThickness(1.0f);
            window.draw(selectionFrame);
        } else if (isHovered) {
            sf::RectangleShape hoverFrame(sf::Vector2f(listW - 12.0f, entryH));
            hoverFrame.setPosition({listX + 6.0f, entryY});
            hoverFrame.setFillColor(sf::Color(24, 24, 24));
            window.draw(hoverFrame);
        }
        sf::Text entryText(m_font, m_worldList[i], 18);
        entryText.setFillColor(sf::Color::White);
        entryText.setPosition({listX + 16.0f, entryY + 8.0f});
        window.draw(entryText);
        entryY += entryH + 4.0f;
    }
    if (m_worldList.empty()) {
        sf::Text emptyText(m_font, "No worlds found!", 18);
        emptyText.setFillColor(sf::Color(128, 128, 128));
        sf::FloatRect elb = emptyText.getLocalBounds();
        emptyText.setOrigin({elb.position.x + elb.size.x / 2.0f, elb.position.y + elb.size.y / 2.0f});
        emptyText.setPosition({cx, listY + listH / 2.0f});
        window.draw(emptyText);
    }

    float bottomY = m_windowHeight - 60.0f;
    float btnW = 140.0f;
    float gap = 10.0f;
    float totalW = btnW * 4 + gap * 3;
    float startX = cx - totalW / 2.0f;

    MenuButton playBtn   {"Play",          {startX, bottomY}, {btnW, BTN_HEIGHT}, 10};
    MenuButton createBtn {"Create World",  {startX + btnW + gap, bottomY}, {btnW, BTN_HEIGHT}, 12};
    MenuButton deleteBtn {"Delete World",  {startX + (btnW + gap) * 2, bottomY}, {btnW, BTN_HEIGHT}, 13};
    MenuButton cancelBtn {"Cancel",        {startX + (btnW + gap) * 3, bottomY}, {btnW, BTN_HEIGHT}, 11};

    drawMinecraftButton(window, playBtn, mousePos);
    drawMinecraftButton(window, createBtn, mousePos);
    drawMinecraftButton(window, deleteBtn, mousePos);
    drawMinecraftButton(window, cancelBtn, mousePos);
}

void MenuManager::drawPauseMenu(sf::RenderWindow& window, const sf::Vector2f& mousePos) {
    float cx = m_windowWidth / 2.0f;

    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight)));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    window.draw(overlay);

    sf::Text title(m_font, "Game Paused", 32);
    title.setFillColor(sf::Color::White);
    sf::FloatRect lb = title.getLocalBounds();
    title.setOrigin({lb.position.x + lb.size.x / 2.0f, lb.position.y + lb.size.y / 2.0f});
    title.setPosition({cx, m_windowHeight / 2.0f - 120.0f});
    window.draw(title);

    float startY = m_windowHeight / 2.0f - 60.0f;
    MenuButton resumeBtn   {"Resume",          {cx - BTN_WIDTH / 2.0f, startY}, {BTN_WIDTH, BTN_HEIGHT}, 20};
    MenuButton settingsBtn {"Settings",        {cx - BTN_WIDTH / 2.0f, startY + BTN_HEIGHT + BTN_SPACING}, {BTN_WIDTH, BTN_HEIGHT}, 3};
    MenuButton networkBtn  {"Open to Network", {cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING) * 2}, {BTN_WIDTH, BTN_HEIGHT}, 32};
    MenuButton quitBtn     {"Quit to Menu",    {cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING) * 3}, {BTN_WIDTH, BTN_HEIGHT}, 21};

    drawMinecraftButton(window, resumeBtn, mousePos);
    drawMinecraftButton(window, settingsBtn, mousePos);
    drawMinecraftButton(window, networkBtn, mousePos);
    drawMinecraftButton(window, quitBtn, mousePos);
}

void MenuManager::drawNetworkMenu(sf::RenderWindow& window, const sf::Vector2f& mousePos) {
    float cx = m_windowWidth / 2.0f;

    sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight)));
    overlay.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(overlay);

    sf::Text title(m_font, "Open to Network", 32);
    title.setFillColor(sf::Color::White);
    sf::FloatRect lb = title.getLocalBounds();
    title.setOrigin({lb.position.x + lb.size.x / 2.0f, lb.position.y + lb.size.y / 2.0f});
    title.setPosition({cx, m_windowHeight / 2.0f - 120.0f});
    window.draw(title);

    float startY = m_windowHeight / 2.0f - 50.0f;
    float halfW = BTN_WIDTH / 2.0f - 4.0f;
    MenuButton lanBtn    {"Open Locally (LAN)", {cx - BTN_WIDTH / 2.0f, startY}, {halfW, BTN_HEIGHT}, 30};
    MenuButton tunnelBtn {"Open via Tunnel",    {cx + 4.0f, startY}, {halfW, BTN_HEIGHT}, 31};

    drawMinecraftButton(window, lanBtn, mousePos);
    drawMinecraftButton(window, tunnelBtn, mousePos);

    float statusY = startY + BTN_HEIGHT + 16.0f;

    sf::Text lanStatus(m_font,
        m_serverRunning ? ("LAN: " + m_lanAddress + ":" + std::to_string(m_serverPort)) : "LAN not running",
        16);
    lanStatus.setFillColor(m_serverRunning ? sf::Color(80, 220, 80) : sf::Color(180, 180, 180));
    lanStatus.setPosition({cx - BTN_WIDTH / 2.0f, statusY});
    window.draw(lanStatus);

    sf::Text tunnelStatus(m_font,
        m_tunnelRunning ? ("Tunnel: " + m_tunnelUrl) : "Tunnel not running",
        16);
    tunnelStatus.setFillColor(m_tunnelRunning ? sf::Color(80, 220, 80) : sf::Color(180, 180, 180));
    tunnelStatus.setPosition({cx - BTN_WIDTH / 2.0f, statusY + 28.0f});
    window.draw(tunnelStatus);

    MenuButton backBtn {"Back", {cx - BTN_WIDTH / 2.0f, statusY + 80.0f}, {BTN_WIDTH, BTN_HEIGHT}, 33};
    drawMinecraftButton(window, backBtn, mousePos);
}

void MenuManager::drawCreateDialog(sf::RenderWindow& window, const sf::Vector2f& mousePos) {
    float cx = m_windowWidth / 2.0f;
    float cy = m_windowHeight / 2.0f;

    sf::RectangleShape dimmer(sf::Vector2f(static_cast<float>(m_windowWidth), static_cast<float>(m_windowHeight)));
    dimmer.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(dimmer);

    float panelW = 400.0f;
    float panelH = 200.0f;
    sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
    panel.setPosition({cx - panelW / 2.0f, cy - panelH / 2.0f});
    panel.setFillColor(sf::Color(30, 30, 30));
    panel.setOutlineColor(sf::Color(100, 100, 100));
    panel.setOutlineThickness(2.0f);
    window.draw(panel);

    sf::Text title(m_font, "Create New World", 24);
    title.setFillColor(sf::Color::White);
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin({tb.position.x + tb.size.x / 2.0f, tb.position.y + tb.size.y / 2.0f});
    title.setPosition({cx, cy - panelH / 2.0f + 30.0f});
    window.draw(title);

    if (m_worldNameInput) {
        m_worldNameInput->update(1.0f / 60.0f, mousePos, m_mouseClicked);
        m_worldNameInput->draw(window);
    }

    float btnY = cy + panelH / 2.0f - 55.0f;
    float btnW = 120.0f;
    float gap = 20.0f;

    MenuButton createBtn {"Create", {cx - btnW - gap / 2.0f, btnY}, {btnW, BTN_HEIGHT}, 14};
    MenuButton cancelBtn {"Cancel", {cx + gap / 2.0f, btnY}, {btnW, BTN_HEIGHT}, 15};

    drawMinecraftButton(window, createBtn, mousePos);
    drawMinecraftButton(window, cancelBtn, mousePos);
}