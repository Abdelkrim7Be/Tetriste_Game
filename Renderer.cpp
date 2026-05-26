#include "Renderer.h"
#include <vector>

Renderer::Renderer(sf::RenderWindow &win, AssetManager &asmgr) 
    : window(win), assets(asmgr) {}

void Renderer::render(Game &game, Piece *nextPiece) {
    window.clear(sf::Color(30, 30, 30));
    
    // Draw board pieces
    if (game.head != nullptr) {
        Piece *current = game.head;
        float x = 50.0f;
        float y = 300.0f;
        int count = 0;
        
        do {
            drawPiece(*current, x, y);
            x += 60.0f;
            current = current->nextPiece;
            count++;
        } while (current != game.head && count < game.piecesCount);
    }
    
    // Draw next piece preview
    if (nextPiece != nullptr) {
        sf::Text nextLabel("Next:", assets.getFont("main"), 20);
        nextLabel.setPosition(600.0f, 50.0f);
        window.draw(nextLabel);
        drawPiece(*nextPiece, 600.0f, 80.0f);
    }
    
    // Draw score
    sf::Text scoreText("Score: " + std::to_string(game.score), assets.getFont("main"), 24);
    scoreText.setPosition(20.0f, 20.0f);
    window.draw(scoreText);
    
    window.display();
}

void Renderer::drawPiece(Piece &piece, float x, float y) {
    std::string texName = getTextureName(piece.color, piece.shape);
    sf::Sprite sprite(assets.getTexture(texName));
    sprite.setPosition(x, y);
    // Assuming sprites are roughly 50x50
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
