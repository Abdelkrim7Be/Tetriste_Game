#include <SFML/Graphics.hpp>
#include <iostream>
#include <ctime>
#include <vector>
#include "gameDeclaration.h"
#include "generalHeader.h"
#include "AssetManager.h"
#include "Renderer.h"

void generatePlaceholderAssets(AssetManager &assets) {
    T_Color colors[] = {T_Color::BLUE, T_Color::YELLOW, T_Color::RED, T_Color::GREEN, T_Color::WHITE};
    T_Shape shapes[] = {T_Shape::SQUARE, T_Shape::DIAMOND, T_Shape::CIRCLE, T_Shape::TRIANGLE, T_Shape::STAR};
    
    const int size = 50;
    const int padding = 5;

    for (auto c : colors) {
        for (auto s : shapes) {
            sf::RenderTexture rt;
            rt.create(size, size);
            rt.clear(sf::Color::Transparent);
            
            sf::Color color;
            switch(c) {
                case T_Color::BLUE: color = sf::Color(52, 152, 219); break;
                case T_Color::YELLOW: color = sf::Color(241, 196, 15); break;
                case T_Color::RED: color = sf::Color(231, 76, 60); break;
                case T_Color::GREEN: color = sf::Color(46, 204, 113); break;
                case T_Color::WHITE: color = sf::Color(236, 240, 241); break;
            }
            
            sf::Shape* shapeObj = nullptr;
            
            if (s == T_Shape::SQUARE) {
                sf::RectangleShape* rect = new sf::RectangleShape(sf::Vector2f(size - 2*padding, size - 2*padding));
                shapeObj = rect;
            } else if (s == T_Shape::CIRCLE) {
                sf::CircleShape* circle = new sf::CircleShape((size - 2*padding) / 2.0f);
                shapeObj = circle;
            } else if (s == T_Shape::TRIANGLE) {
                sf::CircleShape* tri = new sf::CircleShape((size - 2*padding) / 2.0f, 3);
                shapeObj = tri;
            } else if (s == T_Shape::DIAMOND) {
                sf::CircleShape* diamond = new sf::CircleShape((size - 2*padding) / 2.0f, 4);
                diamond->setRotation(45);
                // Adjust position due to rotation
                diamond->setOrigin((size - 2*padding) / 2.0f, (size - 2*padding) / 2.0f);
                diamond->setPosition(size/2.0f, size/2.0f);
                shapeObj = diamond;
            } else if (s == T_Shape::STAR) {
                // Approximate star with 5-sided circle for now, or use a vertex array
                sf::CircleShape* star = new sf::CircleShape((size - 2*padding) / 2.0f, 5);
                shapeObj = star;
            }

            if (shapeObj) {
                shapeObj->setFillColor(color);
                shapeObj->setOutlineThickness(2);
                shapeObj->setOutlineColor(sf::Color(255, 255, 255, 150));
                if (s != T_Shape::DIAMOND) {
                    shapeObj->setPosition(padding, padding);
                }
                rt.draw(*shapeObj);
                delete shapeObj;
            }
            
            rt.display();
            
            std::string name;
            switch(c) {
                case T_Color::BLUE: name = "blue"; break;
                case T_Color::YELLOW: name = "yellow"; break;
                case T_Color::RED: name = "red"; break;
                case T_Color::GREEN: name = "green"; break;
                case T_Color::WHITE: name = "white"; break;
            }
            std::string sname;
            switch(s) {
                case T_Shape::SQUARE: sname = "square"; break;
                case T_Shape::DIAMOND: sname = "diamond"; break;
                case T_Shape::CIRCLE: sname = "circle"; break;
                case T_Shape::TRIANGLE: sname = "triangle"; break;
                case T_Shape::STAR: sname = "star"; break;
            }
            
            sf::Image img = rt.getTexture().copyToImage();
            std::string filename = "assets/" + name + "_" + sname + ".png";
            img.saveToFile(filename);
            assets.loadTexture(name + "_" + sname, filename);
        }
    }
}

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    sf::RenderWindow window(sf::VideoMode(800, 600), "Tetriste Graphical");
    window.setFramerateLimit(60);

    AssetManager assets;
    
    // Ensure assets directory exists
    #ifdef _WIN32
        system("if not exist assets mkdir assets");
    #else
        system("mkdir -p assets");
    #endif

    generatePlaceholderAssets(assets);
    
    // Font discovery
    std::vector<std::string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "assets/font.ttf" // Local fallback
    };

    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (assets.loadFont("main", path)) {
            fontLoaded = true;
            break;
        }
    }

    if (!fontLoaded) {
        std::cerr << "Warning: Could not load any system fonts. UI text may not display." << std::endl;
    }

    Renderer renderer(window, assets);

    int randomColorIndex = rand() % 5;
    int randomShapeIndex = rand() % 5;
    
    // In this implementation, initializeGame creates a new Game object on the heap
    Game *currentGame = Game(0,0).initializeGame(randomColorIndex, randomShapeIndex);

    for (int i = 0; i < 5; ++i) {
        Piece *newPiece = currentGame->drawPiece(rand() % 5, rand() % 5);
        currentGame->insertPieceInRight(currentGame, newPiece);
    }

    Piece *nextPiece = currentGame->drawPiece(rand() % 5, rand() % 5);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            
            if (event.type == sf::Event::KeyPressed) {
                bool actionTaken = false;
                if (event.key.code == sf::Keyboard::J) {
                    currentGame->insertPieceInLeft(currentGame, nextPiece);
                    actionTaken = true;
                } else if (event.key.code == sf::Keyboard::K) {
                    currentGame->insertPieceInRight(currentGame, nextPiece);
                    actionTaken = true;
                } else if (event.key.code == sf::Keyboard::C) {
                    // Simplification: rotate to next color for now or show a menu
                    currentGame->colorShifting(currentGame, T_Color::RED, 0); 
                } else if (event.key.code == sf::Keyboard::S) {
                    currentGame->shapeShifting(currentGame, T_Shape::SQUARE, 0);
                }
                
                if (actionTaken) {
                    int scoreChange = currentGame->updateGame(currentGame);
                    if (scoreChange > 0) {
                        renderer.addPopup("+" + std::to_string(scoreChange), sf::Vector2f(300.0f, 300.0f));
                    }
                    nextPiece = currentGame->drawPiece(rand() % 5, rand() % 5);
                    if (currentGame->piecesCount == 0) {
                        std::cout << "You Won!" << std::endl;
                        renderer.addPopup("YOU WON!", sf::Vector2f(300.0f, 300.0f), sf::Color::Green);
                    }
                }
            }
        }

        renderer.render(*currentGame, nextPiece);
    }

    delete currentGame;
    return 0;
}
