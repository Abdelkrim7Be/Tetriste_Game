#include "Renderer.h"
#include <vector>
#include <cmath>

Renderer::Renderer(sf::RenderWindow &win, AssetManager &asmgr) 
    : window(win), assets(asmgr) {}

void Renderer::render(Game &game, Piece *nextPiece) {
    window.clear(sf::Color(20, 20, 25)); // Slightly darker, more blue-ish background
    
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
        float startX = 80.0f; // Shifted right by 30 to account for center origin (50 + 30)
        float startY = 325.0f; // Shifted down by 25 to account for center origin (300 + 25)
        int count = 0;
        
        // Draw a horizontal line or "tube" for the pieces
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
    
    // Draw UI in Sidebar
    if (assets.hasFont("main")) {
        // Score section
        sf::Text scoreLabel("SCORE", assets.getFont("main"), 18);
        scoreLabel.setPosition(620.0f, 30.0f);
        scoreLabel.setFillColor(sf::Color(150, 150, 150));
        window.draw(scoreLabel);

        sf::Text scoreText(std::to_string(game.score), assets.getFont("main"), 36);
        scoreText.setPosition(620.0f, 55.0f);
        window.draw(scoreText);

        // Next Piece section
        sf::Text nextLabel("NEXT PIECE", assets.getFont("main"), 18);
        nextLabel.setPosition(620.0f, 150.0f);
        nextLabel.setFillColor(sf::Color(150, 150, 150));
        window.draw(nextLabel);

        if (nextPiece != nullptr) {
            // Pulse animation for the next piece
            float time = clock.getElapsedTime().asSeconds();
            float scale = 1.0f + 0.1f * std::sin(time * 4.0f); // Pulsate between 0.9 and 1.1
            drawPiece(*nextPiece, 675.0f, 200.0f, scale);
        }

        // Controls hint
        sf::Text hint("J: Left\nK: Right\nC: Color\nS: Shape", assets.getFont("main"), 16);
        hint.setPosition(620.0f, 450.0f);
        hint.setFillColor(sf::Color(100, 100, 100));
        window.draw(hint);
    }
    
    window.display();
}

void Renderer::drawPiece(Piece &piece, float x, float y, float scale) {
    std::string texName = getTextureName(piece.color, piece.shape);
    sf::Sprite sprite(assets.getTexture(texName));
    
    // Set origin to center for scaling
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
