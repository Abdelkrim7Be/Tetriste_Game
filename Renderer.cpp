#include "Renderer.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

Renderer::Renderer(sf::RenderWindow &win, AssetManager &asmgr, UserManager &umgr) 
    : window(win), assets(asmgr), userManager(umgr) {}

void Renderer::render(Game &game, Piece *nextPiece, GameState state) {
    float deltaTime = clock.restart().asSeconds();
    window.clear(sf::Color(20, 20, 25)); 
    
    if (state == GameState::MENU) {
        drawMainMenu();
    } else if (state == GameState::GAME_OVER) {
        drawGameOver(game.score);
    } else {
        sf::Vector2u winSize = window.getSize();
        float viewWidth = static_cast<float>(winSize.x);
        float viewHeight = static_cast<float>(winSize.y);
        float sidebarWidth = 200.0f;
        float sidebarX = viewWidth - sidebarWidth;

        // Draw Dynamic Background Grid
        sf::RectangleShape gridLine(sf::Vector2f(viewWidth, 1.0f));
        gridLine.setFillColor(sf::Color(40, 40, 50));
        for (float y = 0; y < viewHeight; y += 50.0f) {
            gridLine.setPosition(0, y);
            window.draw(gridLine);
        }
        gridLine.setSize(sf::Vector2f(1.0f, viewHeight));
        for (float x = 0; x < viewWidth; x += 50.0f) {
            gridLine.setPosition(x, 0);
            window.draw(gridLine);
        }

        // Draw Sidebar Background (Anchored Right)
        sf::RectangleShape sidebar(sf::Vector2f(sidebarWidth, viewHeight));
        sidebar.setPosition(sidebarX, 0.0f);
        sidebar.setFillColor(sf::Color(35, 35, 45, 220));
        window.draw(sidebar);

        // Draw board pieces with wrapping logic
        if (game.head != nullptr) {
            Piece *current = game.head;
            float startX = 60.0f; 
            float startY = 60.0f;
            float currentX = startX;
            float currentY = startY;
            float spacingX = 60.0f;
            float spacingY = 80.0f;
            int count = 0;
            
            // Draw board rows background
            float boardWidth = sidebarX - startX;
            int rowsNeeded = (game.piecesCount * spacingX / boardWidth) + 1;
            for (int i = 0; i < rowsNeeded; ++i) {
                sf::RectangleShape rowBg(sf::Vector2f(boardWidth + 20.0f, 60.0f));
                rowBg.setPosition(startX - 30.0f, startY + (i * spacingY) - 30.0f);
                rowBg.setFillColor(sf::Color(50, 50, 60, 100));
                rowBg.setOutlineThickness(1);
                rowBg.setOutlineColor(sf::Color(100, 100, 120, 150));
                window.draw(rowBg);
            }

            do {
                drawPiece(*current, currentX, currentY);
                current = current->nextPiece;
                count++;
                
                currentX += spacingX;
                if (currentX > sidebarX - spacingX) {
                    currentX = startX;
                    currentY += spacingY;
                }
            } while (current != game.head && count < game.piecesCount);
        }
        
        // Update and Draw Popups
        for (auto it = popups.begin(); it != popups.end(); ) {
            it->lifetime -= deltaTime;
            if (it->lifetime <= 0) {
                it = popups.erase(it);
            } else {
                it->text.move(it->velocity * deltaTime);
                // Fade out
                sf::Color c = it->text.getFillColor();
                c.a = static_cast<sf::Uint8>(255 * (it->lifetime / it->maxLifetime));
                it->text.setFillColor(c);
                window.draw(it->text);
                ++it;
            }
        }

        // Draw UI in Sidebar (Anchored Right)
        if (assets.hasFont("main")) {
            float uiX = sidebarX + 20.0f;
            
            sf::Text scoreLabel("SCORE", assets.getFont("main"), 18);
            scoreLabel.setPosition(uiX, 10.0f);
            scoreLabel.setFillColor(sf::Color(150, 150, 150));
            window.draw(scoreLabel);

            sf::Text scoreText(std::to_string(game.score), assets.getFont("main"), 32);
            scoreText.setPosition(uiX, 25.0f);
            window.draw(scoreText);

            // Display Current User and Record
            sf::Text userLabel(userManager.getCurrentUserPseudo(), assets.getFont("main"), 18);
            userLabel.setPosition(uiX, 65.0f);
            userLabel.setFillColor(sf::Color(100, 200, 255));
            window.draw(userLabel);

            sf::Text recordText("REC: " + std::to_string(userManager.getCurrentUserRecord()), assets.getFont("main"), 14);
            recordText.setPosition(uiX, 90.0f);
            recordText.setFillColor(sf::Color(200, 200, 100));
            window.draw(recordText);

            // Dynamic Capacity Display
            float startX = 60.0f;
            float startY = 60.0f;
            float spacingX = 60.0f;
            float spacingY = 80.0f;
            float boardWidth = sidebarX - startX;
            int pPerRow = static_cast<int>(boardWidth / spacingX);
            if (pPerRow < 1) pPerRow = 1;
            int mRows = static_cast<int>((viewHeight - startY - 25.0f) / spacingY) + 1;
            if (mRows < 1) mRows = 0;
            int capacity = pPerRow * mRows;

            sf::Text capLabel("CAPACITY", assets.getFont("main"), 18);
            capLabel.setPosition(uiX, 120.0f);
            capLabel.setFillColor(sf::Color(150, 150, 150));
            window.draw(capLabel);

            sf::Text capText(std::to_string(game.piecesCount) + " / " + std::to_string(capacity), assets.getFont("main"), 24);
            capText.setPosition(uiX, 140.0f);
            capText.setFillColor(game.piecesCount >= capacity ? sf::Color::Red : sf::Color::White);
            window.draw(capText);

            sf::Text nextLabel("NEXT PIECE", assets.getFont("main"), 18);
            nextLabel.setPosition(uiX, 180.0f);
            nextLabel.setFillColor(sf::Color(150, 150, 150));
            window.draw(nextLabel);

            if (nextPiece != nullptr) {
                static float totalTime = 0;
                totalTime += deltaTime;
                float scale = 1.0f + 0.1f * std::sin(totalTime * 4.0f);
                drawPiece(*nextPiece, sidebarX + 100.0f, 240.0f, scale);
            }

            drawLeaderboard(uiX, 320.0f);

            sf::Text hint("J: Left  K: Right\nC: Color S: Shape\nA: About", assets.getFont("main"), 16);
            hint.setPosition(uiX, viewHeight - 100.0f);
            hint.setFillColor(sf::Color(100, 100, 100));
            window.draw(hint);
        }

        if (state == GameState::PAUSED) {
            sf::RectangleShape overlay(sf::Vector2f(viewWidth, viewHeight));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);

            if (assets.hasFont("main")) {
                sf::Text pauseText("PAUSED", assets.getFont("main"), 60);
                pauseText.setOrigin(pauseText.getLocalBounds().width / 2.0f, pauseText.getLocalBounds().height / 2.0f);
                pauseText.setPosition(viewWidth / 2.0f, viewHeight / 2.0f);
                window.draw(pauseText);

                sf::Text hint("Press ESC to Resume", assets.getFont("main"), 20);
                hint.setOrigin(hint.getLocalBounds().width / 2.0f, 0);
                hint.setPosition(viewWidth / 2.0f, viewHeight / 2.0f + 50.0f);
                window.draw(hint);
            }
        }
        
        if (showAbout) {
            drawAboutPage();
        }
    }

    window.display();
}

void Renderer::drawMainMenu() {
    if (!assets.hasFont("main")) return;

    float centerX = window.getSize().x / 2.0f;
    float startY = 150.0f;

    sf::Text title("TETRISTE", assets.getFont("main"), 80);
    title.setOrigin(title.getLocalBounds().width / 2.0f, 0);
    title.setPosition(centerX, 50.0f);
    title.setFillColor(sf::Color(0, 255, 150));
    window.draw(title);

    std::vector<std::string> options = {"PLAY", "RULES", "EXIT"};
    for (size_t i = 0; i < options.size(); ++i) {
        sf::Text opt(options[i], assets.getFont("main"), 40);
        opt.setOrigin(opt.getLocalBounds().width / 2.0f, 0);
        opt.setPosition(centerX, startY + 100.0f + i * 70.0f);
        
        if (static_cast<int>(i) == menuSelection) {
            opt.setFillColor(sf::Color::Yellow);
            opt.setScale(1.1f, 1.1f);
        } else {
            opt.setFillColor(sf::Color::White);
        }
        window.draw(opt);
    }

    sf::Text user("Logged in as: " + userManager.getCurrentUserPseudo(), assets.getFont("main"), 18);
    user.setOrigin(user.getLocalBounds().width / 2.0f, 0);
    user.setPosition(centerX, window.getSize().y - 50.0f);
    user.setFillColor(sf::Color(150, 150, 150));
    window.draw(user);

    if (showAbout) {
        drawAboutPage();
    }
}

void Renderer::drawGameOver(int finalScore) {
    if (!assets.hasFont("main")) return;

    float centerX = window.getSize().x / 2.0f;

    sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(sf::Color(100, 0, 0, 150));
    window.draw(overlay);

    sf::Text title("GAME OVER", assets.getFont("main"), 72);
    title.setOrigin(title.getLocalBounds().width / 2.0f, 0);
    title.setPosition(centerX, 100.0f);
    title.setFillColor(sf::Color::White);
    window.draw(title);

    sf::Text score("Final Score: " + std::to_string(finalScore), assets.getFont("main"), 36);
    score.setOrigin(score.getLocalBounds().width / 2.0f, 0);
    score.setPosition(centerX, 250.0f);
    window.draw(score);

    sf::Text record("Personal Best: " + std::to_string(userManager.getCurrentUserRecord()), assets.getFont("main"), 24);
    record.setOrigin(record.getLocalBounds().width / 2.0f, 0);
    record.setPosition(centerX, 300.0f);
    record.setFillColor(sf::Color(255, 215, 0));
    window.draw(record);

    sf::Text hint("Press SPACE to return to Menu", assets.getFont("main"), 20);
    hint.setOrigin(hint.getLocalBounds().width / 2.0f, 0);
    hint.setPosition(centerX, 450.0f);
    window.draw(hint);
}

void Renderer::drawLeaderboard(float x, float y) {
    if (!assets.hasFont("main")) return;

    sf::Text title("TOP 5 RANKING", assets.getFont("main"), 18);
    title.setPosition(x, y);
    title.setFillColor(sf::Color(255, 215, 0)); // Gold
    window.draw(title);

    auto top5 = userManager.getTop5();
    for (size_t i = 0; i < top5.size(); ++i) {
        sf::Text rank(std::to_string(i+1) + ". " + top5[i].pseudo, assets.getFont("main"), 14);
        rank.setPosition(x, y + 30.0f + i * 20.0f);
        window.draw(rank);

        sf::Text score(std::to_string(top5[i].record), assets.getFont("main"), 14);
        score.setPosition(x + 130.0f, y + 30.0f + i * 20.0f);
        score.setFillColor(sf::Color(150, 150, 150));
        window.draw(score);
    }
}

void Renderer::drawAboutPage() {
    sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 230));
    window.draw(overlay);

    if (assets.hasFont("main")) {
        float centerX = window.getSize().x / 2.0f;
        
        sf::Text title("TETRISTE: MASTER ALGORITHMICS", assets.getFont("main"), 36);
        title.setOrigin(title.getLocalBounds().width / 2.0f, 0);
        title.setPosition(centerX, 50.0f);
        title.setFillColor(sf::Color(0, 255, 150));
        window.draw(title);

        sf::Text rules(
            "PURPOSE:\n"
            "Master complex data structures & cyclic shifts in C++.\n\n"
            "RULES:\n"
            "1. Insert pieces Left (J) or Right (K).\n"
            "2. Match 3+ pieces of same Color or Shape to score.\n"
            "3. Shifting (C/S) uses the attribute of your NEXT piece\n"
            "   to rotate all pieces sharing that attribute.\n"
            "4. Mastery of the triple-linked list is key to victory.", 
            assets.getFont("main"), 20
        );
        rules.setOrigin(rules.getLocalBounds().width / 2.0f, 0);
        rules.setPosition(centerX, 150.0f);
        window.draw(rules);

        sf::Text close("Press 'A' to return to game", assets.getFont("main"), 18);
        close.setOrigin(close.getLocalBounds().width / 2.0f, 0);
        close.setPosition(centerX, window.getSize().y - 100.0f);
        close.setFillColor(sf::Color(100, 100, 100));
        window.draw(close);
    }
}

void Renderer::addPopup(std::string content, sf::Vector2f position, sf::Color color) {
    if (!assets.hasFont("main")) return;
    
    FloatingText popup;
    popup.text.setFont(assets.getFont("main"));
    popup.text.setString(content);
    popup.text.setCharacterSize(24);
    popup.text.setFillColor(color);
    // Center text
    sf::FloatRect bounds = popup.text.getLocalBounds();
    popup.text.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    popup.text.setPosition(position);
    
    popup.lifetime = 1.5f;
    popup.maxLifetime = 1.5f;
    popup.velocity = sf::Vector2f(0.0f, -50.0f); // Move upwards
    
    popups.push_back(popup);
}

void Renderer::drawPiece(Piece &piece, float x, float y, float scale) {
    std::string texName = getTextureName(piece.color, piece.shape);
    sf::Sprite sprite(assets.getTexture(texName));
    
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
    
    sprite.setPosition(x, y);
    sprite.setScale(scale, scale);
    window.draw(sprite);
}

std::string Renderer::getTextureName(T_Color color, T_Shape shape) {
    std::string c, s;
    switch(color) {
        case T_Color::BLUE: c = "blue"; break;
        case T_Color::YELLOW: c = "yellow"; break;
        case T_Color::RED: c = "red"; break;
        case T_Color::GREEN: c = "green"; break;
        case T_Color::WHITE: c = "white"; break;
    }
    switch(shape) {
        case T_Shape::SQUARE: s = "square"; break;
        case T_Shape::DIAMOND: s = "diamond"; break;
        case T_Shape::CIRCLE: s = "circle"; break;
        case T_Shape::TRIANGLE: s = "triangle"; break;
        case T_Shape::STAR: s = "star"; break;
    }
    return c + "_" + s;
}
