#include "MenuManager.h"
#include "MenuWidget.h"
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

static const sf::Color BG_COLOR(20, 20, 20);
static const sf::Color BTN_COLOR(60, 60, 60);
static const sf::Color BTN_HOVER(80, 80, 80);
static const sf::Color TEXT_COLOR(200, 200, 200);
static const sf::Color TITLE_COLOR(255, 255, 255);
static const float BTN_WIDTH = 300.0f;
static const float BTN_HEIGHT = 50.0f;
static const float BTN_SPACING = 15.0f;
static const float DIALOG_WIDTH = 420.0f;
static const float DIALOG_HEIGHT = 300.0f;

MenuManager::MenuManager()
    : m_nameInput("World Name", m_font, 0, 0, 1),
      m_serverInput("Server Address", m_font, 0, 0, 1)
{
}

void MenuManager::loadFont() {
    m_fontLoaded = m_font.openFromFile("/System/Library/Fonts/Helvetica.ttc");
}

void MenuManager::init(Game* game, unsigned int windowWidth, unsigned int windowHeight) {
    m_game = game;
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;

    m_background.setSize({(float)windowWidth, (float)windowHeight});
    m_background.setFillColor(BG_COLOR);

    loadFont();
    refreshWorldList();
}

void MenuManager::setState(MenuState state) {
    m_state = state;
}

// ---------------------------------------------------------------------------
// Event handling
// ---------------------------------------------------------------------------

void MenuManager::handleEvent(const sf::Event& event, sf::RenderWindow& window) {
    if (!m_fontLoaded) return;

    // Track mouse click (single-frame press)
    if (const auto* btn = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (btn->button == sf::Mouse::Button::Left) {
            m_mouseClicked = true;
        }
    }

    // Global: escape toggles pause / resumes
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::Escape) {
            if (m_state == MenuState::Playing) {
                m_state = MenuState::PauseMenu;
            } else if (m_state == MenuState::PauseMenu) {
                m_wantsResume = true;
                m_state = MenuState::Playing;
            } else if (m_state == MenuState::ServerInfoScreen) {
                m_state = MenuState::PauseMenu;
            }
        }
    }

    // Forward text events to focused TextInput widgets
    if (event.is<sf::Event::TextEntered>()) {
        if (m_state == MenuState::SingleplayerMenu && m_showCreateDialog) {
            m_nameInput.handleEvent(event);
        } else if (m_state == MenuState::MultiplayerMenu) {
            m_serverInput.handleEvent(event);
        }
    }
}

// ---------------------------------------------------------------------------
// Update
// ---------------------------------------------------------------------------

void MenuManager::update(float dt, sf::RenderWindow& window) {
    // Update TextInput cursor blinking
    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
    if (m_state == MenuState::SingleplayerMenu && m_showCreateDialog) {
        m_nameInput.update(dt, mousePos, m_mouseClicked);
    }
    if (m_state == MenuState::MultiplayerMenu) {
        m_serverInput.update(dt, mousePos, m_mouseClicked);
    }
}

// ---------------------------------------------------------------------------
// Draw
// ---------------------------------------------------------------------------

void MenuManager::draw(sf::RenderWindow& window) {
    if (!m_fontLoaded) return;

    window.draw(m_background);

    switch (m_state) {
        case MenuState::MainMenu:        drawMainMenu(window); break;
        case MenuState::SingleplayerMenu: drawSingleplayerMenu(window); break;
        case MenuState::MultiplayerMenu:  drawMultiplayerMenu(window); break;
        case MenuState::SettingsMenu:     drawSettingsMenu(window); break;
        case MenuState::PauseMenu:        drawPauseMenu(window); break;
        case MenuState::ServerInfoScreen: drawServerInfoScreen(window); break;
        case MenuState::Playing: break;
    }

    if (m_showCreateDialog) {
        drawCreateWorldDialog(window);
    }

    // Reset single-frame click flag
    m_mouseClicked = false;
}

// ---------------------------------------------------------------------------
// Main Menu
// ---------------------------------------------------------------------------

void MenuManager::drawMainMenu(sf::RenderWindow& window) {
    float cx = (float)m_windowWidth / 2.0f;
    float cy = (float)m_windowHeight / 2.0f;

    // Title
    Label title("Quadro", m_font, 0, cy - 180.0f, 64);
    auto lb = title.text.getLocalBounds();
    title.text.setOrigin({lb.position.x + lb.size.x / 2.0f, 0.0f});
    title.text.setPosition({cx, cy - 180.0f});
    title.draw(window);

    // Buttons
    float startY = cy - 60.0f;
    Button singleplayer("Singleplayer", m_font, cx - BTN_WIDTH / 2.0f, startY, BTN_WIDTH, BTN_HEIGHT);
    Button multiplayer("Multiplayer", m_font, cx - BTN_WIDTH / 2.0f, startY + BTN_HEIGHT + BTN_SPACING, BTN_WIDTH, BTN_HEIGHT);
    Button settings("Settings", m_font, cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING) * 2, BTN_WIDTH, BTN_HEIGHT);

    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
    singleplayer.update(mousePos);
    multiplayer.update(mousePos);
    settings.update(mousePos);

    singleplayer.draw(window);
    multiplayer.draw(window);
    settings.draw(window);

    if (singleplayer.contains(mousePos) && m_mouseClicked) {
        m_state = MenuState::SingleplayerMenu;
        refreshWorldList();
    }
    if (multiplayer.contains(mousePos) && m_mouseClicked) {
        m_state = MenuState::MultiplayerMenu;
    }
    if (settings.contains(mousePos) && m_mouseClicked) {
        m_state = MenuState::SettingsMenu;
    }
}

// ---------------------------------------------------------------------------
// Singleplayer Menu
// ---------------------------------------------------------------------------

void MenuManager::drawSingleplayerMenu(sf::RenderWindow& window) {
    float cx = (float)m_windowWidth / 2.0f;
    float startY = 80.0f;

    Label title("Singleplayer", m_font, 0, startY, 32);
    auto lb = title.text.getLocalBounds();
    title.text.setOrigin({lb.position.x + lb.size.x / 2.0f, 0.0f});
    title.text.setPosition({cx, startY});
    title.draw(window);

    // World list area
    float listX = cx - 250.0f;
    float listY = startY + 60.0f;
    float listW = 500.0f;
    float listH = 300.0f;

    sf::RectangleShape listBg({listW, listH});
    listBg.setPosition({listX, listY});
    listBg.setFillColor(sf::Color(30, 30, 30));
    listBg.setOutlineColor(sf::Color(80, 80, 80));
    listBg.setOutlineThickness(2.0f);
    window.draw(listBg);

    // Draw world entries
    float entryY = listY + 10.0f;
    float entryH = 36.0f;
    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);

    for (size_t i = 0; i < m_worldList.size() && entryY + entryH < listY + listH - 5.0f; ++i) {
        sf::RectangleShape entryBg({listW - 20.0f, entryH});
        entryBg.setPosition({listX + 10.0f, entryY});
        bool isSelected = (m_worldList[i] == m_selectedWorld);
        entryBg.setFillColor(isSelected ? sf::Color(70, 70, 70) : sf::Color(40, 40, 40));
        entryBg.setOutlineColor(isSelected ? sf::Color(120, 120, 120) : sf::Color(60, 60, 60));
        entryBg.setOutlineThickness(1.0f);
        window.draw(entryBg);

        Label entry(m_worldList[i], m_font, listX + 20.0f, entryY + 8.0f, 16);
        entry.draw(window);

        if (entryBg.getGlobalBounds().contains(mousePos) && m_mouseClicked) {
            m_selectedWorld = m_worldList[i];
        }

        entryY += entryH + 4.0f;
    }

    if (m_worldList.empty()) {
        Label empty("No saved worlds", m_font, listX + 140.0f, listY + 130.0f, 16);
        empty.draw(window);
    }

    // Buttons at bottom
    float btnY = listY + listH + 20.0f;
    Button createBtn("Create New World", m_font, cx - 260.0f, btnY, 170.0f, BTN_HEIGHT);
    Button playBtn("Play", m_font, cx - 75.0f, btnY, 150.0f, BTN_HEIGHT);
    Button backBtn("Back", m_font, cx + 90.0f, btnY, 170.0f, BTN_HEIGHT);

    createBtn.update(mousePos);
    playBtn.update(mousePos);
    backBtn.update(mousePos);

    createBtn.draw(window);
    playBtn.draw(window);
    backBtn.draw(window);

    if (createBtn.contains(mousePos) && m_mouseClicked) {
        m_showCreateDialog = true;
        m_newWorldName.clear();
        m_nameInput.text.clear();
        m_nameInput.value.setString("");
        m_nameInput.setFocus(false);
    }
    if (playBtn.contains(mousePos) && m_mouseClicked) {
        if (!m_selectedWorld.empty()) {
            m_wantsLoadWorld = true;
            m_state = MenuState::Playing;
        }
    }
    if (backBtn.contains(mousePos) && m_mouseClicked) {
        m_state = MenuState::MainMenu;
    }
}

// ---------------------------------------------------------------------------
// Create World Dialog
// ---------------------------------------------------------------------------

void MenuManager::drawCreateWorldDialog(sf::RenderWindow& window) {
    float cx = (float)m_windowWidth / 2.0f;
    float cy = (float)m_windowHeight / 2.0f;
    float dx = cx - DIALOG_WIDTH / 2.0f;
    float dy = cy - DIALOG_HEIGHT / 2.0f;

    // Dim background
    sf::RectangleShape dim({(float)m_windowWidth, (float)m_windowHeight});
    dim.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(dim);

    // Dialog panel
    sf::RectangleShape panel({DIALOG_WIDTH, DIALOG_HEIGHT});
    panel.setPosition({dx, dy});
    panel.setFillColor(sf::Color(35, 35, 35));
    panel.setOutlineColor(sf::Color(100, 100, 100));
    panel.setOutlineThickness(2.0f);
    window.draw(panel);

    Label dialogTitle("Create New World", m_font, dx + 100.0f, dy + 15.0f, 24);
    dialogTitle.draw(window);

    // World name input — update position each frame but keep focus/text state
    float nameX = dx + 30.0f;
    float nameY = dy + 60.0f;
    m_nameInput.box.setPosition({nameX, nameY + 25.0f});
    m_nameInput.box.setSize({DIALOG_WIDTH - 60.0f, 35.0f});
    m_nameInput.label.setPosition({nameX, nameY});
    m_nameInput.value.setPosition({nameX + 8.0f, nameY + 32.0f});
    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
    m_nameInput.update(0.016f, mousePos, m_mouseClicked);
    m_nameInput.draw(window);

    // Sync from TextInput (never overwrite INTO it)
    m_newWorldName = m_nameInput.text;

    // World size buttons
    Label sizeLabel("World Size:", m_font, dx + 30.0f, dy + 130.0f, 16);
    sizeLabel.draw(window);

    float sizeBtnY = dy + 155.0f;
    float sizeBtnW = 100.0f;

    struct SizeOption { const char* label; int value; };
    SizeOption sizes[] = { {"Small (10)", 10}, {"Medium (20)", 20}, {"Large (30)", 30} };

    for (int i = 0; i < 3; ++i) {
        float bx = dx + 30.0f + i * (sizeBtnW + 10.0f);
        Button sizeBtn(sizes[i].label, m_font, bx, sizeBtnY, sizeBtnW, 35.0f);
        if (m_worldSize == sizes[i].value) {
            sizeBtn.shape.setFillColor(sf::Color(100, 100, 100));
        }
        sizeBtn.update(mousePos);
        sizeBtn.draw(window);
        if (sizeBtn.contains(mousePos) && m_mouseClicked) {
            m_worldSize = sizes[i].value;
        }
    }

    // Create / Cancel buttons
    float dlgBtnY = dy + DIALOG_HEIGHT - 60.0f;
    Button createBtn("Create", m_font, dx + 50.0f, dlgBtnY, 140.0f, 40.0f);
    Button cancelBtn("Cancel", m_font, dx + DIALOG_WIDTH - 190.0f, dlgBtnY, 140.0f, 40.0f);

    createBtn.update(mousePos);
    cancelBtn.update(mousePos);

    createBtn.draw(window);
    cancelBtn.draw(window);

    if (createBtn.contains(mousePos) && m_mouseClicked) {
        if (!m_newWorldName.empty()) {
            m_wantsCreateWorld = true;
            m_showCreateDialog = false;
            m_state = MenuState::Playing;
        }
    }
    if (cancelBtn.contains(mousePos) && m_mouseClicked) {
        m_showCreateDialog = false;
    }
}

// ---------------------------------------------------------------------------
// Multiplayer Menu
// ---------------------------------------------------------------------------

void MenuManager::drawMultiplayerMenu(sf::RenderWindow& window) {
    float cx = (float)m_windowWidth / 2.0f;
    float startY = 80.0f;

    Label title("Multiplayer", m_font, 0, startY, 32);
    auto lb = title.text.getLocalBounds();
    title.text.setOrigin({lb.position.x + lb.size.x / 2.0f, 0.0f});
    title.text.setPosition({cx, startY});
    title.draw(window);

    // Set default "localhost" once when entering this menu
    if (m_serverInput.text.empty() && m_serverAddress.empty()) {
        m_serverInput.text = "localhost";
        m_serverInput.value.setString("localhost");
    }

    // Server address input — update position each frame but keep focus/text state
    float srvX = cx - 200.0f;
    float srvY = startY + 80.0f;
    m_serverInput.box.setPosition({srvX, srvY + 25.0f});
    m_serverInput.box.setSize({400.0f, 35.0f});
    m_serverInput.label.setPosition({srvX, srvY});
    m_serverInput.value.setPosition({srvX + 8.0f, srvY + 32.0f});
    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
    m_serverInput.update(0.016f, mousePos, m_mouseClicked);
    m_serverInput.draw(window);

    // Sync from TextInput
    m_serverAddress = m_serverInput.text;

    // Buttons
    float btnY = startY + 180.0f;

    Button connectBtn("Connect", m_font, cx - BTN_WIDTH / 2.0f, btnY, BTN_WIDTH, BTN_HEIGHT);
    Button backBtn("Back", m_font, cx - BTN_WIDTH / 2.0f, btnY + BTN_HEIGHT + BTN_SPACING, BTN_WIDTH, BTN_HEIGHT);

    connectBtn.update(mousePos);
    backBtn.update(mousePos);

    connectBtn.draw(window);
    backBtn.draw(window);

    if (connectBtn.contains(mousePos) && m_mouseClicked) {
        if (!m_serverAddress.empty()) {
            m_wantsConnect = true;
            m_state = MenuState::Playing;
        }
    }
    if (backBtn.contains(mousePos) && m_mouseClicked) {
        m_state = MenuState::MainMenu;
    }
}

// ---------------------------------------------------------------------------
// Settings Menu
// ---------------------------------------------------------------------------

void MenuManager::drawSettingsMenu(sf::RenderWindow& window) {
    float cx = (float)m_windowWidth / 2.0f;
    float startY = 80.0f;

    Label title("Settings", m_font, 0, startY, 32);
    auto lb = title.text.getLocalBounds();
    title.text.setOrigin({lb.position.x + lb.size.x / 2.0f, 0.0f});
    title.text.setPosition({cx, startY});
    title.draw(window);

    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);

    // Resolution section
    Label resLabel("Resolution:", m_font, cx - 200.0f, startY + 70.0f, 18);
    resLabel.draw(window);

    struct Resolution { int w; int h; const char* label; };
    Resolution resolutions[] = {
        {1280, 720, "1280x720"},
        {1680, 1050, "1680x1050"},
        {1920, 1080, "1920x1080"},
        {2560, 1440, "2560x1440"}
    };

    float resBtnY = startY + 100.0f;
    float resBtnW = 120.0f;
    for (int i = 0; i < 4; ++i) {
        float bx = cx - 260.0f + i * (resBtnW + 15.0f);
        Button resBtn(resolutions[i].label, m_font, bx, resBtnY, resBtnW, 35.0f);
        if (m_windowWidth == resolutions[i].w && m_windowHeight == resolutions[i].h) {
            resBtn.shape.setFillColor(sf::Color(100, 100, 100));
        }
        resBtn.update(mousePos);
        resBtn.draw(window);
        if (resBtn.contains(mousePos) && m_mouseClicked) {
            m_windowWidth = resolutions[i].w;
            m_windowHeight = resolutions[i].h;
        }
    }

    // Render scale section
    Label scaleLabel("Render Scale:", m_font, cx - 200.0f, resBtnY + 70.0f, 18);
    scaleLabel.draw(window);

    float scales[] = { 0.25f, 0.5f, 0.75f, 1.0f };
    const char* scaleLabels[] = { "0.25", "0.50", "0.75", "1.00" };
    float scaleBtnY = resBtnY + 100.0f;
    float scaleBtnW = 80.0f;

    for (int i = 0; i < 4; ++i) {
        float bx = cx - 185.0f + i * (scaleBtnW + 15.0f);
        Button scaleBtn(scaleLabels[i], m_font, bx, scaleBtnY, scaleBtnW, 35.0f);
        if (m_renderScale == scales[i]) {
            scaleBtn.shape.setFillColor(sf::Color(100, 100, 100));
        }
        scaleBtn.update(mousePos);
        scaleBtn.draw(window);
        if (scaleBtn.contains(mousePos) && m_mouseClicked) {
            m_renderScale = scales[i];
        }
    }

    // Back button
    Button backBtn("Back", m_font, cx - BTN_WIDTH / 2.0f, scaleBtnY + 90.0f, BTN_WIDTH, BTN_HEIGHT);
    backBtn.update(mousePos);
    backBtn.draw(window);
    if (backBtn.contains(mousePos) && m_mouseClicked) {
        m_state = MenuState::MainMenu;
    }
}

// ---------------------------------------------------------------------------
// Pause Menu
// ---------------------------------------------------------------------------

void MenuManager::drawPauseMenu(sf::RenderWindow& window) {
    float cx = (float)m_windowWidth / 2.0f;
    float cy = (float)m_windowHeight / 2.0f;

    // Semi-transparent overlay
    sf::RectangleShape overlay({(float)m_windowWidth, (float)m_windowHeight});
    overlay.setFillColor(sf::Color(0, 0, 0, 120));
    window.draw(overlay);

    Label title("Paused", m_font, 0, cy - 160.0f, 36);
    auto lb = title.text.getLocalBounds();
    title.text.setOrigin({lb.position.x + lb.size.x / 2.0f, 0.0f});
    title.text.setPosition({cx, cy - 160.0f});
    title.draw(window);

    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
    float startY = cy - 100.0f;

    Button resumeBtn("Resume", m_font, cx - BTN_WIDTH / 2.0f, startY, BTN_WIDTH, BTN_HEIGHT);
    Button lanBtn("Open to LAN", m_font, cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING), BTN_WIDTH, BTN_HEIGHT);
    Button tunnelBtn("Open with LocalTunnel", m_font, cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING) * 2, BTN_WIDTH, BTN_HEIGHT);
    Button settingsBtn("Settings", m_font, cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING) * 3, BTN_WIDTH, BTN_HEIGHT);
    Button quitBtn("Quit to Menu", m_font, cx - BTN_WIDTH / 2.0f, startY + (BTN_HEIGHT + BTN_SPACING) * 4, BTN_WIDTH, BTN_HEIGHT);

    resumeBtn.update(mousePos);
    lanBtn.update(mousePos);
    tunnelBtn.update(mousePos);
    settingsBtn.update(mousePos);
    quitBtn.update(mousePos);

    resumeBtn.draw(window);
    lanBtn.draw(window);
    tunnelBtn.draw(window);
    settingsBtn.draw(window);
    quitBtn.draw(window);

    if (resumeBtn.contains(mousePos) && m_mouseClicked) {
        m_wantsResume = true;
        m_state = MenuState::Playing;
    }
    if (lanBtn.contains(mousePos) && m_mouseClicked) {
        m_wantsOpenToLAN = true;
    }
    if (tunnelBtn.contains(mousePos) && m_mouseClicked) {
        m_wantsOpenToTunnel = true;
    }
    if (settingsBtn.contains(mousePos) && m_mouseClicked) {
        m_state = MenuState::SettingsMenu;
    }
    if (quitBtn.contains(mousePos) && m_mouseClicked) {
        m_wantsQuitToMenu = true;
        m_state = MenuState::MainMenu;
    }
}

// ---------------------------------------------------------------------------
// Refresh saved world list from saves/ directory
// ---------------------------------------------------------------------------

void MenuManager::refreshWorldList() {
    m_worldList.clear();

    fs::path savesDir("saves");
    if (!fs::exists(savesDir) || !fs::is_directory(savesDir)) {
        fs::create_directories(savesDir);
        return;
    }

    for (auto& entry : fs::directory_iterator(savesDir)) {
        if (entry.is_directory()) {
            m_worldList.push_back(entry.path().filename().string());
        }
    }

    std::sort(m_worldList.begin(), m_worldList.end());
}

// ---------------------------------------------------------------------------
// Server Info Screen
// ---------------------------------------------------------------------------

void MenuManager::showServerInfo(const std::string& info) {
    m_serverInfoText = info;
    m_stateBeforeInfo = m_state;
    m_state = MenuState::ServerInfoScreen;
}

void MenuManager::drawServerInfoScreen(sf::RenderWindow& window) {
    float cx = (float)m_windowWidth / 2.0f;
    float cy = (float)m_windowHeight / 2.0f;
    float panelW = 500.0f;
    float panelH = 220.0f;
    float px = cx - panelW / 2.0f;
    float py = cy - panelH / 2.0f;

    // Dim background
    sf::RectangleShape dim({(float)m_windowWidth, (float)m_windowHeight});
    dim.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(dim);

    // Panel
    sf::RectangleShape panel({panelW, panelH});
    panel.setPosition({px, py});
    panel.setFillColor(sf::Color(35, 35, 35));
    panel.setOutlineColor(sf::Color(100, 100, 100));
    panel.setOutlineThickness(2.0f);
    window.draw(panel);

    Label title("Server Started", m_font, px + 140.0f, py + 20.0f, 24);
    title.draw(window);

    // Server info text — wrap long lines manually
    float textY = py + 70.0f;
    const float maxLineW = panelW - 60.0f;

    // Split info text by newlines or auto-wrap
    std::string remaining = m_serverInfoText;
    while (!remaining.empty()) {
        // Find a newline
        auto nl = remaining.find('\n');
        std::string line = (nl != std::string::npos) ? remaining.substr(0, nl) : remaining;

        // If the line is too wide, try to wrap at spaces
        while (line.size() > 40) {
            auto space = line.rfind(' ', 40);
            if (space == std::string::npos) break;
            std::string part = line.substr(0, space);
            Label lbl(part, m_font, px + 30.0f, textY, 16);
            lbl.draw(window);
            textY += 24.0f;
            line = line.substr(space + 1);
        }
        if (!line.empty()) {
            Label lbl(line, m_font, px + 30.0f, textY, 16);
            lbl.draw(window);
            textY += 24.0f;
        }

        if (nl != std::string::npos)
            remaining = remaining.substr(nl + 1);
        else
            remaining.clear();
    }

    // OK button
    sf::Vector2f mousePos = (sf::Vector2f)sf::Mouse::getPosition(window);
    Button okBtn("OK", m_font, cx - 70.0f, py + panelH - 60.0f, 140.0f, 40.0f);
    okBtn.update(mousePos);
    okBtn.draw(window);

    if (okBtn.contains(mousePos) && m_mouseClicked) {
        m_state = MenuState::PauseMenu;
    }
}
