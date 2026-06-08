#pragma once
#include <SFML/Graphics.hpp>
#include "MenuTypes.h"
#include "MenuWidget.h"
#include "../core/Camera.h"

class Game;

class MenuManager {
public:
    MenuManager();

    void loadFont();
    void init(Game* game, unsigned int windowWidth, unsigned int windowHeight);

    void handleEvent(const sf::Event& event, sf::RenderWindow& window);
    void update(float dt, sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    MenuState getState() const { return m_state; }
    void setState(MenuState state);

    // Settings
    float getRenderScale() const { return m_renderScale; }
    int getWindowWidth() const { return m_windowWidth; }
    int getWindowHeight() const { return m_windowHeight; }

    // Singleplayer
    const std::string& getSelectedWorld() const { return m_selectedWorld; }
    const std::string& getNewWorldName() const { return m_newWorldName; }
    bool wantsCreateWorld() const { return m_wantsCreateWorld; }
    void consumeCreateWorld() { m_wantsCreateWorld = false; }
    bool wantsLoadWorld() const { return m_wantsLoadWorld; }
    void consumeLoadWorld() { m_wantsLoadWorld = false; }

    // Multiplayer
    const std::string& getServerAddress() const { return m_serverAddress; }
    bool wantsConnect() const { return m_wantsConnect; }
    void consumeConnect() { m_wantsConnect = false; }

    // Open to network
    bool wantsOpenToLAN() const { return m_wantsOpenToLAN; }
    void consumeOpenToLAN() { m_wantsOpenToLAN = false; }
    bool wantsOpenToTunnel() const { return m_wantsOpenToTunnel; }
    void consumeOpenToTunnel() { m_wantsOpenToTunnel = false; }

    // Pause
    bool wantsResume() const { return m_wantsResume; }
    void consumeResume() { m_wantsResume = false; }
    bool wantsQuitToMenu() const { return m_wantsQuitToMenu; }
    void consumeQuitToMenu() { m_wantsQuitToMenu = false; }

    // World settings
    int getWorldSize() const { return m_worldSize; }
    std::string getWorldSeed() const { return m_newWorldName; }

    // True only on the frame a mouse button was first pressed
    bool mouseClicked() const { return m_mouseClicked; }

    // Server info screen
    void showServerInfo(const std::string& info);
    const std::string& getServerInfo() const { return m_serverInfoText; }

private:
    void drawMainMenu(sf::RenderWindow& window);
    void drawSingleplayerMenu(sf::RenderWindow& window);
    void drawMultiplayerMenu(sf::RenderWindow& window);
    void drawSettingsMenu(sf::RenderWindow& window);
    void drawPauseMenu(sf::RenderWindow& window);
    void drawCreateWorldDialog(sf::RenderWindow& window);
    void drawServerInfoScreen(sf::RenderWindow& window);

    void refreshWorldList();

    MenuState m_state = MenuState::MainMenu;
    MenuState m_stateBeforeInfo = MenuState::PauseMenu;
    Game* m_game = nullptr;
    unsigned int m_windowWidth = 1680;
    unsigned int m_windowHeight = 1050;

    sf::Font m_font;
    bool m_fontLoaded = false;

    // Click tracking (true on the single frame a press begins)
    bool m_mouseClicked = false;
    bool m_prevMouseDown = false;

    // Settings
    float m_renderScale = 0.25f;

    // Singleplayer
    std::string m_selectedWorld;
    std::string m_newWorldName;
    bool m_wantsCreateWorld = false;
    bool m_wantsLoadWorld = false;
    bool m_showCreateDialog = false;
    std::vector<std::string> m_worldList;
    int m_worldSize = 20;

    // Persistent TextInput widgets
    TextInput m_nameInput;
    TextInput m_serverInput;

    // Multiplayer
    std::string m_serverAddress;
    bool m_wantsConnect = false;

    // Open to network
    bool m_wantsOpenToLAN = false;
    bool m_wantsOpenToTunnel = false;

    // Pause
    bool m_wantsResume = false;
    bool m_wantsQuitToMenu = false;

    // Server info screen
    std::string m_serverInfoText;

    // UI elements (rebuilt per frame for simplicity)
    sf::RectangleShape m_background;
};
