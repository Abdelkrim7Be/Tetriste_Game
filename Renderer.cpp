#include "Renderer.h"
#include <vector>
#include <cmath>
#include <algorithm>

Renderer::Renderer(sf::RenderWindow &win, AssetManager &asmgr) 
    : window(win), assets(asmgr) {}

void Renderer::render(Game &game, Piece *nextPiece) {
    float deltaTime = clock.restart().asSeconds();
    window.clear(sf::Color(20, 20, 25)); 
    
    // Draw Background Grid
    sf::RectangleShape gridLine(sf::Vector2f(800.0f, 1.0f));
    gridLine.setFillColor(sf::Color(40, 40, 50));
    for (int i = 0; i < 12; ++i) {
        gridLine.setPosition(0, i * 50.0f);
        window.draw(gridLine);
    }
    gridLine.setSize(sf::Vector2f(1.0f, 600.0f));
    for (int i = 0; i < 16; ++i) {
        gridLine.setPosition(i * 50.0f, 0);
        window.draw(gridLine);
    }

    // Draw Sidebar Background
    sf::RectangleShape sidebar(sf::Vector2f(200.0f, 600.0f));
    sidebar.setPosition(600.0f, 0.0f);
    sidebar.setFillColor(sf::Color(35, 35, 45, 200));
    window.draw(sidebar);

    // Draw board pieces
    if (game.head != nullptr) {
        Piece *current = game.head;
        float startX = 80.0f; 
        float startY = 325.0f;
        int count = 0;
        
        sf::RectangleShape tube(sf::Vector2f(550.0f, 60.0f));
        tube.setPosition(25.0f, 295.0f);
        tube.setFillColor(sf::Color(50, 50, 60, 100));
        tube.setOutlineThickness(2);
        tube.setOutlineColor(sf::Color(100, 100, 120));
        window.draw(tube);

        do {
            drawPiece(*current, startX + (count * 60.0f), startY);
            current = current->nextPiece;
            count++;
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

    // Draw UI in Sidebar
    if (assets.hasFont("main")) {
        sf::Text scoreLabel("SCORE", assets.getFont("main"), 18);
        scoreLabel.setPosition(620.0f, 30.0f);
        scoreLabel.setFillColor(sf::Color(150, 150, 150));
        window.draw(scoreLabel);

        sf::Text scoreText(std::to_string(game.score), assets.getFont("main"), 36);
        scoreText.setPosition(620.0f, 55.0f);
        window.draw(scoreText);

        sf::Text nextLabel("NEXT PIECE", assets.getFont("main"), 18);
        nextLabel.setPosition(620.0f, 150.0f);
        nextLabel.setFillColor(sf::Color(150, 150, 150));
        window.draw(nextLabel);

        if (nextPiece != nullptr) {
            // Pulse animation for the next piece using a total time accumulation
            static float totalTime = 0;
            totalTime += deltaTime;
            float scale = 1.0f + 0.1f * std::sin(totalTime * 4.0f);
            drawPiece(*nextPiece, 675.0f, 200.0f, scale);
        }

        sf::Text hint("J: Left\nK: Right\nC: Color\nS: Shape", assets.getFont("main"), 16);
        hint.setPosition(620.0f, 450.0f);
        hint.setFillColor(sf::Color(100, 100, 100));
        window.draw(hint);
    }
    
    window.display();
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
