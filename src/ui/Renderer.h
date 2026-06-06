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
    float lifetime;
    float maxLifetime;
    sf::Vector2f velocity;
};

enum class GameState {
    LOGIN,
    MENU,
    DIFFICULTY_SELECT,
    PLAYING,
    PAUSED,
    GAME_OVER,
    ACHIEVEMENTS,
    SETTINGS,
    PROTOCOLS
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

enum class ShellVariant {
    None,
    Minimal,
    Dashboard,
    PlayHud
};

enum class NavItem {
    LOGIC,
    NODES,
    DECRYPT,
    CONFIG
};

enum class HeaderStyle {
    Default,
    Auth
};

struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    bool unlocked = false;

    Achievement(std::string i, std::string n, std::string d, bool u = false)
        : id(i), name(n), description(d), unlocked(u) {}
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
    void resetMatchHighlight() { matchHighlightEnabled = false; }
    void enableMatchHighlight() { matchHighlightEnabled = true; }

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
    bool matchHighlightEnabled = false;
    int menuSelection = 0;
    Difficulty difficulty = Difficulty::RECRUIT;

    std::string loginPseudo;
    std::string loginPin;
    AuthState authState = AuthState::PSEUDO;
    int avatarIndex = 0;
    float cursorBlinkTimer = 0;

    sf::Clock clock;
    float totalTime = 0.0f;
    float insertionTimer = 0.0f;
    float flashTimer = 0.0f;
    std::vector<FloatingText> popups;

    bool hasFont() const;
    sf::Font &font();
    sf::Font &fontBold();
    void drawText(const std::string &str, float x, float y, UI::CodeSize size, sf::Color color, bool bold = false);
    float textWidth(const std::string &str, UI::CodeSize size, bool bold = false);
    std::string nodeIdFromPseudo(const std::string &pseudo) const;

    void drawCard(const std::string& title, sf::Vector2f pos, sf::Vector2f size);
    void drawFancyPiece(Piece &piece, float x, float y, float scale = 1.0f, bool highlight = false, bool backdrop = false);
    void drawContentBackdrop();
    void drawGlassPanel(sf::Vector2f size, sf::Vector2f pos);
    void drawFolderTab(const std::string &label, float panelX, float panelY, float tabW = 140.0f);
    void drawInvertedButton(float x, float y, float w, float h, const std::string &label,
                            const std::string &sub, bool selected);
    void drawAsciiBox(float x, float y);
    void drawBitRateColumn(float x, float y);
    void drawStatusChip(float x, float y, const std::string &text, sf::Color accent);
    void drawFloatingChips(float rightX, float topY);

    void drawBackgroundGrid();
    void drawCornerLogs(bool dashboard = false);
    void drawHeader(HeaderStyle style = HeaderStyle::Default);
    void drawFooter(int activeIndex = 0);
    void drawDashboardSidebar(NavItem active);
    void drawTerminalShell(ShellVariant variant, NavItem nav = NavItem::LOGIC, HeaderStyle header = HeaderStyle::Default, int footerActive = 0);
    float contentLeft() const;
    float contentTop() const;
    float contentWidth() const;
    float contentHeight() const;
    float contentCenterY() const;
    float uiScale() const;
    void getPlayPieceOrigin(int pieceCount, float playX, float playY, float playW, float playH,
                            float spacingX, float spacingY, float &originX, float &originY) const;

    void drawLoginScreen();
    void drawAvatarSelection();
    void drawDifficultySelection();
    void drawMainMenu();
    void drawProtocolsPage(bool overlay = false);
    void drawProtocolsOverlay();
    void drawPlayfield(float x, float y, float w, float h);
    void drawNodeSlot(float x, float y, float scale, bool active = false);
    void drawPlayfieldSlots(float playX, float playY, float playW, float playH);
    void drawTacticalMargins();
    void drawLogicVisualizer(float x, float y, float w, float h);
    void drawHudSidebar(Game &game, Piece *nextPiece, float sidebarX, float viewHeight);
    void drawHudSection(const std::string &title, float x, float y, float w);
    int playfieldCapacity() const;
    void getPlayLayout(float &startX, float &startY, float &spacingX, float &spacingY,
                       float &sidebarX, float &playX, float &playY, float &playW, float &playH) const;
    void drawGameOver(int finalScore, int nodesInBuffer);
    void drawPiece(Piece &piece, float x, float y, float scale = 1.0f);
    void drawAboutPage();
    void drawAchievementsPage();
    void drawSettingsPage();
    void drawLeaderboard(float x, float y);
    void drawStats(float x, float y);
    void drawPlayingHud(Game &game, Piece *nextPiece, GameState state);
    void drawProtocolDiagram(float x, float y, float w, float h);
    void drawMiniPiece(T_Color color, T_Shape shape, float x, float y, float scale = 0.85f);
    void drawWrappedLines(const std::vector<std::string> &lines, float x, float y, float maxW, UI::CodeSize size, sf::Color color, float lineH = 18.0f);
    float drawKeyChip(const std::string &key, float x, float y);
    std::string getTextureName(T_Color color, T_Shape shape);
};

#endif
