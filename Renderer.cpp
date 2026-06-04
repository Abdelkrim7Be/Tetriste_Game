#include "Renderer.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

Renderer::Renderer(sf::RenderWindow &win, AssetManager &asmgr, UserManager &umgr) 
    : window(win), assets(asmgr), userManager(umgr) {}

void Renderer::render(Game &game, Piece *nextPiece, GameState state) {
    float deltaTime = clock.restart().asSeconds();
    totalTime += deltaTime;
    cursorBlinkTimer += deltaTime;
    if (insertionTimer > 0) insertionTimer -= deltaTime;
    if (flashTimer > 0) flashTimer -= deltaTime;
    window.clear(UI::DeepNavy); 
    
    if (state == GameState::LOGIN) {
        drawLoginScreen();
    } else if (state == GameState::MENU) {
        drawMainMenu();
    } else if (state == GameState::DIFFICULTY_SELECT) {
        drawDifficultySelection();
    } else if (state == GameState::GAME_OVER) {
        drawGameOver(game.score);
    } else if (state == GameState::ACHIEVEMENTS) {
        drawAchievementsPage();
    } else if (state == GameState::SETTINGS) {
        drawSettingsPage();
    } else {
        sf::Vector2u winSize = window.getSize();
        float viewWidth = static_cast<float>(winSize.x);
        float viewHeight = static_cast<float>(winSize.y);
        float sidebarWidth = 240.0f;
        float sidebarX = viewWidth - sidebarWidth;

        // Subtle background grid
        sf::RectangleShape gridLine(sf::Vector2f(viewWidth, 1.0f));
        gridLine.setFillColor(sf::Color(255, 255, 255, 20));
        for (float y = 0; y < viewHeight; y += 40.0f) {
            gridLine.setPosition(0, y);
            window.draw(gridLine);
        }
        gridLine.setSize(sf::Vector2f(1.0f, viewHeight));
        for (float x = 0; x < viewWidth; x += 40.0f) {
            gridLine.setPosition(x, 0);
            window.draw(gridLine);
        }

        // Play area border pulse
        float alpha = 100 + 50 * std::sin(totalTime * 2.0f);
        sf::RectangleShape border(sf::Vector2f(sidebarX - 20.0f, viewHeight - 20.0f));
        border.setPosition(10.0f, 10.0f);
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineThickness(2.0f);
        border.setOutlineColor(sf::Color(UI::CyberTeal.r, UI::CyberTeal.g, UI::CyberTeal.b, (sf::Uint8)alpha));
        window.draw(border);

        // Draw board pieces
        if (game.head != nullptr) {
            Piece *current = game.head;
            float startX = 60.0f, startY = 60.0f;
            float currentX = startX, currentY = startY;
            float spacingX = 60.0f, spacingY = 80.0f;
            int count = 0;

            float punch = 1.0f;
            if (insertionTimer > 0) punch = 1.0f + 0.15f * (insertionTimer / 0.4f);

            do {
                bool isTarget = (nextPiece != nullptr) && (current->color == nextPiece->color || current->shape == nextPiece->shape);
                drawFancyPiece(*current, currentX, currentY, punch, isTarget);
                current = current->nextPiece;
                count++;
                currentX += spacingX;
                if (currentX > sidebarX - spacingX) {
                    currentX = startX;
                    currentY += spacingY;
                }
            } while (current != game.head && count < game.piecesCount);
        }
        
        // Popups
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

        // Sidebar
        drawGlassPanel(sf::Vector2f(sidebarWidth - 10, viewHeight - 20), sf::Vector2f(sidebarX, 10));
        if (assets.hasFont("main")) {
            float uiX = sidebarX + 15.0f;
            
            // Score Card
            drawCard("MISSION SCORE", sf::Vector2f(uiX, 20), sf::Vector2f(210, 80));
            sf::Text scoreText(std::to_string(game.score), assets.getFont("main"), 36);
            scoreText.setFillColor(UI::CyberTeal);
            scoreText.setPosition(uiX + 10, 45);
            window.draw(scoreText);

            // Player Card
            drawCard("OPERATOR: " + userManager.getCurrentUserPseudo(), sf::Vector2f(uiX, 110), sf::Vector2f(210, 60));
            UserProfile cur = userManager.getCurrentUserProfile();
            if (assets.hasTexture(cur.avatarId)) {
                sf::Sprite av(assets.getTexture(cur.avatarId));
                av.setScale(0.8f, 0.8f);
                av.setPosition(uiX + 165, 125);
                window.draw(av);
            }
            sf::Text recText("BEST: " + std::to_string(cur.record), assets.getFont("main"), 16);
            recText.setPosition(uiX + 10, 140);
            recText.setFillColor(sf::Color::Yellow);
            window.draw(recText);

            // Capacity Meter
            drawCard("CAPACITY", sf::Vector2f(uiX, 180), sf::Vector2f(210, 70));
            float startX = 60.0f, startY = 60.0f, spacingX = 60.0f, spacingY = 80.0f;
            int pPerRow = (int)((sidebarX - startX) / spacingX);
            if (pPerRow < 1) pPerRow = 1;
            int mRows = (int)((viewHeight - startY - 25.0f) / spacingY) + 1;
            int capacity = pPerRow * mRows;
            
            sf::RectangleShape barBg(sf::Vector2f(190, 10));
            barBg.setPosition(uiX + 10, 225);
            barBg.setFillColor(sf::Color(50, 50, 50));
            window.draw(barBg);

            float fillWidth = (capacity > 0) ? (190.0f * game.piecesCount / capacity) : 0;
            sf::RectangleShape barFill(sf::Vector2f(fillWidth, 10));
            barFill.setPosition(uiX + 10, 225);
            sf::Color barColor = UI::CyberTeal;
            if (game.piecesCount > capacity * 0.8f) barColor = UI::NeonPink;
            else if (game.piecesCount > capacity * 0.5f) barColor = sf::Color::Yellow;
            barFill.setFillColor(barColor);
            window.draw(barFill);

            // Next Piece
            drawCard("NEXT DATA", sf::Vector2f(uiX, 260), sf::Vector2f(210, 110));
            if (nextPiece != nullptr) {
                float pulse = 1.0f + 0.05f * std::sin(totalTime * 4.0f);
                drawFancyPiece(*nextPiece, uiX + 105, 330, pulse * 1.2f, false);
            }

            drawLeaderboard(uiX, 350.0f);
            drawStats(uiX, 475.0f);

            sf::Text hints("J/K: Insert  C/S: Shift\nA: Info  ESC: Pause", assets.getFont("main"), 14);
            hints.setPosition(uiX + 10, viewHeight - 40.0f);
            hints.setFillColor(UI::MutedText);
            window.draw(hints);
        }

        if (state == GameState::PAUSED) {
            sf::RectangleShape overlay(sf::Vector2f(viewWidth, viewHeight));
            overlay.setFillColor(sf::Color(0, 0, 0, 180));
            window.draw(overlay);
            if (assets.hasFont("main")) {
                sf::Text pText("SYSTEM PAUSED", assets.getFont("main"), 50);
                UI::centerText(pText, sf::Vector2f(viewWidth/2, viewHeight/2));
                window.draw(pText);
            }
        }
        if (showAbout) drawAboutPage();
    }

    if (flashTimer > 0) {
        sf::RectangleShape flash(sf::Vector2f(window.getSize().x, window.getSize().y));
        flash.setFillColor(sf::Color(255, 255, 255, (sf::Uint8)(100 * (flashTimer / 0.2f))));
        window.draw(flash);
    }

    window.display();
}

void Renderer::drawFancyPiece(Piece &piece, float x, float y, float scale, bool highlight) {
    std::string texName = getTextureName(piece.color, piece.shape);
    if (!assets.hasTexture(texName)) return;
    
    sf::Sprite sprite(assets.getTexture(texName));
    sf::FloatRect b = sprite.getLocalBounds();
    sprite.setOrigin(b.width/2, b.height/2);
    sprite.setPosition(x, y);

    // If highlighted, pulse scale slightly and increase outer glow
    float pulse = highlight ? 1.0f + 0.1f * std::sin(totalTime * 6.0f) : 1.0f;
    float finalScale = scale * pulse;
    sprite.setScale(finalScale, finalScale);

    // Subtle Outer Glow (Additive)
    int glowAlpha = highlight ? 180 : 100;
    sprite.setColor(sf::Color(255, 255, 255, glowAlpha));
    sprite.setScale(finalScale * 1.15f, finalScale * 1.15f);
    window.draw(sprite, sf::BlendAdd);

    // Main Draw
    sprite.setColor(sf::Color::White);
    sprite.setScale(finalScale, finalScale);
    window.draw(sprite);
}

void Renderer::drawCard(const std::string& title, sf::Vector2f pos, sf::Vector2f size) {
    sf::RectangleShape card = UI::createPanel(size, pos);
    card.setOutlineColor(sf::Color(UI::CyberTeal.r, UI::CyberTeal.g, UI::CyberTeal.b, 60));
    window.draw(card);

    if (assets.hasFont("main")) {
        sf::Text t(title, assets.getFont("main"), 12);
        t.setPosition(pos.x + 10, pos.y + 5);
        t.setFillColor(UI::CyberTeal);
        window.draw(t);
    }
}

void Renderer::drawGlassPanel(sf::Vector2f size, sf::Vector2f pos) {
    sf::RectangleShape p = UI::createPanel(size, pos);
    window.draw(p);
}

void Renderer::drawLoginScreen() {
    if (!assets.hasFont("main")) return;
    float cx = window.getSize().x / 2.0f, cy = window.getSize().y / 2.0f;

    sf::Text title("TETRISTE", assets.getFont("main"), 90);
    title.setFillColor(UI::CyberTeal);
    UI::centerText(title, sf::Vector2f(cx, 100));
    window.draw(title);

    if (authState == AuthState::AVATAR_SELECT) {
        drawAvatarSelection();
        return;
    }

    drawGlassPanel(sf::Vector2f(420, 320), sf::Vector2f(cx - 210, cy - 140));
    
    auto drawIn = [&](std::string lbl, std::string val, float y, bool active, bool isPassword) {
        sf::Text lText(lbl, assets.getFont("main"), 14);
        lText.setPosition(cx - 190, y - 30);
        lText.setFillColor(UI::MutedText);
        window.draw(lText);

        sf::RectangleShape box(sf::Vector2f(380, 45));
        box.setPosition(cx - 190, y - 5);
        box.setFillColor(sf::Color(10, 10, 20));
        box.setOutlineThickness(active ? 2.0f : 1.0f);
        box.setOutlineColor(active ? UI::CyberTeal : sf::Color(80, 80, 90));
        window.draw(box);

        std::string displayVal = val;
        if (isPassword) displayVal = std::string(val.length(), '*');
        if (active && (int)(cursorBlinkTimer * 2) % 2 == 0) displayVal += "_";
        
        sf::Text vText(displayVal, assets.getFont("main"), 22);
        vText.setPosition(cx - 180, y + 2);
        window.draw(vText);
    };

    if (authState == AuthState::PSEUDO) {
        drawIn("OPERATOR_ID (PSEUDO)", loginPseudo, cy - 20, true, false);
    } else if (authState == AuthState::PIN_ENTRY) {
        drawIn("ENTER 4-DIGIT PIN", loginPin, cy - 20, true, true);
    } else if (authState == AuthState::PIN_SETUP) {
        drawIn("NEW ACCOUNT: SET 4-DIGIT PIN", loginPin, cy - 20, true, true);
    }

    sf::Text h("ENTER: Continue", assets.getFont("main"), 14);
    UI::centerText(h, sf::Vector2f(cx, cy + 80));
    h.setFillColor(UI::MutedText);
    window.draw(h);
}

void Renderer::drawAvatarSelection() {
    float cx = window.getSize().x / 2.0f, cy = window.getSize().y / 2.0f;
    drawGlassPanel(sf::Vector2f(500, 350), sf::Vector2f(cx - 250, cy - 150));

    sf::Text h("SELECT YOUR AVATAR", assets.getFont("main"), 24);
    UI::centerText(h, sf::Vector2f(cx, cy - 110));
    h.setFillColor(UI::CyberTeal);
    window.draw(h);

    // Show a grid of avatars (5 colors x 5 shapes = 25 avatars)
    int count = 0;
    for (int c = 0; c < 5; ++c) {
        for (int s = 0; s < 5; ++s) {
            float x = cx - 180 + s * 90;
            float y = cy - 50 + c * 50;
            
            bool selected = (count == avatarIndex);
            Piece p((T_Color)c, (T_Shape)s, nullptr, nullptr, nullptr, nullptr, nullptr);
            drawFancyPiece(p, x, y, selected ? 1.2f : 0.8f, selected);
            
            if (selected) {
                sf::RectangleShape highlight(sf::Vector2f(60, 45));
                highlight.setOrigin(30, 22.5);
                highlight.setPosition(x, y);
                highlight.setFillColor(sf::Color::Transparent);
                highlight.setOutlineColor(sf::Color::Yellow);
                highlight.setOutlineThickness(2);
                window.draw(highlight);
            }
            count++;
        }
    }

    sf::Text help("USE ARROWS TO SELECT | ENTER TO FINALIZE", assets.getFont("main"), 14);
    UI::centerText(help, sf::Vector2f(cx, cy + 170));
    help.setFillColor(UI::MutedText);
    window.draw(help);
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

void Renderer::drawMainMenu() {
    if (!assets.hasFont("main")) return;
    float cx = window.getSize().x / 2.0f;
    
    sf::Text title("TETRISTE", assets.getFont("main"), 90);
    title.setFillColor(UI::CyberTeal);
    UI::centerText(title, sf::Vector2f(cx, 100));
    window.draw(title);

    std::vector<std::string> opts = {
        "ESTABLISH LINK (PLAY)", 
        "PROTOCOL ARCHIVE (ACHIEVEMENTS)",
        "SYSTEM CONFIG (SETTINGS)",
        "PROTOCOLS (RULES)", 
        "TERMINATE (EXIT)"
    };
    for (int i = 0; i < 5; ++i) {
        sf::Text t(opts[i], assets.getFont("main"), 24);
        UI::centerText(t, sf::Vector2f(cx, 260 + i * 60));
        if (i == menuSelection) {
            float pulse = 1.0f + 0.05f * std::sin(totalTime * 5.0f);
            t.setFillColor(sf::Color::Yellow);
            t.setScale(pulse, pulse);
        } else t.setFillColor(sf::Color::White);
        window.draw(t);
    }
    if (showAbout) drawAboutPage();
}

void Renderer::drawGameOver(int s) {
    float cx = window.getSize().x/2.0f;
    sf::RectangleShape bg(sf::Vector2f(window.getSize().x, window.getSize().y));
    bg.setFillColor(sf::Color(50, 0, 0, 200));
    window.draw(bg);

    sf::Text t("CONNECTION LOST", assets.getFont("main"), 60);
    t.setFillColor(sf::Color::Red);
    UI::centerText(t, sf::Vector2f(cx, 150));
    window.draw(t);

    sf::Text sc("FINAL_SCORE: " + std::to_string(s), assets.getFont("main"), 30);
    UI::centerText(sc, sf::Vector2f(cx, 250));
    window.draw(sc);

    sf::Text h("PRESS SPACE TO RESET", assets.getFont("main"), 20);
    UI::centerText(h, sf::Vector2f(cx, 450));
    window.draw(h);
}

void Renderer::drawLeaderboard(float x, float y) {
    drawCard("ELITE OPERATORS (TOP 3)", sf::Vector2f(x, y), sf::Vector2f(210, 115));
    auto top = userManager.getTopRecords(3);
    for (size_t i = 0; i < top.size(); ++i) {
        // Draw Avatar piece
        std::string avatar = top[i].avatarId;
        // Parse avatarId "color_shape"
        sf::Sprite avatarSprite;
        if (assets.hasTexture(avatar)) {
            avatarSprite.setTexture(assets.getTexture(avatar));
            avatarSprite.setScale(0.5f, 0.5f);
            avatarSprite.setPosition(x + 10, y + 30 + i * 24);
            window.draw(avatarSprite);
        }

        sf::Text r(top[i].pseudo, assets.getFont("main"), 14);
        r.setPosition(x + 35, y + 30 + i * 24);
        if (i == 0) r.setFillColor(sf::Color::Yellow); // Gold for #1
        window.draw(r);
        sf::Text s(std::to_string(top[i].record), assets.getFont("main"), 14);
        s.setPosition(x + 150, y + 30 + i * 24);
        s.setFillColor(UI::MutedText);
        window.draw(s);
    }
}

void Renderer::drawAboutPage() {
    sf::RectangleShape ov(sf::Vector2f(window.getSize().x, window.getSize().y));
    ov.setFillColor(sf::Color(5, 10, 20, 245));
    window.draw(ov);

    float cx = window.getSize().x / 2.0f;
    sf::Text t("SYSTEM PROTOCOLS", assets.getFont("main"), 40);
    t.setFillColor(UI::CyberTeal);
    UI::centerText(t, sf::Vector2f(cx, 80));
    window.draw(t);

    std::string txt = "1. ALIGN 3+ MATCHING NODES (COLOR/SHAPE) TO PURGE\n"
                      "2. INSERTION: [J] LEFT | [K] RIGHT\n"
                      "3. SHIFTING: [C] COLOR | [S] SHAPE (USES NEXT NODE)\n"
                      "4. VOLUME: [+] INCREASE | [-] DECREASE\n\n"
                      "TRIPLE-LINKED LOGIC IS REQUIRED FOR SUPREMACY.";
    sf::Text r(txt, assets.getFont("main"), 18);
    UI::centerText(r, sf::Vector2f(cx, 250));
    window.draw(r);

    sf::Text cl("PRESS 'A' TO ACKNOWLEDGE", assets.getFont("main"), 20);
    UI::centerText(cl, sf::Vector2f(cx, 500));
    cl.setFillColor(UI::MutedText);
    window.draw(cl);
}

void Renderer::drawAchievementsPage() {
    sf::RectangleShape bg(sf::Vector2f(window.getSize().x, window.getSize().y));
    bg.setFillColor(UI::DeepCharcoal);
    window.draw(bg);

    float cx = window.getSize().x / 2.0f;
    sf::Text t("ACHIEVEMENTS & PROTOCOLS", assets.getFont("main"), 40);
    t.setFillColor(UI::MintGreen);
    UI::centerText(t, sf::Vector2f(cx, 60));
    window.draw(t);

    UserProfile user = userManager.getCurrentUserProfile();
    std::vector<Achievement> allAchs;
    allAchs.push_back({"first_match", "PURGE INITIATE", "Clear your first sequence of nodes.", false});
    allAchs.push_back({"vet_purger", "VETERAN PURGER", "Purge a total of 100 nodes.", false});
    allAchs.push_back({"elite_op", "ELITE OPERATIVE", "Complete a match on ELITE difficulty.", false});

    float startX = 100, startY = 140;
    float cardW = 600, cardH = 80;

    for (size_t i = 0; i < allAchs.size(); ++i) {
        bool unlocked = std::find(user.achievements.begin(), user.achievements.end(), allAchs[i].id) != user.achievements.end();
        
        sf::RectangleShape card = UI::createPanel(sf::Vector2f(cardW, cardH), sf::Vector2f(startX, startY + i * 100));
        card.setFillColor(sf::Color(37, 37, 41));
        if (unlocked) card.setOutlineColor(UI::MintGreen);
        else card.setOutlineColor(sf::Color(69, 69, 69));
        window.draw(card);

        sf::Text title(allAchs[i].name, assets.getFont("main"), 20);
        title.setPosition(startX + 20, startY + i * 100 + 15);
        title.setFillColor(unlocked ? UI::PureWhite : UI::MutedText);
        window.draw(title);

        sf::Text desc(allAchs[i].description, assets.getFont("main"), 14);
        desc.setPosition(startX + 20, startY + i * 100 + 45);
        desc.setFillColor(UI::MutedText);
        window.draw(desc);

        if (unlocked) {
            sf::Text status("UNLOCKED", assets.getFont("main"), 14);
            status.setPosition(startX + cardW - 100, startY + i * 100 + 30);
            status.setFillColor(UI::MintGreen);
            window.draw(status);
        }
    }

    sf::Text esc("PRESS ESC TO RETURN", assets.getFont("main"), 18);
    UI::centerText(esc, sf::Vector2f(cx, 550));
    esc.setFillColor(UI::MutedText);
    window.draw(esc);
}

void Renderer::drawSettingsPage() {
    sf::RectangleShape bg(sf::Vector2f(window.getSize().x, window.getSize().y));
    bg.setFillColor(sf::Color::Black);
    window.draw(bg);

    float cx = window.getSize().x / 2.0f;
    sf::Text t("SYSTEM CONFIGURATION", assets.getFont("main"), 40);
    t.setFillColor(UI::MatrixGreen);
    UI::centerText(t, sf::Vector2f(cx, 80));
    window.draw(t);

    float vol = assets.getVolume();
    int blocks = (int)(vol / 10.0f);
    std::string volStr = "VOLUME: [";
    for(int i=0; i<10; ++i) volStr += (i < blocks) ? "||" : "..";
    volStr += "] " + std::to_string((int)vol) + "%";

    sf::Text vText(volStr, assets.getFont("main"), 24);
    vText.setFillColor(UI::MatrixGreen);
    UI::centerText(vText, sf::Vector2f(cx, 250));
    window.draw(vText);

    sf::Text hints("[+] INCREASE | [-] DECREASE\n\nPRESS ESC TO SAVE AND EXIT", assets.getFont("main"), 18);
    hints.setFillColor(UI::MatrixGreen);
    UI::centerText(hints, sf::Vector2f(cx, 400));
    window.draw(hints);

    // ASCII Border
    sf::RectangleShape border(sf::Vector2f(600, 400));
    border.setPosition(cx - 300, 100);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(2);
    border.setOutlineColor(UI::MatrixGreen);
    window.draw(border);
}

void Renderer::addPopup(std::string c, sf::Vector2f p, sf::Color clr) {
    if (!assets.hasFont("main")) return;
    FloatingText pop;
    pop.text.setFont(assets.getFont("main"));
    pop.text.setString(c);
    pop.text.setCharacterSize(28);
    pop.text.setFillColor(clr);
    UI::centerText(pop.text, p);
    pop.lifetime = pop.maxLifetime = 1.2f;
    pop.velocity = sf::Vector2f(0, -60);
    popups.push_back(pop);
}

void Renderer::drawPiece(Piece &p, float x, float y, float s) { drawFancyPiece(p, x, y, s, false); }

std::string Renderer::getTextureName(T_Color c, T_Shape s) {
    std::string cs, ss;
    switch(c) { case T_Color::BLUE: cs = "blue"; break; case T_Color::YELLOW: cs = "yellow"; break; case T_Color::RED: cs = "red"; break; case T_Color::GREEN: cs = "green"; break; case T_Color::WHITE: cs = "white"; break; }
    switch(s) { case T_Shape::SQUARE: ss = "square"; break; case T_Shape::DIAMOND: ss = "diamond"; break; case T_Shape::CIRCLE: ss = "circle"; break; case T_Shape::TRIANGLE: ss = "triangle"; break; case T_Shape::STAR: ss = "star"; break; }
    return cs + "_" + ss;
}

void Renderer::drawDifficultySelection() {
    float cx = window.getSize().x / 2.0f, cy = window.getSize().y / 2.0f;
    drawGlassPanel(sf::Vector2f(500, 400), sf::Vector2f(cx - 250, cy - 200));

    sf::Text h("SELECT DIFFICULTY", assets.getFont("main"), 40);
    h.setFillColor(UI::CyberTeal);
    UI::centerText(h, sf::Vector2f(cx, cy - 150));
    window.draw(h);

    std::vector<std::string> diffs = {"RECRUIT", "VETERAN", "ELITE"};
    std::vector<std::string> descs = {
        "Board capacity: 100% | Slow gravity",
        "Board capacity: 80% | Normal gravity",
        "Board capacity: 60% | Fast gravity"
    };

    for (int i = 0; i < 3; ++i) {
        bool selected = (int)difficulty == i;
        sf::Text t(diffs[i], assets.getFont("main"), 30);
        UI::centerText(t, sf::Vector2f(cx, cy - 50 + i * 80));
        
        if (selected) {
            t.setFillColor(sf::Color::Yellow);
            t.setScale(1.1f, 1.1f);
            sf::Text d(descs[i], assets.getFont("main"), 14);
            UI::centerText(d, sf::Vector2f(cx, cy - 20 + i * 80));
            d.setFillColor(UI::MutedText);
            window.draw(d);
        } else {
            t.setFillColor(sf::Color::White);
        }
        window.draw(t);
    }

    sf::Text help("UP/DOWN: Select | ENTER: Deploy", assets.getFont("main"), 14);
    UI::centerText(help, sf::Vector2f(cx, cy + 170));
    help.setFillColor(UI::MutedText);
    window.draw(help);
}

void Renderer::drawStats(float x, float y) {
    drawCard("LIFETIME STATS", sf::Vector2f(x, y), sf::Vector2f(210, 80));
    UserProfile p = userManager.getCurrentUserProfile();
    
    sf::Text m("MATCHES: " + std::to_string(p.matchesPlayed), assets.getFont("main"), 14);
    m.setPosition(x + 10, y + 30);
    window.draw(m);

    sf::Text n("PURGED: " + std::to_string(p.nodesPurged), assets.getFont("main"), 14);
    n.setPosition(x + 10, y + 54);
    window.draw(n);
}
