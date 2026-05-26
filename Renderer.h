#ifndef RENDERER_H
#define RENDERER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "gameDeclaration.h"
#include "AssetManager.h"

struct FloatingText {
    sf::Text text;
    float lifetime; // in seconds
    float maxLifetime;
    sf::Vector2f velocity;
};

class Renderer {
public:
    Renderer(sf::RenderWindow &window, AssetManager &assets);
    
    void render(Game &game, Piece *nextPiece);
    void addPopup(std::string content, sf::Vector2f position, sf::Color color = sf::Color::Yellow);

private:
    sf::RenderWindow &window;
    AssetManager &assets;
    
    void drawPiece(Piece &piece, float x, float y, float scale = 1.0f);
    std::string getTextureName(T_Color color, T_Shape shape);
    
    sf::Clock clock;
    std::vector<FloatingText> popups;
};

#endif
