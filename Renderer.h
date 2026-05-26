#ifndef RENDERER_H
#define RENDERER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "gameDeclaration.h"
#include "AssetManager.h"
#include "UserManager.h"

struct FloatingText {
    sf::Text text;
    float lifetime; // in seconds
    float maxLifetime;
    sf::Vector2f velocity;
};

enum class GameState {
    LOGIN,
    MENU,
    PLAYING,
    PAUSED,
    GAME_OVER
};

class Renderer {
public:
    Renderer(sf::RenderWindow &window, AssetManager &assets, UserManager &userManager);

    void render(Game &game, Piece *nextPiece, GameState state);
    void addPopup(std::string content, sf::Vector2f position, sf::Color color = sf::Color::Yellow);
    void toggleAbout() { showAbout = !showAbout; }
    void setMenuSelection(int index) { menuSelection = index; }
    int getMenuSelection() const { return menuSelection; }

    // Login Input handling
    void handleTextInput(uint32_t unicode);
    void switchLoginField() { activeLoginField = (activeLoginField + 1) % 2; }
    std::string getLoginPseudo() const { return loginPseudo; }
    std::string getLoginPassword() const { return loginPassword; }
    void clearLoginFields() { loginPseudo = ""; loginPassword = ""; }

private:
    sf::RenderWindow &window;
    AssetManager &assets;
    UserManager &userManager;

    bool showAbout = false;
    int menuSelection = 0; // 0: Play, 1: About, 2: Exit
    
    // Login UI State
    std::string loginPseudo;
    std::string loginPassword;
    int activeLoginField = 0; // 0: Pseudo, 1: Password
    float cursorBlinkTimer = 0;

    void drawLoginScreen();
    void drawMainMenu();
    void drawGameOver(int finalScore);
    void drawPiece(Piece &piece, float x, float y, float scale = 1.0f);
    void drawAboutPage();
    void drawLeaderboard(float x, float y);
    std::string getTextureName(T_Color color, T_Shape shape);

    sf::Clock clock;
    std::vector<FloatingText> popups;
};

#endif
