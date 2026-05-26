#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include "gameDeclaration.h"
#include "pieceDeclaration.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "UserManager.h"

void generatePlaceholderAssets(AssetManager& assets) {
    // Generate simple colored shapes if files are missing
    sf::RenderTexture rt;
    rt.create(40, 40);
    
    for (int c = 0; c < 5; ++c) {
        for (int s = 0; s < 5; ++s) {
            rt.clear(sf::Color::Transparent);
            T_Color color = static_cast<T_Color>(c);
            T_Shape shape = static_cast<T_Shape>(s);
            
            sf::Color sfColor;
            switch(color) {
                case T_Color::BLUE: sfColor = sf::Color::Blue; break;
                case T_Color::YELLOW: sfColor = sf::Color::Yellow; break;
                case T_Color::RED: sfColor = sf::Color::Red; break;
                case T_Color::GREEN: sfColor = sf::Color::Green; break;
                case T_Color::WHITE: sfColor = sf::Color::White; break;
            }
            
            sf::CircleShape cs;
            sf::RectangleShape rs;
            sf::ConvexShape convex;
            
            switch(shape) {
                case T_Shape::CIRCLE:
                    cs.setRadius(18);
                    cs.setFillColor(sfColor);
                    cs.setPosition(2, 2);
                    rt.draw(cs);
                    break;
                case T_Shape::SQUARE:
                    rs.setSize(sf::Vector2f(32, 32));
                    rs.setFillColor(sfColor);
                    rs.setPosition(4, 4);
                    rt.draw(rs);
                    break;
                case T_Shape::TRIANGLE:
                    convex.setPointCount(3);
                    convex.setPoint(0, sf::Vector2f(20, 4));
                    convex.setPoint(1, sf::Vector2f(4, 36));
                    convex.setPoint(2, sf::Vector2f(36, 36));
                    convex.setFillColor(sfColor);
                    rt.draw(convex);
                    break;
                case T_Shape::DIAMOND:
                    convex.setPointCount(4);
                    convex.setPoint(0, sf::Vector2f(20, 4));
                    convex.setPoint(1, sf::Vector2f(36, 20));
                    convex.setPoint(2, sf::Vector2f(20, 36));
                    convex.setPoint(3, sf::Vector2f(4, 20));
                    convex.setFillColor(sfColor);
                    rt.draw(convex);
                    break;
                case T_Shape::STAR:
                    convex.setPointCount(10);
                    for (int i = 0; i < 10; ++i) {
                        float angle = i * 2 * 3.14159f / 10;
                        float r = (i % 2 == 0) ? 18.0f : 8.0f;
                        convex.setPoint(i, sf::Vector2f(20 + r * std::cos(angle), 20 + r * std::sin(angle)));
                    }
                    convex.setFillColor(sfColor);
                    rt.draw(convex);
                    break;
            }
            rt.display();
            
            std::string name;
            switch(color) {
                case T_Color::BLUE: name = "blue"; break;
                case T_Color::YELLOW: name = "yellow"; break;
                case T_Color::RED: name = "red"; break;
                case T_Color::GREEN: name = "green"; break;
                case T_Color::WHITE: name = "white"; break;
            }
            std::string sname;
            switch(shape) {
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

    UserManager userManager;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Tetriste Mastery", sf::Style::Default);
    window.setFramerateLimit(60);

    AssetManager assets;
    
    #ifdef _WIN32
        system("if not exist assets mkdir assets");
    #else
        system("mkdir -p assets");
    #endif

    generatePlaceholderAssets(assets);
    
    std::vector<std::string> fontPaths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "assets/font.ttf"
    };

    for (const auto& path : fontPaths) {
        if (assets.loadFont("main", path)) break;
    }

    Renderer renderer(window, assets, userManager);
    sf::Sound sound;
    
    sf::Sound music;
    if (assets.hasSoundBuffer("music")) {
        music.setBuffer(assets.getSoundBuffer("music"));
        music.setLoop(true);
        music.setVolume(50.0f);
        music.play();
    }

    GameState state = GameState::LOGIN;
    Game *currentGame = nullptr;
    Piece *nextPiece = nullptr;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::TextEntered && state == GameState::LOGIN) {
                renderer.handleTextInput(event.text.unicode);
            }
            
            if (event.type == sf::Event::KeyPressed) {
                if (state == GameState::LOGIN) {
                    if (event.key.code == sf::Keyboard::Tab) {
                        renderer.switchLoginField();
                    } else if (event.key.code == sf::Keyboard::Enter) {
                        std::string p = renderer.getLoginPseudo();
                        std::string pw = renderer.getLoginPassword();
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) || sf::Keyboard::isKeyPressed(sf::Keyboard::RShift)) {
                            if (userManager.registerUser(p, pw)) {
                                renderer.addPopup("Registered! Please login", sf::Vector2f(400, 500), sf::Color::Green);
                            } else {
                                renderer.addPopup("Pseudo taken!", sf::Vector2f(400, 500), sf::Color::Red);
                            }
                        } else {
                            if (userManager.login(p, pw)) {
                                state = GameState::MENU;
                                renderer.addPopup("Welcome, " + p + "!", sf::Vector2f(400, 300), sf::Color::Cyan);
                            } else {
                                renderer.addPopup("Invalid Credentials", sf::Vector2f(400, 500), sf::Color::Red);
                            }
                        }
                    }
                } else if (state == GameState::MENU) {
                    if (event.key.code == sf::Keyboard::Up) {
                        renderer.setMenuSelection((renderer.getMenuSelection() + 2) % 3);
                    } else if (event.key.code == sf::Keyboard::Down) {
                        renderer.setMenuSelection((renderer.getMenuSelection() + 1) % 3);
                    } else if (event.key.code == sf::Keyboard::Enter) {
                        int selection = renderer.getMenuSelection();
                        if (selection == 0) { // PLAY
                            if (currentGame) delete currentGame;
                            if (nextPiece) delete nextPiece;
                            currentGame = Game(0,0).initializeGame(rand() % 5, rand() % 5);
                            for (int i = 0; i < 5; ++i) {
                                Piece *p = currentGame->drawPiece(rand() % 5, rand() % 5);
                                currentGame->insertPieceInRight(currentGame, p);
                            }
                            nextPiece = currentGame->drawPiece(rand() % 5, rand() % 5);
                            state = GameState::PLAYING;
                        } else if (selection == 1) { // RULES
                            renderer.toggleAbout();
                        } else if (selection == 2) { // EXIT
                            window.close();
                        }
                    } else if (event.key.code == sf::Keyboard::A) {
                        renderer.toggleAbout();
                    }
                } else if (state == GameState::GAME_OVER) {
                    if (event.key.code == sf::Keyboard::Space) {
                        state = GameState::MENU;
                    }
                } else if (state == GameState::PAUSED) {
                    if (event.key.code == sf::Keyboard::Escape) {
                        state = GameState::PLAYING;
                    }
                } else if (state == GameState::PLAYING) {
                    if (event.key.code == sf::Keyboard::A) {
                        renderer.toggleAbout();
                        continue;
                    }
                    if (event.key.code == sf::Keyboard::Escape) {
                        state = GameState::PAUSED;
                        continue;
                    }

                    if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) {
                        float v = std::min(100.0f, assets.getVolume() + 5.0f);
                        assets.setVolume(v);
                        music.setVolume(v);
                        renderer.addPopup("Volume: " + std::to_string((int)v), sf::Vector2f(window.getSize().x - 100.0f, 30.0f));
                    } else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) {
                        float v = std::max(0.0f, assets.getVolume() - 5.0f);
                        assets.setVolume(v);
                        music.setVolume(v);
                        renderer.addPopup("Volume: " + std::to_string((int)v), sf::Vector2f(window.getSize().x - 100.0f, 30.0f));
                    }

                    if (nextPiece == nullptr && (currentGame == nullptr || currentGame->piecesCount == 0)) continue;

                    sf::Vector2u winSize = window.getSize();
                    float sidebarWidth = 200.0f;
                    float startX = 60.0f;
                    float startY = 60.0f;
                    float spacingX = 60.0f;
                    float spacingY = 80.0f;
                    float boardWidth = static_cast<float>(winSize.x) - sidebarWidth - startX;
                    int piecesPerRow = static_cast<int>(boardWidth / spacingX);
                    if (piecesPerRow < 1) piecesPerRow = 1;
                    int maxRows = static_cast<int>((static_cast<float>(winSize.y) - startY - 25.0f) / spacingY) + 1;
                    if (maxRows < 1) maxRows = 0;
                    int dynamicCapacity = piecesPerRow * maxRows;

                    bool actionTaken = false;
                    bool shifted = false;
                    if (event.key.code == sf::Keyboard::J) {
                        if (nextPiece != nullptr) {
                            if (currentGame->piecesCount < dynamicCapacity) {
                                if (currentGame->insertPieceInLeft(currentGame, nextPiece)) actionTaken = true;
                            } else {
                                state = GameState::GAME_OVER;
                                userManager.updateRecord(currentGame->score);
                            }
                        }
                    } else if (event.key.code == sf::Keyboard::K) {
                        if (nextPiece != nullptr) {
                            if (currentGame->piecesCount < dynamicCapacity) {
                                if (currentGame->insertPieceInRight(currentGame, nextPiece)) actionTaken = true;
                            } else {
                                state = GameState::GAME_OVER;
                                userManager.updateRecord(currentGame->score);
                            }
                        }
                    } else if (event.key.code == sf::Keyboard::C) {
                        if (nextPiece != nullptr) {
                            currentGame->colorShifting(currentGame, nextPiece->color, 0); 
                            shifted = true;
                        }
                    } else if (event.key.code == sf::Keyboard::S) {
                        if (nextPiece != nullptr) {
                            currentGame->shapeShifting(currentGame, nextPiece->shape, 0);
                            shifted = true;
                        }
                    }
                    
                    if (actionTaken && currentGame->piecesCount > 0) {
                        if (assets.hasSoundBuffer("place")) {
                            sound.setBuffer(assets.getSoundBuffer("place"));
                            sound.setVolume(assets.getVolume());
                            sound.play();
                        }
                        int scoreChange = currentGame->updateGame(currentGame);
                        if (scoreChange > 0) {
                            if (assets.hasSoundBuffer("match")) {
                                sound.setBuffer(assets.getSoundBuffer("match"));
                                sound.setVolume(assets.getVolume());
                                sound.play();
                            }
                            renderer.addPopup("+" + std::to_string(scoreChange), sf::Vector2f(window.getSize().x / 2.0f, 100.0f));
                            userManager.updateRecord(currentGame->score);
                        }

                        if (currentGame->piecesCount == 0) {
                            renderer.addPopup("BOARD CLEARED!", sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f), sf::Color::Green);
                        }
                        nextPiece = currentGame->drawPiece(rand() % 5, rand() % 5);
                        }
                        if (shifted) {
                        if (assets.hasSoundBuffer("shift")) {
                            sound.setBuffer(assets.getSoundBuffer("shift"));
                            sound.setVolume(assets.getVolume());
                            sound.play();
                        }
                        }                }
            }
        }

        if (state == GameState::PLAYING || state == GameState::PAUSED) {
            renderer.render(*currentGame, nextPiece, state);
        } else {
            static Game dummyGame(0,0);
            renderer.render(dummyGame, nullptr, state);
        }
    }

    if (currentGame) delete currentGame;
    if (nextPiece) delete nextPiece;
    
    return 0;
}
