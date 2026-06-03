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
    } else if (state == GameState::GAME_OVER) {
        drawGameOver(game.score);
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
        border.setOutlineColor(sf::Color(UI::NeonGreen.r, UI::NeonGreen.g, UI::NeonGreen.b, (sf::Uint8)alpha));
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
            scoreText.setFillColor(UI::NeonGreen);
            scoreText.setPosition(uiX + 10, 45);
            window.draw(scoreText);

            // Player Card
            drawCard("OPERATOR: " + userManager.getCurrentUserPseudo(), sf::Vector2f(uiX, 110), sf::Vector2f(210, 60));
            sf::Text recText("BEST: " + std::to_string(userManager.getCurrentUserRecord()), assets.getFont("main"), 16);
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
            sf::Color barColor = UI::NeonGreen;
            if (game.piecesCount > capacity * 0.8f) barColor = sf::Color::Red;
            else if (game.piecesCount > capacity * 0.5f) barColor = sf::Color::Yellow;
            barFill.setFillColor(barColor);
            window.draw(barFill);

            // Next Piece
            drawCard("NEXT DATA", sf::Vector2f(uiX, 260), sf::Vector2f(210, 110));
            if (nextPiece != nullptr) {
                float pulse = 1.0f + 0.05f * std::sin(totalTime * 4.0f);
                drawFancyPiece(*nextPiece, uiX + 105, 330, pulse * 1.2f, false);
            }

            drawLeaderboard(uiX, 380.0f);

            sf::Text hints("J/K: Insert  C/S: Shift\nA: Info  ESC: Pause", assets.getFont("main"), 14);
            hints.setPosition(uiX + 10, viewHeight - 60.0f);
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
    card.setOutlineColor(sf::Color(UI::NeonGreen.r, UI::NeonGreen.g, UI::NeonGreen.b, 60));
    window.draw(card);

    if (assets.hasFont("main")) {
        sf::Text t(title, assets.getFont("main"), 12);
        t.setPosition(pos.x + 10, pos.y + 5);
        t.setFillColor(UI::NeonGreen);
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
    title.setFillColor(UI::NeonGreen);
    UI::centerText(title, sf::Vector2f(cx, 100));
    window.draw(title);

    drawGlassPanel(sf::Vector2f(420, 320), sf::Vector2f(cx - 210, cy - 140));
    
    auto drawIn = [&](std::string lbl, std::string val, float y, bool active) {
        sf::Text lText(lbl, assets.getFont("main"), 14);
        lText.setPosition(cx - 190, y - 30);
        lText.setFillColor(UI::MutedText);
        window.draw(lText);

        sf::RectangleShape box(sf::Vector2f(380, 45));
        box.setPosition(cx - 190, y - 5);
        box.setFillColor(sf::Color(10, 10, 20));
        box.setOutlineThickness(active ? 2.0f : 1.0f);
        box.setOutlineColor(active ? UI::NeonGreen : sf::Color(80, 80, 90));
        window.draw(box);

        if (active && (int)(cursorBlinkTimer * 2) % 2 == 0) val += "_";
        sf::Text vText(val, assets.getFont("main"), 22);
        vText.setPosition(cx - 180, y + 2);
        window.draw(vText);
    };

    drawIn("OPERATOR_ID (PSEUDO)", loginPseudo, cy - 20, true);

    sf::Text h("ENTER: Start Session", assets.getFont("main"), 14);
    UI::centerText(h, sf::Vector2f(cx, cy + 80));
    h.setFillColor(UI::MutedText);
    window.draw(h);
}

void Renderer::handleTextInput(uint32_t uni) {
    if (uni == '\t' || uni == '\r' || uni == '\n') return;
    if (uni == 8) { if (!loginPseudo.empty()) loginPseudo.pop_back(); }
    else if (uni < 128 && loginPseudo.length() < 16) loginPseudo += (char)uni;
}

void Renderer::drawMainMenu() {
    if (!assets.hasFont("main")) return;
    float cx = window.getSize().x / 2.0f;
    
    sf::Text title("TETRISTE", assets.getFont("main"), 90);
    title.setFillColor(UI::NeonGreen);
    UI::centerText(title, sf::Vector2f(cx, 100));
    window.draw(title);

    std::vector<std::string> opts = {"ESTABLISH LINK (PLAY)", "PROTOCOLS (RULES)", "TERMINATE (EXIT)"};
    for (int i = 0; i < 3; ++i) {
        sf::Text t(opts[i], assets.getFont("main"), 30);
        UI::centerText(t, sf::Vector2f(cx, 280 + i * 80));
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
        sf::Text r(std::to_string(i+1) + ". " + top[i].pseudo, assets.getFont("main"), 14);
        r.setPosition(x + 10, y + 30 + i * 24);
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
    t.setFillColor(UI::NeonGreen);
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
