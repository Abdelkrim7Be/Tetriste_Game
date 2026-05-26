#ifndef RENDERER_H
#define RENDERER_H

#include <SFML/Graphics.hpp>
#include "gameDeclaration.h"
#include "AssetManager.h"

class Renderer {
public:
    Renderer(sf::RenderWindow &window, AssetManager &assets);
    
    void render(Game &game, Piece *nextPiece);

private:
    sf::RenderWindow &window;
    AssetManager &assets;
    
    void drawPiece(Piece &piece, float x, float y, float scale = 1.0f);
    std::string getTextureName(T_Color color, T_Shape shape);
    
    sf::Clock clock;
};

#endif
