#pragma once
#include <SFML/Graphics.hpp>
#include "MenuTypes.h"
#include "MenuWidget.h"
#include "../core/Camera.h"
#include <vector>
#include <string>
#include <optional>

class Game;

class MenuManager {
public:
    MenuManager();

    void init(Game* game, unsigned int windowWidth, unsigned int windowHeight);

    void handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void update(float dt, const sf::RenderWindow& window);
    void draw(sf::RenderWindow& window);

    MenuState getState() const { return m_state; }
    void setState(MenuState state);

    float getRenderScale() const { return m_renderScale; }
    int getWindowWidth() const { return m_windowWidth; }
    int getWindowHeight() const { return m_windowHeight; }

    const std::string& getSelectedWorld() const { return m_selectedWorld; }
    const std::string& getNewWorldName() const { return m_newWorldName; }
    bool wantsCreateWorld() const { return m_wantsCreateWorld; }
    void consumeCreateWorld() { m_wantsCreateWorld = false; }
    bool wantsLoadWorld() const { return m_wantsLoadWorld; }
    void consumeLoadWorld() { m_wantsLoadWorld = false; }

    const std::string& getServerAddress() const { return m_serverAddress; }
    bool wantsConnect() const { return m_wantsConnect; }
    void consumeConnect() { m_wantsConnect = false; }

    bool wantsOpenToLAN() const { return m_wantsOpenToLAN; }
    void consumeOpenToLAN() { m_wantsOpenToLAN = false; }
    bool wantsOpenToTunnel() const { return m_wantsOpenToTunnel; }
    void consumeOpenToTunnel() { m_wantsOpenToTunnel = false; }

    bool wantsResume() const { return m_wantsResume; }
    void consumeResume() { m_wantsResume = false; }
    bool wantsQuitToMenu() const { return m_wantsQuitToMenu; }
    void consumeQuitToMenu() { m_wantsQuitToMenu = false; }
    bool wantsQuit() const { return m_wantsQuit; }
    void consumeQuit() { m_wantsQuit = false; }

    int getWorldSize() const { return m_worldSize; }

    bool mouseClicked() const { return m_mouseClicked; }

    void showServerInfo(const std::string& info);
    const std::string& getServerInfo() const { return m_serverInfoText; }

    void setServerStatus(bool running, unsigned short port, const std::string& tunnelUrl, bool tunnelRunning, const std::string& lanAddress = "");
    bool isServerRunning() const { return m_serverRunning; }
    unsigned short getServerPort() const { return m_serverPort; }
    const std::string& getTunnelUrl() const { return m_tunnelUrl; }
    bool isTunnelRunning() const { return m_tunnelRunning; }

private:
    void loadResources();
    void createButtons();
    void refreshWorldList();

    void drawMainMenu(sf::RenderWindow& window, const sf::Vector2f& mousePos);
    void drawSingleplayerMenu(sf::RenderWindow& window, const sf::Vector2f& mousePos);
    void drawMinecraftButton(sf::RenderWindow& window, const MenuButton& btn, const sf::Vector2f& mousePos);
    void drawPauseMenu(sf::RenderWindow& window, const sf::Vector2f& mousePos);
    void drawNetworkMenu(sf::RenderWindow& window, const sf::Vector2f& mousePos);
    void drawCreateDialog(sf::RenderWindow& window, const sf::Vector2f& mousePos);

    MenuState m_state = MenuState::MainMenu;
    Game* m_game = nullptr;
    unsigned int m_windowWidth = 1680;
    unsigned int m_windowHeight = 1050;

    sf::Font m_font;
    bool m_fontLoaded = false;
    bool m_texturesLoaded = false;

    bool m_mouseClicked = false;
    bool m_prevMouseDown = false;

    float m_renderScale = 0.25f;

    sf::Texture m_dirtTexture;
    sf::Texture m_guiTexture;
    std::optional<sf::Sprite> m_bgSprite;
    std::optional<sf::Sprite> m_btnSprite;

    std::vector<MenuButton> m_mainMenuButtons;
    std::vector<MenuButton> m_singleplayerMenuButtons;

    std::string m_selectedWorld;
    std::string m_newWorldName;
    bool m_wantsCreateWorld = false;
    bool m_wantsLoadWorld = false;
    bool m_showCreateDialog = false;
    std::optional<TextInput> m_worldNameInput;
    std::vector<std::string> m_worldList;
    int m_worldSize = 20;

    std::string m_serverAddress;
    bool m_wantsConnect = false;

    bool m_wantsOpenToLAN = false;
    bool m_wantsOpenToTunnel = false;

    bool m_wantsResume = false;
    bool m_wantsQuitToMenu = false;
    bool m_wantsQuit = false;

    std::string m_serverInfoText;

    bool m_serverRunning = false;
    unsigned short m_serverPort = 0;
    std::string m_tunnelUrl;
    bool m_tunnelRunning = false;
    std::string m_lanAddress;

    sf::RectangleShape m_background;
};
