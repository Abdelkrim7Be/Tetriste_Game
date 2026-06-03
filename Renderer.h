#ifndef RENDERER_H
#define RENDERER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "gameDeclaration.h"
#include "AssetManager.h"
#include "UserManager.h"
#include "UITheme.hpp"

struct FloatingText {
    sf::Text text;
    float lifetime; // in seconds
    float maxLifetime;
    sf::Vector2f velocity;
};

enum class GameState {
    LOGIN,
    MENU,
    DIFFICULTY_SELECT,
    PLAYING,
    PAUSED,
    GAME_OVER
};

enum class AuthState {
    PSEUDO,
    PIN_ENTRY,
    PIN_SETUP,
    AVATAR_SELECT
};

enum class Difficulty {
    RECRUIT,
    VETERAN,
    ELITE
};

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    bool unlocked = false;
};

class Renderer {
public:
    Renderer(sf::RenderWindow &window, AssetManager &assets, UserManager &userManager);

    void render(Game &game, Piece *nextPiece, GameState state);
    void addPopup(std::string content, sf::Vector2f position, sf::Color color = sf::Color::Yellow);
    void toggleAbout() { showAbout = !showAbout; }
    void setMenuSelection(int index) { menuSelection = index; }
    int getMenuSelection() const { return menuSelection; }
    void triggerInsertionEffect() { insertionTimer = 0.4f; }
    void triggerFlash() { flashTimer = 0.2f; }

    // Auth & Difficulty Input
    void handleTextInput(uint32_t unicode);
    std::string getLoginPseudo() const { return loginPseudo; }
    std::string getLoginPin() const { return loginPin; }
    int getAvatarIndex() const { return avatarIndex; }
    void setAvatarIndex(int idx) { avatarIndex = idx; }
    void clearLoginField() { loginPseudo = ""; loginPin = ""; }
    
    AuthState getAuthState() const { return authState; }
    void setAuthState(AuthState s) { authState = s; }
    
    Difficulty getDifficulty() const { return difficulty; }
    void setDifficulty(Difficulty d) { difficulty = d; }

private:
    sf::RenderWindow &window;
    AssetManager &assets;
    UserManager &userManager;

    bool showAbout = false;
    int menuSelection = 0; // 0: Play, 1: About, 2: Exit
    Difficulty difficulty = Difficulty::RECRUIT;
    
    // UI Helpers
    void drawCard(const std::string& title, sf::Vector2f pos, sf::Vector2f size);
    void drawFancyPiece(Piece &piece, float x, float y, float scale = 1.0f, bool highlight = false);
    void drawGlassPanel(sf::Vector2f size, sf::Vector2f pos);

    // Login UI State
    std::string loginPseudo;
    std::string loginPin;
    AuthState authState = AuthState::PSEUDO;
    int avatarIndex = 0;
    float cursorBlinkTimer = 0;

    void drawLoginScreen();
    void drawAvatarSelection();
    void drawDifficultySelection();
    void drawMainMenu();
    void drawGameOver(int finalScore);
    void drawPiece(Piece &piece, float x, float y, float scale = 1.0f);
    void drawAboutPage();
    void drawLeaderboard(float x, float y);
    void drawStats(float x, float y);
    std::string getTextureName(T_Color color, T_Shape shape);

    sf::Clock clock;
    float totalTime = 0.0f;
    float insertionTimer = 0.0f;
    float flashTimer = 0.0f;
    std::vector<FloatingText> popups;
};

#endif
