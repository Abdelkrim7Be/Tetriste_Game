#include "Renderer.h"
#include <unordered_set>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cctype>

Renderer::Renderer(sf::RenderWindow &win, AssetManager &asmgr, UserManager &umgr)
    : window(win), assets(asmgr), userManager(umgr) {}

bool Renderer::hasFont() const { return assets.hasFont("main"); }
sf::Font &Renderer::font() { return assets.getFont("main"); }
sf::Font &Renderer::fontBold() {
    return assets.hasFont("bold") ? assets.getFont("bold") : assets.getFont("main");
}

void Renderer::drawText(const std::string &str, float x, float y, UI::CodeSize size, sf::Color color, bool bold) {
    if (!hasFont()) return;
    sf::Text t(str, bold ? fontBold() : font(), UI::codeSizePx(size));
    t.setPosition(x, y);
    t.setFillColor(color);
    window.draw(t);
}

float Renderer::textWidth(const std::string &str, UI::CodeSize size, bool bold) {
    if (!hasFont()) return 0;
    sf::Text t(str, bold ? fontBold() : font(), UI::codeSizePx(size));
    return t.getLocalBounds().width;
}

std::string Renderer::nodeIdFromPseudo(const std::string &pseudo) const {
    unsigned h = 0;
    for (char c : pseudo) h = h * 31 + (unsigned char)c;
    std::ostringstream oss;
    oss << "AX-" << std::setw(3) << std::setfill('0') << (h % 1000);
    return oss.str();
}

float Renderer::contentLeft() const {
    return UI::SidebarW;
}
float Renderer::contentTop() const {
    return UI::HeaderH;
}
float Renderer::contentWidth() const {
    return (float)window.getSize().x - UI::SidebarW;
}
float Renderer::contentHeight() const {
    return (float)window.getSize().y - UI::HeaderH - UI::FooterH;
}

float Renderer::uiScale() const {
    auto sz = window.getSize();
    return UI::uiScaleFrom((float)sz.x, (float)sz.y);
}

float Renderer::contentCenterY() const {
    return contentTop() + contentHeight() * 0.5f;
}

void Renderer::getPlayPieceOrigin(int pieceCount, float playX, float playY, float playW, float playH,
                                  float spacingX, float spacingY, float &originX, float &originY) const {
    int cols = std::max(1, (int)((playW - spacingX * 0.5f) / spacingX));
    int rows = std::max(1, (pieceCount + cols - 1) / cols);
    float gridW = (float)(cols - 1) * spacingX;
    float gridH = (float)(rows - 1) * spacingY;
    originX = playX + std::max(spacingX * 0.5f, (playW - gridW) * 0.5f);
    originY = playY + std::max(spacingY * 0.5f, (playH - gridH) * 0.5f);
}

void Renderer::render(Game &game, Piece *nextPiece, GameState state) {
    float deltaTime = clock.restart().asSeconds();
    totalTime += deltaTime;
    cursorBlinkTimer += deltaTime;
    if (insertionTimer > 0) insertionTimer -= deltaTime;
    if (flashTimer > 0) flashTimer -= deltaTime;
    window.clear(UI::CyberBg);

    if (state == GameState::LOGIN) {
        if (authState == AuthState::AVATAR_SELECT) {
            drawTerminalShell(ShellVariant::Dashboard, NavItem::NODES, HeaderStyle::Default, 0);
            drawAvatarSelection();
        } else {
            drawTerminalShell(ShellVariant::Minimal, NavItem::LOGIC, HeaderStyle::Auth, 0);
            drawLoginScreen();
        }
    } else if (state == GameState::MENU) {
        drawTerminalShell(ShellVariant::Minimal, NavItem::LOGIC, HeaderStyle::Default, 0);
        drawMainMenu();
    } else if (state == GameState::DIFFICULTY_SELECT) {
        drawTerminalShell(ShellVariant::Dashboard, NavItem::DECRYPT, HeaderStyle::Default, 0);
        drawDifficultySelection();
    } else if (state == GameState::PROTOCOLS) {
        drawTerminalShell(ShellVariant::Dashboard, NavItem::LOGIC, HeaderStyle::Default, 0);
        drawProtocolsPage(false);
    } else if (state == GameState::ACHIEVEMENTS) {
        drawTerminalShell(ShellVariant::Dashboard, NavItem::CONFIG, HeaderStyle::Default, 0);
        drawAchievementsPage();
    } else if (state == GameState::SETTINGS) {
        drawTerminalShell(ShellVariant::Dashboard, NavItem::CONFIG, HeaderStyle::Default, 0);
        drawSettingsPage();
    } else if (state == GameState::GAME_OVER) {
        drawTerminalShell(ShellVariant::Minimal, NavItem::LOGIC, HeaderStyle::Default, 0);
        drawGameOver(game.score, game.piecesCount);
    } else {
        drawPlayingHud(game, nextPiece, state);
    }

    for (auto it = popups.begin(); it != popups.end(); ) {
        it->lifetime -= deltaTime;
        if (it->lifetime <= 0) it = popups.erase(it);
        else {
            it->text.move(it->velocity * deltaTime);
            sf::Color c = it->text.getFillColor();
            c.a = (sf::Uint8)(255 * (it->lifetime / it->maxLifetime));
            it->text.setFillColor(c);
            window.draw(it->text);
            ++it;
        }
    }

    if (flashTimer > 0) {
        sf::RectangleShape flash(sf::Vector2f(window.getSize().x, window.getSize().y));
        flash.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)(100 * (flashTimer / 0.2f))));
        window.draw(flash);
    }
    window.display();
}

// --- Shell ---

void Renderer::drawBackgroundGrid() {
    float viewWidth = (float)window.getSize().x;
    float viewHeight = (float)window.getSize().y;
    sf::RectangleShape gridLine(sf::Vector2f(viewWidth, 1.0f));
    gridLine.setFillColor(UI::CyberGrid);
    for (float y = 0; y < viewHeight; y += 40.0f) {
        gridLine.setPosition(0, y);
        window.draw(gridLine);
    }
    gridLine.setSize(sf::Vector2f(1.0f, viewHeight));
    for (float x = 0; x < viewWidth; x += 40.0f) {
        gridLine.setPosition(x, 0);
        window.draw(gridLine);
    }
    float scanlineY = std::fmod(totalTime * 80.0f, viewHeight);
    sf::RectangleShape scanline(sf::Vector2f(viewWidth, 2.0f));
    scanline.setPosition(0, scanlineY);
    scanline.setFillColor(sf::Color(121, 255, 91, 20));
    window.draw(scanline);
}

void Renderer::drawCornerLogs(bool dashboard) {
    if (!hasFont()) return;
    float w = (float)window.getSize().x;
    float h = (float)window.getSize().y;
    float logLeft = dashboard ? (UI::SidebarW + 12.0f) : 16.0f;
    auto drawLog = [&](const std::string &txt, float x, float y, bool right) {
        sf::Text t(txt, font(), UI::codeSizePx(UI::CodeSize::XS));
        t.setFillColor(sf::Color(121, 255, 91, 35));
        if (right) {
            sf::FloatRect b = t.getLocalBounds();
            t.setPosition(x - b.width, y);
        } else t.setPosition(x, y);
        window.draw(t);
    };
    if (!dashboard) {
        drawLog("[LOG] KERNEL_ATTACHED: 0x00FF21", logLeft, UI::HeaderH + 8, false);
        drawLog("[LOG] BUFFER_STRIDE: 512_BYTES", logLeft, UI::HeaderH + 22, false);
    }
    drawLog("MEM_DUMP: OK", w - 16, h - UI::FooterH - 28, true);
    drawLog("V_SYNC: LOCKED", w - 16, h - UI::FooterH - 16, true);
}

void Renderer::drawContentBackdrop() {
    sf::RectangleShape bg(sf::Vector2f(contentWidth(), contentHeight()));
    bg.setPosition(contentLeft(), contentTop());
    bg.setFillColor(UI::CyberSurfaceLowest);
    window.draw(bg);
}

void Renderer::drawHeader(HeaderStyle style) {
    if (!hasFont()) return;
    float w = (float)window.getSize().x;
    sf::RectangleShape bar(sf::Vector2f(w, UI::HeaderH));
    bar.setFillColor(UI::CyberBg);
    bar.setOutlineThickness(1.0f);
    bar.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(bar);

    drawText("SYS_INIT_v4.0.2", 12, 6, UI::CodeSize::MD, UI::CyberPrimaryFixed, true);

    float headerTy = 8.0f;
    float leftW = textWidth("SYS_INIT_v4.0.2", UI::CodeSize::MD, true) + 24.0f;
    if (style == HeaderStyle::Auth) {
        std::string sid = "SESSION_ID: 0x992F";
        std::string up = "UPTIME: 00:00:12";
        float gap = 20.0f;
        drawText(sid, leftW, headerTy, UI::CodeSize::SM, UI::MutedText);
        drawText(up, leftW + textWidth(sid, UI::CodeSize::SM) + gap, headerTy, UI::CodeSize::SM, UI::MutedText);
    } else {
        std::string latency = "LATENCY: 14MS";
        std::string tags = "[MEM]  [TERM]  [SET]";
        float gap = 20.0f;
        drawText(latency, leftW, headerTy, UI::CodeSize::SM, UI::CyberPrimaryFixed);
        drawText(tags, leftW + textWidth(latency, UI::CodeSize::SM) + gap, headerTy, UI::CodeSize::SM, UI::CyberPrimaryFixed);
    }
}

void Renderer::drawFooter(int activeIndex) {
    if (!hasFont()) return;
    float w = (float)window.getSize().x;
    float h = (float)window.getSize().y;
    const char *labels[] = {"SHELL", "IO", "NET", "PROC", "PWR"};
    const char *icons[] = {"TERM", "IN", "LAN", "MEM", "PWR"};

    sf::RectangleShape bar(sf::Vector2f(w, UI::FooterH));
    bar.setPosition(0, h - UI::FooterH);
    bar.setFillColor(UI::CyberSurfaceLowest);
    bar.setOutlineThickness(1.0f);
    bar.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(bar);

    float colW = w / 5.0f;
    for (int i = 0; i < 5; ++i) {
        bool active = (i == activeIndex);
        float colX = colW * i;
        if (active) {
            sf::RectangleShape bg(sf::Vector2f(colW, UI::FooterH));
            bg.setPosition(colX, h - UI::FooterH);
            bg.setFillColor(UI::CyberPrimaryFixed);
            window.draw(bg);
        }
        std::string iconLine = std::string(icons[i]) + " " + labels[i];
        float tw = textWidth(iconLine, UI::CodeSize::XS);
        float tx = colX + (colW - tw) / 2.0f;
        drawText(iconLine, tx, h - UI::FooterH + 12, UI::CodeSize::XS, active ? UI::CyberOnPrimary : UI::MutedText);
    }
}

void Renderer::drawDashboardSidebar(NavItem active) {
    float h = (float)window.getSize().y;
    sf::RectangleShape side(sf::Vector2f(UI::SidebarW, h - UI::HeaderH - UI::FooterH));
    side.setPosition(0, UI::HeaderH);
    side.setFillColor(sf::Color(4, 6, 8));
    side.setOutlineThickness(1.0f);
    side.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(side);

    drawText("ROOT_USER", UI::MarginMd, UI::HeaderH + 14, UI::CodeSize::MD, UI::CyberPrimaryFixed, true);
    drawText("MEM_USAGE: 42.1GB", UI::MarginMd, UI::HeaderH + 34, UI::CodeSize::SM, UI::MutedText);

    struct NavRow { NavItem id; const char *label; const char *icon; };
    NavRow rows[] = {
        {NavItem::LOGIC, "LOGIC", "[]"},
        {NavItem::NODES, "NODES", "<>"},
        {NavItem::DECRYPT, "DECRYPT", "{}"},
        {NavItem::CONFIG, "CONFIG", "##"}
    };
    float y = UI::HeaderH + 64;
    for (const auto &row : rows) {
        bool sel = (row.id == active);
        if (sel) {
            sf::RectangleShape hi(sf::Vector2f(UI::SidebarW - 4, 28));
            hi.setPosition(2, y);
            hi.setFillColor(UI::CyberSurfaceContainerHigh);
            window.draw(hi);
            sf::RectangleShape bar(sf::Vector2f(2, 28));
            bar.setPosition(0, y);
            bar.setFillColor(UI::CyberPrimaryFixed);
            window.draw(bar);
        }
        drawText(std::string(row.icon) + " " + row.label, UI::MarginMd + 4, y + 6, UI::CodeSize::SM,
                 sel ? UI::CyberPrimaryFixed : UI::MutedText);
        y += 32;
    }

    sf::RectangleShape btn(sf::Vector2f(UI::SidebarW - UI::MarginMd * 2, 28));
    btn.setPosition(UI::MarginMd, h - UI::FooterH - 40);
    btn.setFillColor(sf::Color::Transparent);
    btn.setOutlineThickness(1.0f);
    btn.setOutlineColor(UI::CyberPrimaryFixed);
    window.draw(btn);
    drawText("RUN_DIAGNOSTIC", UI::MarginMd + 8, h - UI::FooterH - 34, UI::CodeSize::SM, UI::CyberPrimaryFixed);
}

void Renderer::drawTerminalShell(ShellVariant variant, NavItem nav, HeaderStyle header, int footerActive) {
    drawBackgroundGrid();
    bool dashboard = (variant == ShellVariant::Dashboard);
    if (dashboard) {
        drawDashboardSidebar(nav);
        drawContentBackdrop();
    }
    if (variant != ShellVariant::None) {
        drawCornerLogs(dashboard);
        drawHeader(header);
        drawFooter(footerActive);
    }
}

// --- UI primitives ---

void Renderer::drawFolderTab(const std::string &label, float panelX, float panelY, float tabW) {
    sf::RectangleShape tab(sf::Vector2f(tabW, 18));
    tab.setPosition(panelX, panelY - 18);
    tab.setFillColor(UI::CyberSurfaceLowest);
    tab.setOutlineThickness(1.0f);
    tab.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(tab);
    drawText(label, panelX + 6, panelY - 15, UI::CodeSize::XS, UI::CyberPrimaryFixed);
}

void Renderer::drawInvertedButton(float x, float y, float w, float h, const std::string &label,
                                const std::string &sub, bool selected) {
    sf::RectangleShape box(sf::Vector2f(w, h));
    box.setPosition(x, y);
    box.setOutlineThickness(1.0f);
    if (selected) {
        box.setFillColor(UI::CyberPrimaryFixed);
        box.setOutlineColor(UI::CyberPrimaryFixed);
    } else {
        box.setFillColor(UI::CyberSurfaceLow);
        box.setOutlineColor(UI::CyberOutlineVariant);
    }
    window.draw(box);
    sf::Color tc = selected ? UI::CyberOnPrimary : UI::CyberPrimaryFixed;
    drawText(label, x + 12, y + 10, UI::CodeSize::MD, tc);
    float sw = textWidth(sub, UI::CodeSize::XS);
    drawText(sub, x + w - sw - 12, y + 14, UI::CodeSize::XS, selected ? UI::CyberOnPrimary : UI::MutedText);
}

void Renderer::drawGlassPanel(sf::Vector2f size, sf::Vector2f pos) {
    sf::RectangleShape p = UI::createPanel(size, pos, false);
    p.setFillColor(UI::CyberSurfaceLowest);
    window.draw(p);
}

void Renderer::drawCard(const std::string& title, sf::Vector2f pos, sf::Vector2f size) {
    sf::RectangleShape card = UI::createPanel(size, pos, false);
    card.setFillColor(UI::CyberSurfaceLowest);
    window.draw(card);
    drawFolderTab(title, pos.x, pos.y, std::min(size.x * 0.65f, 120.0f));
}

void Renderer::drawAsciiBox(float x, float y) {
    drawText("+-------------------------------------------+", x, y, UI::CodeSize::XS, UI::MutedText);
    drawText("| ENCRYPTED_TUNNEL: ESTABLISHED             |", x, y + 14, UI::CodeSize::XS, UI::MutedText);
    drawText("| PROTOCOL: X-LEVEL-4                       |", x, y + 28, UI::CodeSize::XS, UI::MutedText);
    drawText("| LOCATION: REDACTED                        |", x, y + 42, UI::CodeSize::XS, UI::MutedText);
    drawText("+-------------------------------------------+", x, y + 56, UI::CodeSize::XS, UI::MutedText);
}

void Renderer::drawBitRateColumn(float x, float y) {
    drawText("BIT_RATE", x, y, UI::CodeSize::XS, UI::MutedText);
    for (int i = 0; i < 7; ++i) {
        sf::RectangleShape seg(sf::Vector2f(16, 8));
        seg.setPosition(x, y + 16 + i * 10);
        if (i < 4) {
            sf::Uint8 a = (i < 3) ? 255 : 128;
            seg.setFillColor(sf::Color(UI::CyberPrimaryFixed.r, UI::CyberPrimaryFixed.g, UI::CyberPrimaryFixed.b, a));
        } else {
            seg.setFillColor(sf::Color::Transparent);
            seg.setOutlineThickness(1.0f);
            seg.setOutlineColor(UI::CyberOutlineVariant);
        }
        window.draw(seg);
    }
}

void Renderer::drawStatusChip(float x, float y, const std::string &text, sf::Color accent) {
    sf::RectangleShape chip(sf::Vector2f((float)text.size() * 6 + 24, 22));
    chip.setPosition(x, y);
    chip.setFillColor(UI::CyberBg);
    chip.setOutlineThickness(1.0f);
    chip.setOutlineColor(accent);
    window.draw(chip);
    sf::RectangleShape dot(sf::Vector2f(8, 8));
    dot.setPosition(x + 6, y + 7);
    dot.setFillColor(accent);
    window.draw(dot);
    drawText(text, x + 18, y + 4, UI::CodeSize::XS, accent);
}

void Renderer::drawFloatingChips(float rightX, float topY) {
    const char *lines[] = {"RENDER_ENGINE: ACTIVE", "NODE_SYMBOLOGY: LOADED", "BUFFER_LOAD: 88%"};
    float y = topY;
    for (int i = 0; i < 3; ++i) {
        float tw = textWidth(lines[i], UI::CodeSize::XS);
        sf::RectangleShape chip(sf::Vector2f(tw + 16, 18));
        chip.setPosition(rightX - tw - 16, y);
        chip.setFillColor(UI::CyberSurfaceLow);
        chip.setOutlineThickness(1.0f);
        chip.setOutlineColor(UI::CyberOutlineVariant);
        window.draw(chip);
        drawText(lines[i], rightX - tw - 8, y + 2, UI::CodeSize::XS,
                 i == 2 ? UI::CyberSecondaryFixedDim : UI::CyberPrimaryFixed);
        y += 24;
    }
}

// --- Pieces ---

void Renderer::drawMiniPiece(T_Color color, T_Shape shape, float x, float y, float scale) {
    Piece p(color, shape, nullptr, nullptr, nullptr, nullptr, nullptr);
    drawFancyPiece(p, x, y, scale, false, false);
}

void Renderer::drawFancyPiece(Piece &piece, float x, float y, float scale, bool highlight, bool backdrop) {
    std::string texName = getTextureName(piece.color, piece.shape);
    float dim = (float)UI::PieceTexPx * scale;
    float cellSide = dim + 2.0f * UI::PlayCellInset;
    float cellHalf = cellSide * 0.5f;

    if (backdrop) {
        sf::RectangleShape bd(sf::Vector2f(cellSide, cellSide));
        bd.setOrigin(cellHalf, cellHalf);
        bd.setPosition(x, y);
        bd.setFillColor(sf::Color(8, 12, 18, 180));
        bd.setOutlineThickness(1.0f);
        bd.setOutlineColor(UI::CyberOutlineVariant);
        window.draw(bd);
    }
    if (highlight) {
        sf::RectangleShape sel(sf::Vector2f(cellSide, cellSide));
        sel.setOrigin(cellHalf, cellHalf);
        sel.setPosition(x, y);
        sel.setFillColor(sf::Color::Transparent);
        sel.setOutlineThickness(2.0f);
        sel.setOutlineColor(UI::CyberPrimaryFixed);
        window.draw(sel);
    }

    if (!assets.hasTexture(texName)) {
        sf::Color fc = sf::Color::Blue;
        switch (piece.color) {
            case T_Color::YELLOW: fc = sf::Color::Yellow; break;
            case T_Color::RED: fc = sf::Color::Red; break;
            case T_Color::GREEN: fc = sf::Color::Green; break;
            case T_Color::WHITE: fc = sf::Color::White; break;
            default: break;
        }
        float hs = dim * 0.5f;
        if (piece.shape == T_Shape::CIRCLE) {
            sf::CircleShape c(hs);
            c.setOrigin(hs, hs);
            c.setPosition(x, y);
            c.setFillColor(fc);
            window.draw(c);
        } else if (piece.shape == T_Shape::SQUARE) {
            sf::RectangleShape r(sf::Vector2f(dim, dim));
            r.setOrigin(hs, hs);
            r.setPosition(x, y);
            r.setFillColor(fc);
            window.draw(r);
        } else {
            sf::ConvexShape convex;
            if (piece.shape == T_Shape::TRIANGLE) {
                convex.setPointCount(3);
                convex.setPoint(0, sf::Vector2f(x, y - hs));
                convex.setPoint(1, sf::Vector2f(x - hs, y + hs * 0.82f));
                convex.setPoint(2, sf::Vector2f(x + hs, y + hs * 0.82f));
            } else if (piece.shape == T_Shape::DIAMOND) {
                convex.setPointCount(4);
                convex.setPoint(0, sf::Vector2f(x, y - hs));
                convex.setPoint(1, sf::Vector2f(x + hs, y));
                convex.setPoint(2, sf::Vector2f(x, y + hs));
                convex.setPoint(3, sf::Vector2f(x - hs, y));
            } else {
                convex.setPointCount(10);
                for (int i = 0; i < 10; ++i) {
                    float angle = -3.14159f / 2.0f + i * 2 * 3.14159f / 10;
                    float r = (i % 2 == 0) ? hs : hs * 0.42f;
                    convex.setPoint(i, sf::Vector2f(x + r * std::cos(angle), y + r * std::sin(angle)));
                }
            }
            convex.setFillColor(fc);
            window.draw(convex);
        }
        return;
    }
    sf::Sprite sprite(assets.getTexture(texName));
    sf::FloatRect b = sprite.getLocalBounds();
    sprite.setOrigin(b.width / 2, b.height / 2);
    sprite.setPosition(x, y);
    sprite.setColor(sf::Color::White);
    sprite.setScale(scale, scale);
    window.draw(sprite);
}

void Renderer::drawWrappedLines(const std::vector<std::string> &lines, float x, float y, float maxW,
                                UI::CodeSize size, sf::Color color, float lineH) {
    float cy = y;
    for (const auto &line : lines) {
        drawText(line, x, cy, size, color);
        cy += lineH;
    }
}

float Renderer::drawKeyChip(const std::string &key, float x, float y) {
    float tw = textWidth(key, UI::CodeSize::SM, true);
    float chipW = std::max(24.0f, tw + 14.0f);
    sf::RectangleShape chip(sf::Vector2f(chipW, 22));
    chip.setPosition(x, y);
    chip.setFillColor(UI::CyberSurfaceContainerHigh);
    chip.setOutlineThickness(1.0f);
    chip.setOutlineColor(UI::CyberPrimaryFixed);
    window.draw(chip);
    drawText(key, x + (chipW - tw) * 0.5f, y + 3, UI::CodeSize::SM, UI::CyberPrimaryFixed, true);
    return chipW;
}

void Renderer::getPlayLayout(float &startX, float &startY, float &spacingX, float &spacingY,
                             float &sidebarX, float &playX, float &playY, float &playW, float &playH) const {
    float viewWidth = (float)window.getSize().x;
    float viewHeight = (float)window.getSize().y;
    const float hudW = UI::PlayHudW;
    sidebarX = viewWidth - hudW;
    playX = 12.0f;
    playY = UI::HeaderH + 8.0f;
    playW = sidebarX - playX - 8.0f;
    playH = viewHeight - playY - UI::FooterH - 8.0f;
    float u = uiScale();
    float cell = UI::pieceSpacing(u);
    spacingX = spacingY = cell;
    float gridOx, gridOy;
    UI::playGridOrigin(playX, playY, gridOx, gridOy);
    startX = gridOx + cell * 0.5f;
    startY = gridOy + cell * 0.5f;
}

void Renderer::drawNodeSlot(float cx, float cy, float cellSize, bool active) {
    float slotSz = cellSize - 1.0f;
    sf::RectangleShape slot(sf::Vector2f(slotSz, slotSz));
    slot.setOrigin(slotSz * 0.5f, slotSz * 0.5f);
    slot.setPosition(cx, cy);
    slot.setFillColor(active ? sf::Color(9, 15, 22, 220) : sf::Color(6, 10, 16, 200));
    slot.setOutlineThickness(1.0f);
    slot.setOutlineColor(sf::Color(UI::CyberOutlineVariant.r, UI::CyberOutlineVariant.g, UI::CyberOutlineVariant.b, 140));
    window.draw(slot);
}

void Renderer::drawPlayfieldSlots(float playX, float playY, float playW, float playH) {
    float u = uiScale();
    float cell = UI::pieceSpacing(u);
    float gridOx, gridOy;
    UI::playGridOrigin(playX, playY, gridOx, gridOy);
    float gridH = playH - UI::PlayLabelBand - UI::PlayGridMarginY - 6.0f;
    int cols = std::max(1, (int)((playW - UI::PlayGridMarginX * 2) / cell));
    int rows = std::max(1, (int)(gridH / cell));
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            float cx = gridOx + (c + 0.5f) * cell;
            float cy = gridOy + (r + 0.5f) * cell;
            drawNodeSlot(cx, cy, cell, (r + c) % 2 == 0);
        }
    }
}

void Renderer::drawPlayfield(float x, float y, float w, float h) {
    sf::RectangleShape bg(sf::Vector2f(w, h));
    bg.setPosition(x, y);
    bg.setFillColor(sf::Color(6, 10, 14));
    window.draw(bg);

    sf::RectangleShape hdr(sf::Vector2f(w - 4, UI::PlayLabelBand));
    hdr.setPosition(x + 2, y + 2);
    hdr.setFillColor(UI::CyberSurfaceLowest);
    hdr.setOutlineThickness(1.0f);
    hdr.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(hdr);

    float pulse = 0.55f + 0.45f * std::sin(totalTime * 2.0f);
    sf::RectangleShape outer(sf::Vector2f(w, h));
    outer.setPosition(x, y);
    outer.setFillColor(sf::Color::Transparent);
    outer.setOutlineThickness(2.0f);
    outer.setOutlineColor(sf::Color(UI::CyberPrimaryFixed.r, UI::CyberPrimaryFixed.g, UI::CyberPrimaryFixed.b, (sf::Uint8)(180 * pulse)));
    window.draw(outer);

    sf::RectangleShape inner(sf::Vector2f(w - 8, h - 8));
    inner.setPosition(x + 4, y + 4);
    inner.setFillColor(sf::Color::Transparent);
    inner.setOutlineThickness(1.0f);
    inner.setOutlineColor(sf::Color(UI::CyberPrimaryFixed.r, UI::CyberPrimaryFixed.g, UI::CyberPrimaryFixed.b, 90));
    window.draw(inner);

    drawPlayfieldSlots(x, y, w, h);
}

void Renderer::drawPiece(Piece &p, float x, float y, float s) { drawFancyPiece(p, x, y, s, false, false); }

std::string Renderer::getTextureName(T_Color c, T_Shape s) {
    std::string cs, ss;
    switch (c) {
        case T_Color::BLUE: cs = "blue"; break;
        case T_Color::YELLOW: cs = "yellow"; break;
        case T_Color::RED: cs = "red"; break;
        case T_Color::GREEN: cs = "green"; break;
        case T_Color::WHITE: cs = "white"; break;
    }
    switch (s) {
        case T_Shape::SQUARE: ss = "square"; break;
        case T_Shape::DIAMOND: ss = "diamond"; break;
        case T_Shape::CIRCLE: ss = "circle"; break;
        case T_Shape::TRIANGLE: ss = "triangle"; break;
        case T_Shape::STAR: ss = "star"; break;
    }
    return cs + "_" + ss;
}

// --- LOGIN ---

void Renderer::drawLoginScreen() {
    if (!hasFont()) return;
    float u = uiScale();
    float cx = (float)window.getSize().x / 2.0f;
    float viewH = (float)window.getSize().y;
    float areaH = viewH - UI::HeaderH - UI::FooterH;

    drawAsciiBox(16, UI::HeaderH + 8);
    drawBitRateColumn((float)window.getSize().x - 48, UI::HeaderH + areaH * 0.35f);

    float pw = std::min(480.0f, 400.0f * u), ph = 200.0f;
    float blockH = 90.0f + ph + 56.0f;
    float blockTop = UI::HeaderH + (areaH - blockH) * 0.42f;

    sf::Text title("TETRISTE", fontBold(), UI::codeSizePx(UI::CodeSize::BrandAuth));
    title.setFillColor(UI::CyberPrimaryFixed);
    UI::centerText(title, sf::Vector2f(cx, blockTop + 18));
    window.draw(title);

    float px = cx - pw / 2, py = blockTop + 72;
    drawFolderTab("USER_AUTH_MODULE", px, py, 150);
    sf::RectangleShape box(sf::Vector2f(pw, ph));
    box.setPosition(px, py);
    box.setFillColor(UI::CyberSurfaceLowest);
    box.setOutlineThickness(1.0f);
    box.setOutlineColor(UI::CyberOutline);
    window.draw(box);

    std::string lbl = "OPERATOR_ID (PSEUDO)";
    std::string val = loginPseudo;
    if (authState == AuthState::PIN_ENTRY) { lbl = "ENTER 4-DIGIT PIN"; val = std::string(loginPin.length(), '*'); }
    else if (authState == AuthState::PIN_SETUP) { lbl = "SET 4-DIGIT PIN"; val = std::string(loginPin.length(), '*'); }

    drawText(lbl, px + 16, py + 70, UI::CodeSize::SM, UI::MutedText);
    sf::RectangleShape inp(sf::Vector2f(pw - 32, 40));
    inp.setPosition(px + 16, py + 92);
    inp.setFillColor(sf::Color::Black);
    inp.setOutlineThickness(1.0f);
    inp.setOutlineColor(UI::CyberPrimaryFixed);
    window.draw(inp);
    drawText(">", px + 24, py + 98, UI::CodeSize::LG, UI::CyberPrimaryFixed, true);
    drawText(val.empty() ? "" : val, px + 44, py + 98, UI::CodeSize::MD, UI::CyberPrimaryFixed);
    if ((int)(cursorBlinkTimer * 2) % 2 == 0) {
        float cx2 = px + 44 + textWidth(val, UI::CodeSize::MD);
        sf::RectangleShape cur(sf::Vector2f(10, 20));
        cur.setPosition(cx2 + 4, py + 100);
        cur.setFillColor(UI::CyberPrimaryFixed);
        window.draw(cur);
    }

    drawText("ENTER_ID_TO_SYNCHRONIZE", cx - textWidth("ENTER_ID_TO_SYNCHRONIZE", UI::CodeSize::SM) / 2, py + ph + 16,
             UI::CodeSize::SM, UI::MutedText);
    drawText("[ PRESS_ENTER ]", cx - textWidth("[ PRESS_ENTER ]", UI::CodeSize::XS) / 2, py + ph + 36,
             UI::CodeSize::XS, UI::MutedText);

    drawStatusChip(px + pw - 140, py + ph - 10, "AWAITING_INPUT", UI::CyberSecondaryFixed);
}

// --- AVATAR ---

void Renderer::drawAvatarSelection() {
    if (!hasFont()) return;
    float cx = contentLeft() + contentWidth() / 2.0f;
    float right = contentLeft() + contentWidth() - 12;
    float u = uiScale();
    float pw = std::min(std::max(420.0f, 480.0f * u), contentWidth() - 24);
    float cellStep = std::max(58.0f, std::min(74.0f * u, (pw - 48.0f) / 4.0f));
    float gridH = cellStep * 5;
    float pieceScale = UI::pieceGridScale(u);
    float cellSz = (float)UI::PieceTexPx * pieceScale + 12.0f;
    float headerBand = 44.0f;
    float cmdH = 52;
    float ph = headerBand + gridH + cmdH + 16;
    float statsH = 44;
    float titleH = 32;
    float blockH = titleH + ph + statsH + 12;
    float blockTop = contentTop() + (contentHeight() - blockH) * 0.42f;

    drawFloatingChips(right, blockTop + 4);
    drawText("TETRISTE", cx - textWidth("TETRISTE", UI::CodeSize::XL, true) / 2, blockTop + 2,
             UI::CodeSize::XL, UI::CyberPrimaryFixed, true);

    float px = cx - pw / 2;
    float py = blockTop + titleH + 6;

    sf::RectangleShape panel(sf::Vector2f(pw, ph));
    panel.setPosition(px, py);
    panel.setFillColor(UI::CyberSurface);
    panel.setOutlineThickness(1.0f);
    panel.setOutlineColor(UI::CyberPrimaryFixed);
    window.draw(panel);

    std::string nid = "NODE_ID: " + nodeIdFromPseudo(loginPseudo.empty() ? "OP" : loginPseudo);
    drawText("SELECT YOUR AVATAR", px + 14, py + 10, UI::CodeSize::MD, UI::CyberPrimaryFixed, true);
    drawText(nid, px + pw - textWidth(nid, UI::CodeSize::SM) - 14, py + 12, UI::CodeSize::SM, UI::MutedText);

    sf::RectangleShape hdrSep(sf::Vector2f(pw - 28, 1));
    hdrSep.setPosition(px + 14, py + headerBand - 6);
    hdrSep.setFillColor(UI::CyberOutlineVariant);
    window.draw(hdrSep);

    float gridX0 = px + (pw - cellStep * 4) / 2.0f;
    float gridY0 = py + headerBand + cellStep * 0.5f;
    int count = 0;
    for (int c = 0; c < 5; ++c) {
        for (int s = 0; s < 5; ++s) {
            float gx = gridX0 + s * cellStep;
            float gy = gridY0 + c * cellStep;
            bool selected = (count == avatarIndex);
            sf::RectangleShape cell(sf::Vector2f(cellSz, cellSz));
            cell.setPosition(gx - cellSz * 0.5f, gy - cellSz * 0.5f);
            cell.setFillColor(UI::CyberSurfaceLowest);
            cell.setOutlineThickness(selected ? 2.0f : 1.0f);
            cell.setOutlineColor(selected ? UI::CyberPrimaryFixed : UI::CyberOutlineVariant);
            window.draw(cell);
            Piece p((T_Color)c, (T_Shape)s, nullptr, nullptr, nullptr, nullptr, nullptr);
            drawFancyPiece(p, gx, gy, pieceScale, selected, false);
            count++;
        }
    }

    float cmdY = gridY0 + gridH + 10;
    sf::RectangleShape sep(sf::Vector2f(pw - 24, 1));
    sep.setPosition(px + 12, cmdY);
    sep.setFillColor(UI::CyberOutlineVariant);
    window.draw(sep);
    drawText("> COMMAND: INIT_AVATAR_BIND", px + 12, cmdY + 10, UI::CodeSize::SM, UI::CyberPrimaryFixed);
    if ((int)(cursorBlinkTimer * 2) % 2 == 0) {
        float cx2 = px + 12 + textWidth("> COMMAND: INIT_AVATAR_BIND", UI::CodeSize::SM);
        sf::RectangleShape cur(sf::Vector2f(8, 14));
        cur.setPosition(cx2 + 4, cmdY + 12);
        cur.setFillColor(UI::CyberPrimaryFixed);
        window.draw(cur);
    }
    drawText("USE ARROWS TO SELECT | ENTER TO FINALIZE", px + 12, cmdY + 30, UI::CodeSize::XS, UI::MutedText);

    float fy = py + ph + 10;
    float colW = (pw - 16) / 3.0f;
    const char *stats[][2] = {{"LATENCY", "12.0ms"}, {"PACKETS", "2.4k/s"}, {"ENCRYPTION", "AES-256"}};
    for (int i = 0; i < 3; ++i) {
        sf::RectangleShape cell(sf::Vector2f(colW - 4, 34));
        cell.setPosition(px + 8 + i * colW, fy);
        cell.setFillColor(UI::CyberSurfaceLow);
        cell.setOutlineThickness(1.0f);
        cell.setOutlineColor(UI::CyberOutlineVariant);
        window.draw(cell);
        drawText(stats[i][0], px + 12 + i * colW, fy + 4, UI::CodeSize::XS, UI::MutedText);
        drawText(stats[i][1], px + 12 + i * colW, fy + 18, UI::CodeSize::SM, UI::CyberPrimaryFixed);
    }
}

// --- MENU ---

void Renderer::drawMainMenu() {
    if (!hasFont()) return;
    float u = uiScale();
    float cx = (float)window.getSize().x / 2.0f;
    float viewH = (float)window.getSize().y;
    float areaH = viewH - UI::HeaderH - UI::FooterH;
    float itemH = 44.0f, gap = 10.0f;
    float menuInnerH = 36.0f + 5.0f * (itemH + gap) + 8.0f;
    float pw = std::min((float)window.getSize().x * 0.52f, 540.0f * u);
    float ph = menuInnerH + 20.0f;
    float blockH = 96.0f + ph + 62.0f;
    float top = UI::HeaderH + (areaH - blockH) * 0.38f;

    sf::Text title("TETRISTE", fontBold(), UI::codeSizePx(UI::CodeSize::BrandMenu));
    title.setFillColor(UI::CyberPrimaryFixed);
    UI::centerText(title, sf::Vector2f(cx, top + 28));
    window.draw(title);

    float lineW = 280;
    sf::RectangleShape lineL(sf::Vector2f(lineW, 1)), lineR(sf::Vector2f(lineW, 1));
    lineL.setFillColor(UI::CyberOutlineVariant);
    lineR.setFillColor(UI::CyberOutlineVariant);
    lineL.setPosition(cx - lineW - 80, top + 72);
    lineR.setPosition(cx + 80, top + 72);
    window.draw(lineL);
    window.draw(lineR);
    drawText("ADVANCED MASTERY PROTOCOL", cx - textWidth("ADVANCED MASTERY PROTOCOL", UI::CodeSize::SM) / 2, top + 64,
             UI::CodeSize::SM, UI::CyberPrimaryFixed);

    float px = cx - pw / 2, py = top + 96;
    drawFolderTab("LINK_INTERFACE.EXE", px, py, 140);
    drawGlassPanel(sf::Vector2f(pw, ph), sf::Vector2f(px, py));
    drawText("SELECT_TASK_TO_PROCEED:", px + 12, py + 16, UI::CodeSize::SM, UI::MutedText);

    std::vector<std::string> opts = {
        "[01] ESTABLISH_LINK", "[02] SYSTEM_PROTOCOLS", "[03] PROTOCOL_ARCHIVE",
        "[04] SYSTEM_CONFIG", "[05] TERMINATE_PROCESS"
    };
    std::vector<std::string> subs = {"(PLAY)", "(RULES)", "(ARCHIVE)", "(CONFIG)", "(EXIT)"};
    for (int i = 0; i < 5; ++i) {
        drawInvertedButton(px + 12, py + 36 + i * (itemH + gap), pw - 24, itemH, opts[i], subs[i], i == menuSelection);
    }

    drawText("MOD: CRT_88", px + 14, py + menuInnerH + 2, UI::CodeSize::XS, UI::MutedText);

    float statsY = py + ph + 24;
    float statW = std::min(140.0f, pw / 3.0f - 8);
    auto statBox = [&](const std::string &l, const std::string &v, float x) {
        sf::RectangleShape b(sf::Vector2f(statW, 40));
        b.setPosition(x, statsY);
        b.setFillColor(UI::CyberBg);
        b.setOutlineThickness(1.0f);
        b.setOutlineColor(UI::CyberOutlineVariant);
        window.draw(b);
        drawText(l, x + 6, statsY + 4, UI::CodeSize::XS, UI::MutedText);
        drawText(v, x + 6, statsY + 18, UI::CodeSize::SM, UI::CyberPrimaryFixed);
    };
    UserProfile profile = userManager.getCurrentUserProfile();
    float statX0 = cx - statW * 1.5f - 12;
    statBox("SESSIONS", std::to_string(profile.matchesPlayed), statX0);
    statBox("BEST", std::to_string(profile.record), statX0 + statW + 8);
    statBox("LAST", std::to_string(profile.lastScore), statX0 + (statW + 8) * 2);
}

// --- DIFFICULTY ---

void Renderer::drawDifficultySelection() {
    if (!hasFont()) return;
    float cx = contentLeft() + contentWidth() / 2.0f;
    float cy = contentTop() + contentHeight() / 2.0f;
    float pw = contentWidth() - 48, ph = 380;
    float px = cx - pw / 2, py = cy - ph / 2;

    sf::RectangleShape outer(sf::Vector2f(pw, ph));
    outer.setPosition(px, py);
    outer.setFillColor(UI::CyberSurfaceDim);
    outer.setOutlineThickness(1.0f);
    outer.setOutlineColor(UI::CyberOutline);
    window.draw(outer);
    sf::RectangleShape inner(sf::Vector2f(pw - 8, ph - 8));
    inner.setPosition(px + 4, py + 4);
    inner.setFillColor(UI::CyberSurfaceLowest);
    inner.setOutlineThickness(1.0f);
    inner.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(inner);

    sf::RectangleShape badge(sf::Vector2f(220, 22));
    badge.setPosition(cx - 110, py + 24);
    badge.setFillColor(sf::Color::Transparent);
    badge.setOutlineThickness(1.0f);
    badge.setOutlineColor(UI::CyberPrimaryFixed);
    window.draw(badge);
    drawText("PHASE_01: SELECTION_REQUIRED", cx - textWidth("PHASE_01: SELECTION_REQUIRED", UI::CodeSize::SM) / 2,
             py + 28, UI::CodeSize::SM, UI::CyberPrimaryFixed);

    drawText("SELECT SIMULATION STRESS LEVEL", cx - textWidth("SELECT SIMULATION STRESS LEVEL", UI::CodeSize::XL) / 2,
             py + 60, UI::CodeSize::XL, UI::CyberPrimaryFixed, true);
    drawText("System latency optimization engaged | [G_VECT_CALIBRATION]",
             cx - textWidth("System latency optimization engaged | [G_VECT_CALIBRATION]", UI::CodeSize::SM) / 2,
             py + 92, UI::CodeSize::SM, UI::MutedText);

    struct DiffRow { const char *name; const char *meta; };
    DiffRow rows[] = {
        {"RECRUIT", "G_LEVEL: 0.5  |  BOARD_CAPACITY: 100%  |  GRAVITY: SLOW"},
        {"VETERAN", "G_LEVEL: 1.0  |  BOARD_CAPACITY: 80%   |  GRAVITY: NORM"},
        {"ELITE", "G_LEVEL: 2.0  |  BOARD_CAPACITY: 60%   |  GRAVITY: FAST"}
    };
    float cardH = 72;
    for (int i = 0; i < 3; ++i) {
        bool sel = ((int)difficulty == i);
        float cy2 = py + 120 + i * (cardH + 8);
        sf::RectangleShape card(sf::Vector2f(pw - 48, cardH));
        card.setPosition(px + 24, cy2);
        card.setOutlineThickness(1.0f);
        if (sel) {
            card.setFillColor(UI::CyberPrimaryFixed);
            card.setOutlineColor(UI::CyberPrimaryFixed);
        } else {
            card.setFillColor(UI::CyberSurfaceLow);
            card.setOutlineColor(UI::CyberOutlineVariant);
        }
        window.draw(card);
        sf::Color tc = sel ? UI::CyberOnPrimary : UI::CyberPrimaryFixed;
        drawText(rows[i].name, px + 40, cy2 + 12, UI::CodeSize::LG, tc, true);
        drawText(rows[i].meta, px + 40, cy2 + 40, UI::CodeSize::XS, sel ? UI::CyberOnPrimary : UI::MutedText);
    }
    drawText("UP/DOWN: SELECT  |  ENTER: DEPLOY  |  ESC: ABORT", cx - 120, py + ph - 24, UI::CodeSize::SM, UI::MutedText);
}

// --- PROTOCOLS ---

void Renderer::drawProtocolDiagram(float x, float y, float w, float h) {
    const float pad = 14.0f;
    sf::RectangleShape panel(sf::Vector2f(w, h));
    panel.setPosition(x, y);
    panel.setFillColor(UI::CyberSurface);
    panel.setOutlineThickness(1.0f);
    panel.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(panel);

    sf::RectangleShape tag(sf::Vector2f(168, 22));
    tag.setPosition(x + pad, y + 8);
    tag.setFillColor(UI::CyberPrimaryFixed);
    window.draw(tag);
    drawText("RING TOPOLOGY", x + pad + 8, y + 11, UI::CodeSize::SM, UI::CyberOnPrimary, true);

    float cx = x + w * 0.5f;
    float cy = y + 40 + (h - 108) * 0.44f;
    float cell = std::min(30.0f, w * 0.11f);
    auto ringNode = [&](float ox, float oy, const char *label, float lx, float ly) {
        float nx = cx + ox, ny = cy + oy;
        drawNodeSlot(nx, ny, cell, true);
        drawText(label, nx + lx, ny + ly, UI::CodeSize::SM, UI::CyberPrimaryFixed, true);
    };
    ringNode(0, -46, "HEAD", -16, -cell * 0.5f - 16);
    ringNode(50, 34, "TAIL", 6, cell * 0.5f + 4);
    ringNode(-50, 34, "RING", -38, cell * 0.5f + 4);

    auto link = [&](float x1, float y1, float x2, float y2) {
        sf::Vertex seg[] = {
            sf::Vertex(sf::Vector2f(x1, y1), UI::CyberPrimaryFixed),
            sf::Vertex(sf::Vector2f(x2, y2), UI::CyberPrimaryFixed)
        };
        window.draw(seg, 2, sf::Lines);
    };
    link(cx, cy - 30, cx + 36, cy + 22);
    link(cx + 36, cy + 22, cx - 36, cy + 22);
    link(cx - 36, cy + 22, cx, cy - 30);

    float by = y + h - 56;
    drawText("RING:", x + pad, by, UI::CodeSize::SM, UI::CyberText);
    drawText("DOUBLY-LINKED", x + w - textWidth("DOUBLY-LINKED", UI::CodeSize::SM) - pad, by,
             UI::CodeSize::SM, UI::CyberPrimaryFixed);
    sf::RectangleShape barBg(sf::Vector2f(w - pad * 2, 8));
    barBg.setPosition(x + pad, by + 18);
    barBg.setFillColor(UI::CyberSurfaceContainer);
    window.draw(barBg);
    sf::RectangleShape barFill(sf::Vector2f(w - pad * 2, 8));
    barFill.setPosition(x + pad, by + 18);
    barFill.setFillColor(sf::Color(UI::CyberPrimaryFixed.r, UI::CyberPrimaryFixed.g, UI::CyberPrimaryFixed.b, 90));
    window.draw(barFill);
    sf::RectangleShape barAccent(sf::Vector2f((w - pad * 2) * 0.35f, 8));
    barAccent.setPosition(x + pad, by + 18);
    barAccent.setFillColor(UI::CyberPrimaryFixed);
    window.draw(barAccent);
    drawText("TOPOLOGY: CLOSED LOOP", x + pad, by + 32, UI::CodeSize::XS, UI::MutedText);
}

void Renderer::drawLogicVisualizer(float x, float y, float w, float h) {
    const float pad = 14.0f;
    sf::RectangleShape panel(sf::Vector2f(w, h));
    panel.setPosition(x, y);
    panel.setFillColor(UI::CyberSurface);
    panel.setOutlineThickness(1.0f);
    panel.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(panel);

    sf::RectangleShape pip(sf::Vector2f(3, 11));
    pip.setPosition(x + pad, y + 10);
    pip.setFillColor(UI::CyberPrimaryFixed);
    window.draw(pip);
    drawText("LOGIC VISUALIZER", x + pad + 8, y + 8, UI::CodeSize::MD, UI::CyberPrimary, true);
    sf::RectangleShape sep(sf::Vector2f(w - pad * 2 - 8, 1));
    sep.setPosition(x + pad + 8, y + 28);
    sep.setFillColor(UI::CyberOutlineVariant);
    window.draw(sep);

    T_Color colors[] = {T_Color::BLUE, T_Color::YELLOW, T_Color::RED, T_Color::GREEN, T_Color::WHITE};
    T_Shape shapes[] = {T_Shape::SQUARE, T_Shape::DIAMOND, T_Shape::TRIANGLE, T_Shape::STAR, T_Shape::CIRCLE};
    float cellSz = std::min(32.0f, (w - pad * 2) / 5.0f - 4.0f);
    float pieceScale = (cellSz - 2.0f * UI::PlayCellInset) / (float)UI::PieceTexPx;
    float iconStep = (w - pad * 2) / 5.0f;
    float row1Y = y + 52;
    float row2Y = row1Y + cellSz + 22;
    drawText("COLORS", x + pad + 4, row1Y - 14, UI::CodeSize::XS, UI::MutedText);
    drawText("SHAPES", x + pad + 4, row2Y - 14, UI::CodeSize::XS, UI::MutedText);
    for (int i = 0; i < 5; ++i) {
        float cx = x + pad + (i + 0.5f) * iconStep;
        drawNodeSlot(cx, row1Y, cellSz, i % 2 == 0);
        drawMiniPiece(colors[i], T_Shape::SQUARE, cx, row1Y, pieceScale);
        drawNodeSlot(cx, row2Y, cellSz, (i + 1) % 2 == 0);
        drawMiniPiece(T_Color::BLUE, shapes[i], cx, row2Y, pieceScale);
    }

    float legendY = row2Y + cellSz + 18;
    float legendH = h - legendY + y - 10;
    sf::RectangleShape legend(sf::Vector2f(w - pad * 2, legendH));
    legend.setPosition(x + pad, legendY);
    legend.setFillColor(UI::CyberSurfaceLowest);
    legend.setOutlineThickness(1.0f);
    legend.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(legend);
    float lx = x + pad + 12;
    float textBlockH = legendH - 52.0f;
    float ty = legendY + 10.0f;
    drawText("MATCH RULE", lx, ty, UI::CodeSize::MD, UI::CyberPrimaryFixed, true);
    ty += 26.0f;
    drawText("3+ IN A ROW IN THE RING", lx, ty, UI::CodeSize::SM, UI::CyberText);
    ty += 20.0f;
    drawText("(COLOR OR SHAPE)", lx, ty, UI::CodeSize::SM, UI::CyberText);
    ty += 24.0f;
    drawText("LONGER RUN WINS;", lx, ty, UI::CodeSize::SM, UI::MutedText);
    ty += 18.0f;
    drawText("THAT RUN IS PURGED", lx, ty, UI::CodeSize::SM, UI::MutedText);
    if (textBlockH > 130.0f) {
        ty += 22.0f;
        drawText("GREEN FRAME = MATCH ARC", lx, ty, UI::CodeSize::SM, UI::CyberPrimaryFixed);
    }

    float ringCell = std::min(24.0f, (w - pad * 2 - 20) / 7.0f);
    float ringScale = (ringCell - 2.0f * UI::PlayCellInset) / (float)UI::PieceTexPx;
    float ringStep = ringCell + 3.0f;
    const int ringLen = 7;
    float ringW = ringLen * ringStep - 3.0f;
    float ringStartX = x + pad + (w - pad * 2 - ringW) * 0.5f + ringCell * 0.5f;
    float ringY = legendY + legendH - ringCell - 12;
    drawText("PURGE ARC (EXAMPLE)", ringStartX - ringCell * 0.5f, ringY - 15, UI::CodeSize::SM, UI::MutedText);
    T_Color ringColors[] = {T_Color::WHITE, T_Color::BLUE, T_Color::BLUE, T_Color::BLUE,
                            T_Color::RED, T_Color::GREEN, T_Color::YELLOW};
    T_Shape ringShapes[] = {T_Shape::CIRCLE, T_Shape::SQUARE, T_Shape::SQUARE, T_Shape::SQUARE,
                            T_Shape::TRIANGLE, T_Shape::STAR, T_Shape::DIAMOND};
    bool ringMatch[] = {false, true, true, true, false, false, false};
    for (int i = 0; i < ringLen; ++i) {
        float rcx = ringStartX + i * ringStep;
        if (ringMatch[i]) {
            sf::RectangleShape hi(sf::Vector2f(ringCell + 2, ringCell + 2));
            hi.setOrigin((ringCell + 2) * 0.5f, (ringCell + 2) * 0.5f);
            hi.setPosition(rcx, ringY);
            hi.setFillColor(sf::Color::Transparent);
            hi.setOutlineThickness(2.0f);
            hi.setOutlineColor(UI::CyberPrimaryFixed);
            window.draw(hi);
        }
        drawNodeSlot(rcx, ringY, ringCell, i % 2 == 0);
        drawMiniPiece(ringColors[i], ringShapes[i], rcx, ringY, ringScale);
    }
}

void Renderer::drawProtocolsPage(bool overlay) {
    if (!hasFont()) return;
    float left = overlay ? 24.0f : contentLeft() + UI::MarginSm;
    float top = overlay ? UI::HeaderH + 8.0f : contentTop() + UI::MarginSm;
    float cw = overlay ? (float)window.getSize().x - 48.0f : contentWidth() - UI::MarginSm * 2;
    float ch = overlay ? contentHeight() - 16.0f : contentHeight() - UI::MarginSm * 2;
    float sideW = std::max(240.0f, cw * 0.40f);
    float mainW = cw - sideW - 10;
    if (mainW < 360.0f) { mainW = 360.0f; sideW = cw - mainW - 10; }
    float panelH = ch;

    if (overlay) {
        float pulse = 0.45f + 0.55f * std::sin(totalTime * 2.2f);
        sf::RectangleShape glow(sf::Vector2f(cw + 8, panelH + 8));
        glow.setPosition(left - 4, top - 4);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(2.0f);
        glow.setOutlineColor(sf::Color(UI::CyberPrimaryFixed.r, UI::CyberPrimaryFixed.g, UI::CyberPrimaryFixed.b,
                                         (sf::Uint8)(170 * pulse)));
        window.draw(glow);
    }

    sf::RectangleShape mainPanel(sf::Vector2f(mainW, panelH));
    mainPanel.setPosition(left, top);
    mainPanel.setFillColor(UI::CyberSurface);
    mainPanel.setOutlineThickness(1.0f);
    mainPanel.setOutlineColor(overlay ? UI::CyberPrimaryFixed : UI::CyberOutlineVariant);
    window.draw(mainPanel);

    const float pad = 20.0f;
    const float cx = left + pad;
    const float cwInner = mainW - pad * 2;

    sf::RectangleShape bar(sf::Vector2f(mainW, 28));
    bar.setPosition(left, top);
    bar.setFillColor(UI::CyberSurfaceContainerHigh);
    bar.setOutlineThickness(1.0f);
    bar.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(bar);
    drawText("TETRISTE // PROTOCOLS", cx, top + 7, UI::CodeSize::MD, UI::CyberPrimary, true);
    std::string ver = "BUILD v1.4";
    drawText(ver, left + mainW - textWidth(ver, UI::CodeSize::SM) - pad, top + 8, UI::CodeSize::SM, UI::MutedText);

    const float footH = 48.0f;
    const float warnBlockH = 36.0f;
    float footY = top + panelH - footH;
    float warnY = footY - warnBlockH;
    float bodyStart = top + 84;
    float bodyBottom = warnY - 10;
    float bodyAvail = std::max(220.0f, bodyBottom - bodyStart);
    const float secGap = std::max(16.0f, std::min(24.0f, bodyAvail * 0.032f));
    const float lineH = 23.0f;
    float notesH = std::max(88.0f, bodyAvail * 0.17f);
    float secBlock = bodyAvail - notesH - secGap * 3.0f;
    const float secRatios[] = {0.24f, 0.30f, 0.24f, 0.14f};
    float secHeights[4];
    for (int i = 0; i < 4; ++i) secHeights[i] = secBlock * secRatios[i];
    float notesY = bodyBottom - notesH;

    float py = top + 40;
    sf::RectangleShape titlePip(sf::Vector2f(3, 14));
    titlePip.setPosition(cx, py + 4);
    titlePip.setFillColor(UI::CyberPrimaryFixed);
    window.draw(titlePip);
    drawText("SYSTEM PROTOCOLS", cx + 10, py, UI::CodeSize::XL, UI::CyberPrimaryFixed, true);
    sf::RectangleShape titleLine(sf::Vector2f(220, 2));
    titleLine.setPosition(cx + 10, py + 34);
    titleLine.setFillColor(UI::CyberPrimaryFixed);
    window.draw(titleLine);

    py = bodyStart;

    auto secBand = [&](float sy, float sh) {
        sf::RectangleShape band(sf::Vector2f(cwInner, sh));
        band.setPosition(cx, sy);
        band.setFillColor(sf::Color(7, 13, 20, 170));
        band.setOutlineThickness(1.0f);
        band.setOutlineColor(sf::Color(UI::CyberOutlineVariant.r, UI::CyberOutlineVariant.g, UI::CyberOutlineVariant.b, 100));
        window.draw(band);
        sf::RectangleShape stripe(sf::Vector2f(2, sh - 8));
        stripe.setPosition(cx, sy + 4);
        stripe.setFillColor(UI::CyberPrimaryFixed);
        window.draw(stripe);
    };
    auto secBadge = [&](const char *num, float sy) -> float {
        sf::RectangleShape pill(sf::Vector2f(36, 26));
        pill.setPosition(cx + 10, sy + 8);
        pill.setFillColor(sf::Color(10, 24, 8, 230));
        pill.setOutlineThickness(1.0f);
        pill.setOutlineColor(UI::CyberPrimaryFixed);
        window.draw(pill);
        drawText(num, cx + 19, sy + 11, UI::CodeSize::LG, UI::CyberPrimaryFixed, true);
        return cx + 54;
    };
    auto ctrlChipRow = [&](float bx, float by, float bw,
                           const std::vector<std::string> &keys, const std::string &label,
                           const std::vector<std::string> &keys2 = {}, const std::string &label2 = "") {
        sf::RectangleShape box(sf::Vector2f(bw, 40));
        box.setPosition(bx, by);
        box.setFillColor(UI::CyberSurfaceContainer);
        box.setOutlineThickness(1.0f);
        box.setOutlineColor(UI::CyberOutlineVariant);
        window.draw(box);
        float kx = bx + 12;
        float ky = by + 9;
        for (size_t i = 0; i < keys.size(); ++i) {
            kx += drawKeyChip(keys[i], kx, ky) + 5;
            if (i + 1 < keys.size()) {
                drawText("/", kx, ky + 4, UI::CodeSize::SM, UI::MutedText);
                kx += 12;
            }
        }
        kx += 8;
        drawText(label, kx, ky + 4, UI::CodeSize::SM, UI::CyberText);
        if (!keys2.empty()) {
            float sepX = kx + textWidth(label, UI::CodeSize::SM) + 12;
            drawText("|", sepX, ky + 4, UI::CodeSize::SM, UI::MutedText);
            kx = sepX + 16;
            for (size_t i = 0; i < keys2.size(); ++i) {
                kx += drawKeyChip(keys2[i], kx, ky) + 5;
                if (i + 1 < keys2.size()) {
                    drawText("/", kx, ky + 4, UI::CodeSize::SM, UI::MutedText);
                    kx += 12;
                }
            }
            drawText(label2, kx + 8, ky + 4, UI::CodeSize::SM, UI::CyberText);
        }
    };

    float h1 = secHeights[0];
    secBand(py, h1);
    float tx = secBadge("01", py);
    drawWrappedLines({
        "PURGE 3+ CONSECUTIVE NODES IN THE RING BUFFER.",
        "MATCH BY COLOR OR BY SHAPE (WHICHEVER RUN IS LONGER).",
        "PURGED NODES AWARD SCORE AND STACK COMBO MULTIPLIERS."
    }, tx, py + 8, cwInner - 58, UI::CodeSize::MD, UI::CyberText, lineH);
    py += h1 + secGap;

    float h2 = secHeights[1];
    secBand(py, h2);
    tx = secBadge("02", py);
    drawText("NODE SHIFTING OPERANDS:", tx, py + 10, UI::CodeSize::MD, UI::CyberText);
    float rowY = py + 36;
    float rowGap = std::max(10.0f, (h2 - 88.0f) * 0.5f);
    ctrlChipRow(cx + 14, rowY, cwInner - 28, {"J", "A"}, "INSERT AT HEAD", {"K", "D"}, "INSERT AT TAIL");
    ctrlChipRow(cx + 14, rowY + 40 + rowGap, cwInner - 28, {"C", "S"}, "SHIFT RINGS — MATCHES PURGE IMMEDIATELY");
    py += h2 + secGap;

    float h3 = secHeights[2];
    secBand(py, h3);
    tx = secBadge("03", py);
    drawWrappedLines({
        "NODE_BUFFER HAS A FIXED CELL CAPACITY.",
        "REACHING CAPACITY WITHOUT PURGING TRIGGERS DE-RES (GAME OVER).",
        "PURGE MATCHES TO FREE SLOTS AND STAY IN THE SESSION."
    }, tx, py + 8, cwInner - 58, UI::CodeSize::MD, UI::CyberText, lineH);
    py += h3 + secGap;

    float h4 = secHeights[3];
    secBand(py, h4);
    tx = secBadge("04", py);
    float kx = tx;
    kx += drawKeyChip("+", kx, py + 10) + 8;
    drawText("INCREASE", kx, py + 14, UI::CodeSize::SM, UI::CyberText);
    kx += textWidth("INCREASE", UI::CodeSize::SM) + 14;
    kx += drawKeyChip("-", kx, py + 10) + 8;
    drawText("DECREASE VOLUME", kx, py + 14, UI::CodeSize::SM, UI::CyberText);
    drawWrappedLines({
        "SONIC FEEDBACK AIDS TIMING_SYNC DURING TRANSFERS."
    }, tx, py + 36, cwInner - 58, UI::CodeSize::SM, UI::MutedText, 20);

    sf::RectangleShape note(sf::Vector2f(cwInner, notesH));
    note.setPosition(cx, notesY);
    note.setFillColor(UI::CyberSurfaceLowest);
    note.setOutlineThickness(1.0f);
    note.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(note);
    sf::RectangleShape noteStripe(sf::Vector2f(2, notesH - 10));
    noteStripe.setPosition(cx, notesY + 5);
    noteStripe.setFillColor(UI::CyberPrimaryFixed);
    window.draw(noteStripe);
    drawText("OPERATIVE NOTES", cx + 14, notesY + 12, UI::CodeSize::MD, UI::CyberPrimaryFixed, true);
    drawWrappedLines({
        "COMBO MULTIPLIER RESETS IF A PURGE FAILS.",
        "C/S ROTATE SHAPE/COLOR LISTS — NOT THE RING ORDER.",
        "GREEN FRAME = NODES IN THE 3+ MATCH ARC."
    }, cx + 14, notesY + 32, cwInner - 20, UI::CodeSize::SM, UI::MutedText, 20);

    sf::RectangleShape warnSep(sf::Vector2f(cwInner, 1));
    warnSep.setPosition(cx, warnY - 8);
    warnSep.setFillColor(UI::CyberOutlineVariant);
    window.draw(warnSep);
    drawText("[WARN: BUFFER OVERFLOW = SESSION DE-RES]", cx, warnY, UI::CodeSize::SM, UI::CyberSecondaryFixed);
    sf::RectangleShape foot(sf::Vector2f(mainW, 44));
    foot.setPosition(left, footY);
    foot.setFillColor(UI::CyberBg);
    foot.setOutlineThickness(1.0f);
    foot.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(foot);
    drawKeyChip("A", cx, footY + 10);
    drawText("PRESS TO ACKNOWLEDGE", cx + 38, footY + 16, UI::CodeSize::SM, UI::CyberText);
    std::string escLbl = overlay ? "ESC: RESUME" : "ESC: RETURN";
    drawText(escLbl, left + mainW - textWidth(escLbl, UI::CodeSize::SM) - pad, footY + 16, UI::CodeSize::SM, UI::MutedText);
    if ((int)(cursorBlinkTimer * 2) % 2 == 0)
        drawText("_", left + mainW - pad - 8, footY + 16, UI::CodeSize::SM, UI::MutedText);

    float sideX = left + mainW + 10;
    sf::RectangleShape sideBg(sf::Vector2f(sideW, panelH));
    sideBg.setPosition(sideX, top);
    sideBg.setFillColor(UI::CyberSurface);
    sideBg.setOutlineThickness(1.0f);
    sideBg.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(sideBg);
    float diagramH = panelH * 0.38f;
    float vizH = panelH - diagramH - 8;
    drawProtocolDiagram(sideX, top, sideW, diagramH);
    drawLogicVisualizer(sideX, top + diagramH + 8, sideW, vizH);
}

void Renderer::drawProtocolsOverlay() {
    sf::RectangleShape dim(sf::Vector2f(window.getSize().x, window.getSize().y));
    dim.setFillColor(sf::Color(0, 0, 0, 210));
    window.draw(dim);
    drawProtocolsPage(true);
}

void Renderer::drawAboutPage() { drawProtocolsPage(false); }

// --- ACHIEVEMENTS / SETTINGS ---

void Renderer::drawAchievementsPage() {
    if (!hasFont()) return;
    float left = contentLeft() + UI::MarginMd;
    float cardW = contentWidth() - UI::MarginMd * 2;
    const float cardH = 72.0f;
    const float cardGap = 16.0f;
    const size_t achCount = 3;

    UserProfile user = userManager.getCurrentUserProfile();
    std::vector<Achievement> allAchs;
    allAchs.push_back({"first_match", "PURGE_INITIATE", "Clear your first sequence of nodes.", false});
    allAchs.push_back({"vet_purger", "VETERAN_PURGER", "Purge a total of 100 nodes.", false});
    allAchs.push_back({"elite_op", "ELITE_OPERATIVE", "Complete a match on ELITE difficulty.", false});

    float titleH = 36.0f;
    float listH = titleH + achCount * (cardH + cardGap);
    float blockTop = contentTop() + (contentHeight() - listH) * 0.5f;

    sf::RectangleShape frame(sf::Vector2f(cardW + 24, listH + 24));
    frame.setPosition(left - 12, blockTop - 12);
    frame.setFillColor(UI::CyberSurface);
    frame.setOutlineThickness(1.0f);
    frame.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(frame);

    drawText("PROTOCOL_ARCHIVE", left, blockTop, UI::CodeSize::XL, UI::CyberPrimaryFixed, true);

    for (size_t i = 0; i < allAchs.size(); ++i) {
        bool unlocked = std::find(user.achievements.begin(), user.achievements.end(), allAchs[i].id) != user.achievements.end();
        float cy = blockTop + titleH + i * (cardH + cardGap);
        sf::RectangleShape card(sf::Vector2f(cardW, cardH));
        card.setPosition(left, cy);
        card.setFillColor(UI::CyberSurfaceLowest);
        card.setOutlineThickness(1.0f);
        card.setOutlineColor(unlocked ? UI::CyberPrimaryFixed : UI::CyberOutlineVariant);
        window.draw(card);
        drawText(allAchs[i].name, left + 16, cy + 12, UI::CodeSize::LG, unlocked ? UI::CyberPrimaryFixed : UI::MutedText, true);
        drawText(allAchs[i].description, left + 16, cy + 38, UI::CodeSize::SM, UI::CyberText);
        if (unlocked) drawText("UNLOCKED", left + cardW - 90, cy + 26, UI::CodeSize::SM, UI::CyberPrimaryFixed);
    }
    drawText("ESC: RETURN", contentLeft() + contentWidth() / 2 - 40, blockTop + listH + 16,
             UI::CodeSize::MD, UI::MutedText);
}

void Renderer::drawSettingsPage() {
    if (!hasFont()) return;
    float cx = contentLeft() + contentWidth() / 2.0f;
    float cy = contentTop() + contentHeight() / 2.0f;

    sf::RectangleShape border(sf::Vector2f(contentWidth() - 48, 320));
    border.setPosition(contentLeft() + 24, cy - 160);
    border.setFillColor(UI::CyberSurfaceLowest);
    border.setOutlineThickness(1.0f);
    border.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(border);

    drawText("SYSTEM_CONFIGURATION", cx - textWidth("SYSTEM_CONFIGURATION", UI::CodeSize::XL, true) / 2,
             cy - 120, UI::CodeSize::XL, UI::CyberPrimaryFixed, true);

    float vol = assets.getVolume();
    int blocks = (int)(vol / 10.0f);
    std::string volStr = "AUDIO_GAIN: [";
    for (int i = 0; i < 10; ++i) volStr += (i < blocks) ? "■" : ".";
    volStr += "] " + std::to_string((int)vol) + "%";
    drawText(volStr, cx - textWidth(volStr, UI::CodeSize::LG) / 2, cy - 20, UI::CodeSize::LG, UI::CyberPrimaryFixed);

    drawText("[+] GAIN_UP  |  [-] GAIN_DOWN", cx - 100, cy + 40, UI::CodeSize::MD, UI::CyberPrimaryFixed);
    drawText("ESC: SAVE_AND_EXIT", cx - 70, cy + 80, UI::CodeSize::SM, UI::MutedText);
}

// --- PLAYING HUD ---

void Renderer::drawHudSection(const std::string &title, float x, float y, float w) {
    const float padTop = 11.0f;
    const float padX = 12.0f;
    float titleY = y + padTop;
    sf::RectangleShape pip(sf::Vector2f(3, 11));
    pip.setPosition(x + padX, titleY + 1);
    pip.setFillColor(UI::CyberPrimaryFixed);
    window.draw(pip);
    drawText(title, x + padX + 8, titleY, UI::CodeSize::SM, UI::CyberPrimaryFixed, true);
    sf::RectangleShape line(sf::Vector2f(w - padX * 2 - 8, 1));
    line.setPosition(x + padX + 8, titleY + 18);
    line.setFillColor(sf::Color(UI::CyberOutlineVariant.r, UI::CyberOutlineVariant.g, UI::CyberOutlineVariant.b, 160));
    window.draw(line);
}

int Renderer::playfieldCapacity() const {
    float sx, sy, spacingX, spacingY, sbX, playX, playY, playW, playH;
    getPlayLayout(sx, sy, spacingX, spacingY, sbX, playX, playY, playW, playH);
    int cols = std::max(1, (int)((playW - UI::PlayGridMarginX * 2) / spacingX));
    float gridH = playH - UI::PlayLabelBand - UI::PlayGridMarginY - 6.0f;
    int rows = std::max(1, (int)(gridH / spacingY));
    int cap = cols * rows;
    if (difficulty == Difficulty::VETERAN) cap = (int)(cap * 0.8f);
    else if (difficulty == Difficulty::ELITE) cap = (int)(cap * 0.6f);
    return std::max(cap, 12);
}

void Renderer::drawHudSidebar(Game &game, Piece *nextPiece, float sidebarX, float viewHeight) {
    if (!hasFont()) return;
    float u = uiScale();
    const float lbAvatarScale = 0.17f;
    const float lbIconPx = (float)UI::PieceTexPx * lbAvatarScale;
    float uiX = sidebarX + 14;
    float top = UI::HeaderH + 6;
    float panelW = (float)window.getSize().x - sidebarX - 12;
    float panelH = viewHeight - top - UI::FooterH - 4;
    const float hintH = 54.0f;
    float innerTop = top + 12;
    float innerPad = 6.0f;
    float stackH = panelH - innerPad * 2.0f - hintH;
    const int kStackSections = 5;
    float secH = stackH / (float)kStackSections;
    int capacity = playfieldCapacity();
    float fillPct = (capacity > 0) ? (float)game.piecesCount / (float)capacity : 0.0f;
    fillPct = std::min(1.0f, fillPct);
    std::string curPseudo = userManager.getCurrentUserPseudo();

    sf::RectangleShape frame(sf::Vector2f(panelW, panelH));
    frame.setPosition(sidebarX + 2, top);
    frame.setFillColor(sf::Color(3, 7, 11, 252));
    frame.setOutlineThickness(2.0f);
    frame.setOutlineColor(UI::CyberPrimaryFixed);
    window.draw(frame);
    sf::RectangleShape frameIn(sf::Vector2f(panelW - 6, panelH - 6));
    frameIn.setPosition(sidebarX + 5, top + 3);
    frameIn.setFillColor(sf::Color::Transparent);
    frameIn.setOutlineThickness(1.0f);
    frameIn.setOutlineColor(sf::Color(UI::CyberPrimaryFixed.r, UI::CyberPrimaryFixed.g, UI::CyberPrimaryFixed.b, 50));
    window.draw(frameIn);

    const float secHeaderH = 32.0f;
    const float contentPad = 14.0f;
    const float contentX = uiX + contentPad;

    auto sectionBg = [&](float sy, float sh, bool accent = false) {
        float bandW = panelW - contentPad * 2;
        sf::RectangleShape band(sf::Vector2f(bandW, sh - 10));
        band.setPosition(contentX, sy + 5);
        band.setFillColor(accent ? sf::Color(12, 22, 16, 210) : sf::Color(7, 13, 20, 200));
        band.setOutlineThickness(1.0f);
        band.setOutlineColor(accent ? sf::Color(UI::CyberWarning.r, UI::CyberWarning.g, UI::CyberWarning.b, 100)
                                    : UI::CyberOutlineVariant);
        window.draw(band);
        sf::RectangleShape stripe(sf::Vector2f(2, sh - 14));
        stripe.setPosition(contentX, sy + 7);
        stripe.setFillColor(accent ? UI::CyberWarning : UI::CyberPrimaryFixed);
        window.draw(stripe);
    };

    auto bufferTone = [&]() -> sf::Color {
        if (fillPct > 0.85f) return UI::NeonPink;
        if (fillPct > 0.55f) return UI::CyberWarning;
        return UI::CyberPrimaryFixed;
    };

    float y = innerTop;

    sectionBg(y, secH);
    drawHudSection("MISSION SCORE", contentX, y, panelW - contentPad * 2);
    const float scorePadX = 22.0f;
    std::string scoreStr = std::to_string(game.score);
    sf::Text scoreTxt(scoreStr, fontBold(), (unsigned)(34 * u));
    scoreTxt.setPosition(contentX + scorePadX, y + secHeaderH + 6);
    scoreTxt.setFillColor(sf::Color::White);
    window.draw(scoreTxt);
    float ptsX = contentX + scorePadX + scoreTxt.getLocalBounds().width + 8;
    drawText("PTS", ptsX, y + secHeaderH + 20, UI::CodeSize::SM, UI::MutedText);
    if (game.globalComboMultiplier > 1) {
        std::string combo = "x" + std::to_string(game.globalComboMultiplier);
        sf::RectangleShape comboChip(sf::Vector2f(52, 18));
        comboChip.setPosition(contentX + panelW - contentPad * 2 - 58, y + secHeaderH + 10);
        comboChip.setFillColor(sf::Color(40, 32, 0, 220));
        comboChip.setOutlineThickness(1.0f);
        comboChip.setOutlineColor(UI::CyberWarning);
        window.draw(comboChip);
        drawText(combo, contentX + panelW - contentPad * 2 - 48, y + secHeaderH + 12, UI::CodeSize::XS, UI::CyberWarning);
    }
    sf::RectangleShape scoreLine(sf::Vector2f(panelW - contentPad * 2 - scorePadX - 8, 2));
    scoreLine.setPosition(contentX + scorePadX, y + secH - 14);
    scoreLine.setFillColor(bufferTone());
    window.draw(scoreLine);
    y += secH;

    sectionBg(y, secH);
    drawHudSection("OPERATOR", contentX, y, panelW - contentPad * 2);
    UserProfile cur = userManager.getCurrentUserProfile();
    float opInnerY = y + secHeaderH + 6;
    float opInnerH = secH - secHeaderH - 12;
    float opBoxW = panelW - contentPad * 2 - 8;
    float opBoxLeft = contentX + 4;
    sf::RectangleShape opBox(sf::Vector2f(opBoxW, opInnerH));
    opBox.setPosition(opBoxLeft, opInnerY);
    opBox.setFillColor(UI::CyberSurfaceLowest);
    opBox.setOutlineThickness(1.0f);
    opBox.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(opBox);
    sf::RectangleShape opTop(sf::Vector2f(opBoxW, 1));
    opTop.setPosition(opBoxLeft, opInnerY);
    opTop.setFillColor(sf::Color(UI::CyberPrimaryFixed.r, UI::CyberPrimaryFixed.g, UI::CyberPrimaryFixed.b, 40));
    window.draw(opTop);
    const float opAvScale = 0.40f * u;
    const float avSz = (float)UI::PieceTexPx * opAvScale;
    const float avGap = 12.0f;
    const float blockH = 52.0f;
    float avX = opBoxLeft + 10;
    float avY = opInnerY + (opInnerH - blockH) * 0.5f + 2.0f;
    sf::RectangleShape avFrame(sf::Vector2f(avSz + 4, avSz + 4));
    avFrame.setPosition(avX - 2, avY - 2);
    avFrame.setFillColor(sf::Color(4, 8, 12));
    avFrame.setOutlineThickness(1.0f);
    avFrame.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(avFrame);
    if (assets.hasTexture(cur.avatarId)) {
        sf::Sprite av(assets.getTexture(cur.avatarId));
        av.setScale(opAvScale, opAvScale);
        av.setPosition(avX, avY);
        window.draw(av);
    }
    float opTextX = avX + avSz + avGap;
    std::string pseudo = curPseudo.empty() ? "OPERATOR" : curPseudo;
    if ((int)pseudo.size() > 14) pseudo = pseudo.substr(0, 13) + "~";
    drawText(pseudo, opTextX, avY + 2, UI::CodeSize::MD, UI::CyberPrimaryFixed);
    drawText("BUFFER HEADROOM", opTextX, avY + 20, UI::CodeSize::XS, UI::MutedText);
    float barW = opBoxLeft + opBoxW - opTextX - 10;
    float intFill = std::max(0.04f, 1.0f - fillPct);
    sf::RectangleShape intBg(sf::Vector2f(barW, 7));
    intBg.setPosition(opTextX, avY + 34);
    intBg.setFillColor(UI::CyberSurfaceContainerHigh);
    window.draw(intBg);
    sf::Color intCol = bufferTone();
    sf::RectangleShape intBar(sf::Vector2f(barW * intFill, 7));
    intBar.setPosition(opTextX, avY + 34);
    intBar.setFillColor(intCol);
    window.draw(intBar);
    std::string opScores = "LAST " + std::to_string(cur.lastScore) + "  BEST " + std::to_string(cur.record);
    drawText(opScores, opTextX, avY + 46, UI::CodeSize::XS, UI::CyberWarning);
    y += secH;

    sectionBg(y, secH);
    float rowY = y;
    float gutter = 10.0f;
    float rowW = panelW - contentPad * 2 - 8;
    float halfW = (rowW - gutter) / 2.0f;
    float capX = contentX + 4;
    float nextX = capX + halfW + gutter;
    drawHudSection("BUFFER FILL", capX, rowY, halfW);
    drawHudSection("NEXT NODE", nextX, rowY, halfW);

    float meterTop = rowY + secHeaderH + 6;
    float meterH = secH - secHeaderH - 16;
    sf::RectangleShape vMeter(sf::Vector2f(18, meterH));
    vMeter.setPosition(capX + 8, meterTop);
    vMeter.setOutlineThickness(1.0f);
    vMeter.setOutlineColor(UI::CyberOutlineVariant);
    vMeter.setFillColor(UI::CyberSurfaceContainer);
    window.draw(vMeter);
    for (int t = 1; t <= 4; ++t) {
        float ty = meterTop + meterH * (1.0f - t * 0.2f);
        sf::RectangleShape tick(sf::Vector2f(22, 1));
        tick.setPosition(capX + 9, ty);
        tick.setFillColor(sf::Color(UI::CyberOutlineVariant.r, UI::CyberOutlineVariant.g, UI::CyberOutlineVariant.b, 120));
        window.draw(tick);
    }
    sf::Color fc = bufferTone();
    float vFillH = (meterH - 4) * fillPct;
    sf::RectangleShape vFill(sf::Vector2f(14, vFillH));
    vFill.setPosition(capX + 10, meterTop + 2 + (meterH - 4) - vFillH);
    vFill.setFillColor(fc);
    window.draw(vFill);
    std::string capLbl = std::to_string(game.piecesCount) + "/" + std::to_string(capacity);
    drawText(capLbl, capX + 32, meterTop + 6, UI::CodeSize::MD, fc, true);
    drawText(std::to_string((int)(fillPct * 100)) + "%", capX + 32, meterTop + 26, UI::CodeSize::SM, UI::MutedText);

    const float nextPreviewFrac = 0.58f;
    float nextBoxW = (halfW - 12) * nextPreviewFrac;
    float nextBoxH = meterH * nextPreviewFrac;
    float nextBoxLeft = nextX + 8;
    float nextBoxTop = meterTop + 4;
    sf::RectangleShape nextBox(sf::Vector2f(nextBoxW, nextBoxH));
    nextBox.setPosition(nextBoxLeft, nextBoxTop);
    nextBox.setFillColor(UI::CyberSurfaceLowest);
    nextBox.setOutlineThickness(1.0f);
    nextBox.setOutlineColor(UI::CyberOutlineVariant);
    window.draw(nextBox);
    float ncx = nextBoxLeft + nextBoxW * 0.5f;
    float ncy = nextBoxTop + nextBoxH * 0.5f;
    if (nextPiece) {
        float cell = std::min(nextBoxW, nextBoxH) - 14.0f;
        float nScale = (cell - 2.0f * UI::PlayCellInset) / (float)UI::PieceTexPx;
        drawNodeSlot(ncx, ncy, cell, true);
        drawFancyPiece(*nextPiece, ncx, ncy, nScale, false, false);
    } else {
        drawText("---", ncx - 10, ncy - 8, UI::CodeSize::MD, UI::MutedText);
    }
    int previewCount = 0;
    if (matchHighlightEnabled && nextPiece && game.piecesCount >= 2) {
        std::vector<Piece *> preview;
        game.collectMatchPreview(&game, nextPiece, preview);
        previewCount = (int)preview.size();
    }
    std::string prevLbl = previewCount >= 2 ? ("MATCH " + std::to_string(previewCount) + " READY")
                                            : "NO MATCH YET";
    sf::Color prevCol = previewCount >= 2 ? UI::CyberPrimaryFixed : UI::MutedText;
    drawText(prevLbl, nextX + 8, rowY + secH - 16, UI::CodeSize::XS, prevCol);
    y += secH;

    sectionBg(y, secH, true);
    drawHudSection("TOP OPERATORS", contentX, y, panelW - contentPad * 2);
    auto topRecs = userManager.getTopRecords(3);
    float rowStep = (secH - secHeaderH - 6) / 3.0f;
    const char *ranks[] = {"01", "02", "03"};
    float nameX = contentX + 46;
    float rowW2 = panelW - contentPad * 2 - 8;
    for (int i = 0; i < 3; ++i) {
        float ly = y + secHeaderH + 4 + i * rowStep;
        float rowH = rowStep - 5;
        bool hasEntry = i < (int)topRecs.size();
        bool isSelf = hasEntry && topRecs[i].pseudo == curPseudo;
        bool isLeader = i == 0 && hasEntry;

        sf::RectangleShape rowBg(sf::Vector2f(rowW2, rowH));
        rowBg.setPosition(contentX + 4, ly);
        if (!hasEntry) {
            rowBg.setFillColor(sf::Color(6, 10, 14, 160));
            rowBg.setOutlineColor(sf::Color(UI::CyberOutlineVariant.r, UI::CyberOutlineVariant.g, UI::CyberOutlineVariant.b, 60));
        } else if (isLeader) {
            rowBg.setFillColor(sf::Color(22, 30, 14, 230));
            rowBg.setOutlineColor(UI::CyberWarning);
        } else if (isSelf) {
            rowBg.setFillColor(sf::Color(10, 20, 28, 220));
            rowBg.setOutlineColor(UI::CyberPrimaryFixed);
        } else {
            rowBg.setFillColor(UI::CyberSurfaceLowest);
            rowBg.setOutlineColor(UI::CyberOutlineVariant);
        }
        rowBg.setOutlineThickness(isLeader ? 2.0f : 1.0f);
        window.draw(rowBg);
        if (isLeader) {
            sf::RectangleShape accent(sf::Vector2f(3, rowH - 2));
            accent.setPosition(contentX + 5, ly + 1);
            accent.setFillColor(UI::CyberWarning);
            window.draw(accent);
        }

        std::string rankLbl = std::string("[") + ranks[i] + "]";
        drawText(rankLbl, contentX + 10, ly + (rowH - 12) * 0.5f - 1, UI::CodeSize::XS, UI::MutedText);
        if (!hasEntry) {
            drawText("---", nameX, ly + (rowH - 12) * 0.5f, UI::CodeSize::SM, UI::MutedText);
            continue;
        }
        float iconY = ly + (rowH - lbIconPx) * 0.5f;
        if (assets.hasTexture(topRecs[i].avatarId)) {
            sf::Sprite av(assets.getTexture(topRecs[i].avatarId));
            av.setScale(lbAvatarScale, lbAvatarScale);
            av.setPosition(contentX + 30, iconY);
            window.draw(av);
        }
        sf::Color nameCol = isLeader ? UI::CyberWarning : (isSelf ? UI::CyberPrimaryFixed : UI::CyberText);
        std::string name = topRecs[i].pseudo;
        if ((int)name.size() > 11) name = name.substr(0, 10) + "~";
        drawText(name, nameX, ly + (rowH - 14) * 0.5f, UI::CodeSize::SM, nameCol);
        std::string sc = std::to_string(topRecs[i].record);
        drawText(sc, contentX + rowW2 - textWidth(sc, UI::CodeSize::SM) - 6, ly + (rowH - 14) * 0.5f,
                 UI::CodeSize::SM, isLeader ? UI::CyberWarning : UI::CyberText);
    }
    y += secH;

    sectionBg(y, secH);
    drawHudSection("SESSION TELEMETRY", contentX, y, panelW - contentPad * 2);
    UserProfile p = userManager.getCurrentUserProfile();
    float telY = y + secHeaderH + 4;
    float telW = panelW - contentPad * 2 - 8;
    float colW = telW / 3.0f;
    auto telCell = [&](const std::string &lbl, const std::string &val, float cx, sf::Color vc) {
        sf::RectangleShape cell(sf::Vector2f(colW - 6, secH - secHeaderH - 18));
        cell.setPosition(cx, telY);
        cell.setFillColor(UI::CyberSurfaceContainer);
        cell.setOutlineThickness(1.0f);
        cell.setOutlineColor(UI::CyberOutlineVariant);
        window.draw(cell);
        drawText(lbl, cx + 8, telY + 6, UI::CodeSize::XS, UI::MutedText);
        drawText(val, cx + 8, telY + 22, UI::CodeSize::LG, vc);
    };
    telCell("SESSIONS", std::to_string(p.matchesPlayed), contentX + 4, UI::CyberText);
    telCell("PURGED", std::to_string(p.nodesPurged), contentX + 4 + colW, UI::CyberWarning);
    telCell("IN BUFFER", std::to_string(game.piecesCount), contentX + 4 + colW * 2, UI::CyberPrimaryFixed);
    std::string status = (fillPct > 0.85f) ? "CRITICAL" : (fillPct > 0.55f ? "ELEVATED" : "STABLE");
    sf::Color statusCol = bufferTone();
    sf::RectangleShape statusPill(sf::Vector2f(88, 16));
    statusPill.setPosition(contentX + telW - 92, y + secH - 22);
    statusPill.setFillColor(sf::Color(statusCol.r, statusCol.g, statusCol.b, 40));
    statusPill.setOutlineThickness(1.0f);
    statusPill.setOutlineColor(statusCol);
    window.draw(statusPill);
    drawText(status, contentX + telW - 84, y + secH - 20, UI::CodeSize::XS, statusCol);
    y += secH;

    sf::RectangleShape hintBox(sf::Vector2f(panelW - contentPad * 2, hintH - 4));
    hintBox.setPosition(contentX, top + panelH - hintH - innerPad + 2);
    hintBox.setFillColor(UI::CyberBg);
    hintBox.setOutlineThickness(1.0f);
    hintBox.setOutlineColor(UI::CyberPrimaryFixed);
    window.draw(hintBox);
    float hintY = top + panelH - hintH - innerPad + 10;
    drawText("J/A HEAD", contentX + 10, hintY, UI::CodeSize::XS, UI::CyberPrimaryFixed);
    drawText("K/D TAIL", contentX + 76, hintY, UI::CodeSize::XS, UI::CyberPrimaryFixed);
    drawText("C SHAPE", contentX + 10, hintY + 16, UI::CodeSize::XS, UI::MutedText);
    drawText("S COLOR", contentX + 76, hintY + 16, UI::CodeSize::XS, UI::MutedText);
    drawText("ESC PAUSE", contentX + panelW - contentPad * 2 - 68, hintY + 8, UI::CodeSize::XS, UI::MutedText);
}

void Renderer::drawPlayingHud(Game &game, Piece *nextPiece, GameState state) {
    float viewWidth = (float)window.getSize().x;
    float viewHeight = (float)window.getSize().y;

    drawTerminalShell(ShellVariant::Minimal, NavItem::LOGIC, HeaderStyle::Default, 0);

    float startX, startY, spacingX, spacingY, sidebarX, playX, playY, playW, playH;
    getPlayLayout(startX, startY, spacingX, spacingY, sidebarX, playX, playY, playW, playH);

    drawPlayfield(playX, playY, playW, playH);
    drawText("NODE_BUFFER (RING ORDER)", playX + 14, playY + 6, UI::CodeSize::SM, UI::MutedText);
    std::string activeLbl = std::to_string(game.piecesCount) + " NODES";
    drawText(activeLbl, playX + playW - textWidth(activeLbl, UI::CodeSize::SM) - 14, playY + 6,
             UI::CodeSize::SM, UI::CyberPrimaryFixed);

    std::unordered_set<Piece *> purgePreview;
    if (matchHighlightEnabled && nextPiece != nullptr && game.piecesCount >= 2) {
        std::vector<Piece *> preview;
        game.collectMatchPreview(&game, nextPiece, preview);
        for (Piece *p : preview) purgePreview.insert(p);
    }

    if (game.head != nullptr) {
        Piece *current = game.head;
        float currentX = startX, currentY = startY;
        int count = 0;
        float u = uiScale();
        float pieceScale = UI::piecePlayScale(u);
        float punch = pieceScale;
        if (insertionTimer > 0) {
            float bump = 0.05f * (insertionTimer / 0.4f);
            punch = std::min(pieceScale + bump, (UI::playCellSize(u) - 4.0f) / (float)UI::PieceTexPx);
        }
        float gridOx, gridOy;
        UI::playGridOrigin(playX, playY, gridOx, gridOy);
        int cols = std::max(1, (int)((playW - UI::PlayGridMarginX * 2) / spacingX));
        float gridBottom = gridOy + (int)((playH - UI::PlayLabelBand - UI::PlayGridMarginY - 6) / spacingY) * spacingY;
        do {
            bool isTarget = purgePreview.count(current) > 0;
            drawFancyPiece(*current, currentX, currentY, punch, isTarget, false);
            current = current->nextPiece;
            count++;
            if (count % cols == 0) {
                currentX = startX;
                currentY += spacingY;
            } else {
                currentX += spacingX;
            }
            if (currentY > gridBottom - spacingY * 0.5f) break;
        } while (current != game.head && count < game.piecesCount);
    }

    drawHudSidebar(game, nextPiece, sidebarX, viewHeight);

    if (state == GameState::PAUSED) {
        float cx = playX + playW / 2, cy = playY + playH / 2;
        sf::RectangleShape overlay(sf::Vector2f(360, 120));
        overlay.setPosition(cx - 180, cy - 60);
        overlay.setFillColor(UI::CyberBg);
        overlay.setOutlineThickness(1.0f);
        overlay.setOutlineColor(UI::CyberPrimaryFixed);
        window.draw(overlay);
        drawText("SYSTEM_PAUSED", cx - textWidth("SYSTEM_PAUSED", UI::CodeSize::XL, true) / 2, cy - 20,
                 UI::CodeSize::XL, UI::CyberPrimaryFixed, true);
        drawText("ESC: RESUME", cx - 50, cy + 20, UI::CodeSize::SM, UI::MutedText);
    }
    if (showAbout) drawProtocolsOverlay();
}

void Renderer::drawLeaderboard(float x, float y) {
    float w = (float)window.getSize().x - x - 14;
    if (w > 210) w = 210;
    drawCard("ELITE_OPERATORS (TOP 3)", sf::Vector2f(x, y), sf::Vector2f(w, 100));
    auto top = userManager.getTopRecords(3);
    for (size_t i = 0; i < top.size(); ++i) {
        if (assets.hasTexture(top[i].avatarId)) {
            sf::Sprite av(assets.getTexture(top[i].avatarId));
            av.setScale(0.45f, 0.45f);
            av.setPosition(x + 8, y + 22 + i * 24);
            window.draw(av);
        }
        drawText(top[i].pseudo, x + 32, y + 22 + i * 24, UI::CodeSize::SM,
                 i == 0 ? UI::CyberWarning : UI::CyberPrimaryFixed);
        drawText(std::to_string(top[i].record), x + 150, y + 22 + i * 24, UI::CodeSize::SM, UI::CyberText);
    }
}

void Renderer::drawStats(float x, float y) {
    float w = (float)window.getSize().x - x - 14;
    if (w > 210) w = 210;
    drawCard("LIFETIME_STATS", sf::Vector2f(x, y), sf::Vector2f(w, 80));
    UserProfile p = userManager.getCurrentUserProfile();
    drawText("SESSIONS: " + std::to_string(p.matchesPlayed), x + 10, y + 20, UI::CodeSize::XS, UI::CyberText);
    drawText("PURGED:   " + std::to_string(p.nodesPurged), x + 10, y + 34, UI::CodeSize::XS, UI::CyberText);
    drawText("LAST:     " + std::to_string(p.lastScore), x + 10, y + 48, UI::CodeSize::XS, UI::CyberPrimaryFixed);
    drawText("BEST:     " + std::to_string(p.record), x + 10, y + 62, UI::CodeSize::XS, UI::CyberWarning);
}

void Renderer::drawGameOver(int s, int nodesInBuffer) {
    if (!hasFont()) return;
    float cx = (float)window.getSize().x / 2.0f;
    float cy = (float)window.getSize().y / 2.0f;

    int capacity = playfieldCapacity();

    sf::RectangleShape panel(sf::Vector2f(540, 340));
    panel.setPosition(cx - 270, cy - 170);
    panel.setFillColor(UI::CyberSurfaceLowest);
    panel.setOutlineThickness(2.0f);
    panel.setOutlineColor(UI::CyberWarning);
    window.draw(panel);

    sf::RectangleShape stripe(sf::Vector2f(540, 3));
    stripe.setPosition(cx - 270, cy - 170);
    stripe.setFillColor(UI::NeonPink);
    window.draw(stripe);

    drawText("BUFFER_OVERFLOW", cx - textWidth("BUFFER_OVERFLOW", UI::CodeSize::XL, true) / 2, cy - 100,
             UI::CodeSize::XL, UI::CyberWarning, true);
    drawText("NODE_BUFFER EXCEEDED — SESSION DE-RES", cx - textWidth("NODE_BUFFER EXCEEDED — SESSION DE-RES", UI::CodeSize::SM) / 2,
             cy - 58, UI::CodeSize::SM, UI::MutedText);
    std::string capLine = "CAPACITY BREACH: " + std::to_string(nodesInBuffer) + " / " + std::to_string(capacity) + " NODES";
    drawText(capLine, cx - textWidth(capLine, UI::CodeSize::MD) / 2, cy - 24, UI::CodeSize::MD, UI::NeonPink);
    drawText("FINAL_SCORE: " + std::to_string(s), cx - textWidth("FINAL_SCORE: " + std::to_string(s), UI::CodeSize::LG) / 2,
             cy + 24, UI::CodeSize::LG, UI::CyberPrimaryFixed);
    drawText("SPACE: RETURN_TO_MENU", cx - textWidth("SPACE: RETURN_TO_MENU", UI::CodeSize::MD) / 2, cy + 72,
             UI::CodeSize::MD, UI::CyberPrimaryFixed);
}

void Renderer::handleTextInput(uint32_t uni) {
    if (uni == '\t' || uni == '\r' || uni == '\n') return;
    if (uni == 8) {
        if (authState == AuthState::PSEUDO) {
            if (!loginPseudo.empty()) loginPseudo.pop_back();
        } else if (authState == AuthState::PIN_ENTRY || authState == AuthState::PIN_SETUP) {
            if (!loginPin.empty()) loginPin.pop_back();
        }
    } else if (uni < 128) {
        if (authState == AuthState::PSEUDO) {
            if (loginPseudo.length() < 16) loginPseudo += (char)uni;
        } else if (authState == AuthState::PIN_ENTRY || authState == AuthState::PIN_SETUP) {
            if (std::isdigit(uni) && loginPin.length() < 4) loginPin += (char)uni;
        }
    }
}

void Renderer::addPopup(std::string c, sf::Vector2f p, sf::Color clr) {
    if (!hasFont()) return;
    FloatingText pop;
    pop.text.setFont(font());
    pop.text.setString(c);
    pop.text.setCharacterSize(22);
    pop.text.setFillColor(clr);
    UI::centerText(pop.text, p);
    pop.lifetime = pop.maxLifetime = 1.2f;
    pop.velocity = sf::Vector2f(0, -50);
    popups.push_back(pop);
}
