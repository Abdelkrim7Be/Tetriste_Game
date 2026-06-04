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
            sf::CircleShape cs; sf::RectangleShape rs; sf::ConvexShape convex;
            switch(shape) {
                case T_Shape::CIRCLE: cs.setRadius(18); cs.setFillColor(sfColor); cs.setPosition(2, 2); rt.draw(cs); break;
                case T_Shape::SQUARE: rs.setSize(sf::Vector2f(32, 32)); rs.setFillColor(sfColor); rs.setPosition(4, 4); rt.draw(rs); break;
                case T_Shape::TRIANGLE: convex.setPointCount(3); convex.setPoint(0, sf::Vector2f(20, 4)); convex.setPoint(1, sf::Vector2f(4, 36)); convex.setPoint(2, sf::Vector2f(36, 36)); convex.setFillColor(sfColor); rt.draw(convex); break;
                case T_Shape::DIAMOND: convex.setPointCount(4); convex.setPoint(0, sf::Vector2f(20, 4)); convex.setPoint(1, sf::Vector2f(36, 20)); convex.setPoint(2, sf::Vector2f(20, 36)); convex.setPoint(3, sf::Vector2f(4, 20)); convex.setFillColor(sfColor); rt.draw(convex); break;
                case T_Shape::STAR: convex.setPointCount(10); for (int i = 0; i < 10; ++i) { float angle = i * 2 * 3.14159f / 10; float r = (i % 2 == 0) ? 18.0f : 8.0f; convex.setPoint(i, sf::Vector2f(20 + r * std::cos(angle), 20 + r * std::sin(angle))); } convex.setFillColor(sfColor); rt.draw(convex); break;
            }
            rt.display();
            std::string name, sname;
            switch(color) { case T_Color::BLUE: name = "blue"; break; case T_Color::YELLOW: name = "yellow"; break; case T_Color::RED: name = "red"; break; case T_Color::GREEN: name = "green"; break; case T_Color::WHITE: name = "white"; break; }
            switch(shape) { case T_Shape::SQUARE: sname = "square"; break; case T_Shape::DIAMOND: sname = "diamond"; break; case T_Shape::CIRCLE: sname = "circle"; break; case T_Shape::TRIANGLE: sname = "triangle"; break; case T_Shape::STAR: sname = "star"; break; }
            sf::Image img = rt.getTexture().copyToImage();
            std::string filename = "assets/" + name + "_" + sname + ".png";
            img.saveToFile(filename);
            assets.loadTexture(name + "_" + sname, filename);
        }
    }
}

void generatePlaceholderAudio(AssetManager& assets) {
    auto createTone = [](float freq, float duration, float volume = 0.5f) {
        const unsigned int sampleRate = 44100;
        const unsigned int sampleCount = static_cast<unsigned int>(sampleRate * duration);
        std::vector<sf::Int16> samples(sampleCount);
        for (unsigned int i = 0; i < sampleCount; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            // Sine wave with basic envelope
            float envelope = 1.0f;
            if (t < 0.01f) envelope = t / 0.01f;
            if (t > duration - 0.05f) envelope = (duration - t) / 0.05f;
            samples[i] = static_cast<sf::Int16>(32767 * volume * envelope * std::sin(2 * 3.14159f * freq * t));
        }
        sf::SoundBuffer buffer;
        buffer.loadFromSamples(&samples[0], sampleCount, 1, sampleRate);
        return buffer;
    };

    assets.addSoundBuffer("place", createTone(440.0f, 0.1f, 0.3f));  // A4
    assets.addSoundBuffer("match", createTone(880.0f, 0.2f, 0.4f));  // A5
    assets.addSoundBuffer("shift", createTone(220.0f, 0.15f, 0.3f)); // A3

    // Background Music (4-note melody loop)
    const unsigned int sampleRate = 44100;
    const float duration = 4.0f; // 4 seconds loop
    const unsigned int sampleCount = static_cast<unsigned int>(sampleRate * duration);
    std::vector<sf::Int16> samples(sampleCount, 0);
    float freqs[] = { 261.63f, 329.63f, 392.00f, 349.23f }; // C4, E4, G4, F4
    for (int n = 0; n < 4; ++n) {
        float startTime = n * 1.0f;
        for (unsigned int i = static_cast<unsigned int>(startTime * sampleRate); i < (n+1) * sampleRate; ++i) {
            float t = static_cast<float>(i) / sampleRate;
            float localT = t - startTime;
            float envelope = std::exp(-3.0f * localT); // Plucky sound
            samples[i] = static_cast<sf::Int16>(10000 * envelope * std::sin(2 * 3.14159f * freqs[n] * t));
        }
    }
    sf::SoundBuffer musicBuffer;
    musicBuffer.loadFromSamples(&samples[0], sampleCount, 1, sampleRate);
    assets.addSoundBuffer("music", musicBuffer);
}

void initializeNewGame(Game*& currentGame, Piece*& nextPiece) {
    std::cout << "DEBUG: Initializing new game..." << std::endl;
    if (currentGame) {
        delete currentGame;
        currentGame = nullptr;
    }
    // nextPiece was owned by the old currentGame or is a loose piece from drawPiece
    // If it was from drawPiece(rand, rand) and not inserted, it's a leak or belongs to the game
    // For simplicity, let's assume initializeGame handles its own pieces.
    nextPiece = nullptr; 

    currentGame = Game(0,0).initializeGame(rand() % 5, rand() % 5);
    for (int i = 0; i < 5; ++i) {
        Piece *p = currentGame->drawPiece(rand() % 5, rand() % 5);
        currentGame->insertPieceInRight(currentGame, p);
    }
    nextPiece = currentGame->drawPiece(rand() % 5, rand() % 5);
    std::cout << "DEBUG: New game initialized. piecesCount=" << currentGame->piecesCount << std::endl;
}

void handlePlayingInput(sf::Event& event, sf::RenderWindow& window, GameState& state, Game* currentGame, Piece*& nextPiece, Renderer& renderer, AssetManager& assets, UserManager& userManager, sf::Sound& sound, sf::Sound& music) {
    if (event.key.code == sf::Keyboard::A) { renderer.toggleAbout(); return; }
    if (event.key.code == sf::Keyboard::Escape) { state = GameState::PAUSED; return; }
    if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) {
        float v = std::min(100.0f, assets.getVolume() + 5.0f); assets.setVolume(v); music.setVolume(v); renderer.addPopup("Volume: " + std::to_string((int)v), sf::Vector2f(window.getSize().x - 100.0f, 30.0f));
    } else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) {
        float v = std::max(0.0f, assets.getVolume() - 5.0f); assets.setVolume(v); music.setVolume(v); renderer.addPopup("Volume: " + std::to_string((int)v), sf::Vector2f(window.getSize().x - 100.0f, 30.0f));
    }
    if (nextPiece == nullptr && (currentGame == nullptr || currentGame->piecesCount == 0)) return;
    sf::Vector2u winSize = window.getSize();
    float sidebarWidth = 200.0f; float startX = 60.0f; float startY = 60.0f; float spacingX = 60.0f; float spacingY = 80.0f;
    float boardWidth = static_cast<float>(winSize.x) - sidebarWidth - startX;
    int piecesPerRow = static_cast<int>(boardWidth / spacingX); if (piecesPerRow < 1) piecesPerRow = 1;
    int maxRows = static_cast<int>((static_cast<float>(winSize.y) - startY - 25.0f) / spacingY) + 1; if (maxRows < 1) maxRows = 0;
    int dynamicCapacity = piecesPerRow * maxRows;
    bool actionTaken = false; bool shifted = false;
    if (event.key.code == sf::Keyboard::J) {
        if (nextPiece != nullptr) { if (currentGame->piecesCount < dynamicCapacity) { if (currentGame->insertPieceInLeft(currentGame, nextPiece)) { actionTaken = true; renderer.triggerInsertionEffect(); } } else { state = GameState::GAME_OVER; userManager.updateRecord(currentGame->score); } }
    } else if (event.key.code == sf::Keyboard::K) {
        if (nextPiece != nullptr) { if (currentGame->piecesCount < dynamicCapacity) { if (currentGame->insertPieceInRight(currentGame, nextPiece)) { actionTaken = true; renderer.triggerInsertionEffect(); } } else { state = GameState::GAME_OVER; userManager.updateRecord(currentGame->score); } }
    } else if (event.key.code == sf::Keyboard::C) { if (nextPiece != nullptr) { currentGame->colorShifting(currentGame, nextPiece->color, 0); shifted = true; renderer.triggerFlash(); }
    } else if (event.key.code == sf::Keyboard::S) { if (nextPiece != nullptr) { currentGame->shapeShifting(currentGame, nextPiece->shape, 0); shifted = true; renderer.triggerFlash(); } }
    if (actionTaken && currentGame->piecesCount > 0) {
        if (assets.hasSoundBuffer("place")) { sound.setBuffer(assets.getSoundBuffer("place")); sound.setVolume(assets.getVolume()); sound.play(); }
        int scoreChange = currentGame->updateGame(currentGame);
        if (scoreChange > 0) {
            if (assets.hasSoundBuffer("match")) { sound.setBuffer(assets.getSoundBuffer("match")); sound.setVolume(assets.getVolume()); sound.play(); }
            std::string popupText = "+" + std::to_string(scoreChange);
            if (currentGame->globalComboMultiplier > 1) popupText += " (COMBO x" + std::to_string(currentGame->globalComboMultiplier) + "!)";
            renderer.addPopup(popupText, sf::Vector2f(window.getSize().x / 2.0f, 100.0f)); 
            
            // Stats update
            UserProfile p = userManager.getCurrentUserProfile();
            p.nodesPurged += currentGame->nodesPurgedInLastUpdate;
            
            // Achievement: Veteran Purger (100 nodes)
            auto hasAch = [&](std::string id) { return std::find(p.achievements.begin(), p.achievements.end(), id) != p.achievements.end(); };
            if (p.nodesPurged >= 100 && !hasAch("vet_purger")) {
                p.achievements.push_back("vet_purger");
                renderer.addPopup("ACHIEVEMENT: VETERAN PURGER", sf::Vector2f(400, 200), sf::Color::Magenta);
            }
            
            userManager.updateProfile(p);
            userManager.updateRecord(currentGame->score);
            currentGame->globalComboMultiplier++;
        } else {
            currentGame->globalComboMultiplier = 1;
        }
        if (currentGame->piecesCount == 0) { 
            renderer.addPopup("BOARD CLEARED!", sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f), sf::Color::Green);
            renderer.triggerFlash();
        }
        nextPiece = currentGame->drawPiece(rand() % 5, rand() % 5);
    }
    if (shifted && assets.hasSoundBuffer("shift")) { sound.setBuffer(assets.getSoundBuffer("shift")); sound.setVolume(assets.getVolume()); sound.play(); }
}

int main() {
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
    generatePlaceholderAudio(assets);
    std::vector<std::string> fontPaths = { "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/TTF/DejaVuSans.ttf", "/usr/share/fonts/liberation/LiberationSans-Regular.ttf", "C:\\Windows\\Fonts\\arial.ttf", "assets/font.ttf" };
    bool fontLoaded = false;
    for (const auto& path : fontPaths) { if (assets.loadFont("main", path)) { fontLoaded = true; break; } }
    if (!fontLoaded) std::cerr << "Warning: No system fonts found." << std::endl;
    Renderer renderer(window, assets, userManager);
    sf::Sound sound, music;
    if (assets.hasSoundBuffer("music")) { music.setBuffer(assets.getSoundBuffer("music")); music.setLoop(true); music.setVolume(50.0f); music.play(); }
    GameState state = GameState::LOGIN; Game *currentGame = nullptr; Piece *nextPiece = nullptr;
    std::string tempPseudo = "";

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::TextEntered && state == GameState::LOGIN) renderer.handleTextInput(event.text.unicode);
            if (event.type == sf::Event::KeyPressed) {
                if (state == GameState::LOGIN) {
                    AuthState auth = renderer.getAuthState();
                    if (event.key.code == sf::Keyboard::Enter) {
                        if (auth == AuthState::PSEUDO) {
                            tempPseudo = renderer.getLoginPseudo();
                            // Trim
                            tempPseudo.erase(0, tempPseudo.find_first_not_of(" \t\r\n"));
                            tempPseudo.erase(tempPseudo.find_last_not_of(" \t\r\n") + 1);

                            if (tempPseudo.empty()) {
                                renderer.addPopup("Name cannot be empty!", sf::Vector2f(400, 500), sf::Color::Red);
                            } else {
                                if (userManager.userExists(tempPseudo)) {
                                    renderer.setAuthState(AuthState::PIN_ENTRY);
                                    renderer.addPopup("Welcome back, " + tempPseudo + "!", sf::Vector2f(400, 300), sf::Color::Cyan);
                                } else {
                                    renderer.setAuthState(AuthState::PIN_SETUP);
                                    renderer.addPopup("New Profile: Set PIN", sf::Vector2f(400, 300), sf::Color::Green);
                                }
                            }
                        } else if (auth == AuthState::PIN_ENTRY) {
                            std::string pin = renderer.getLoginPin();
                            if (pin.length() != 4) {
                                renderer.addPopup("PIN must be 4 digits", sf::Vector2f(400, 500), sf::Color::Red);
                            } else if (userManager.verifyPin(tempPseudo, pin)) {
                                if (userManager.loginOrCreate(tempPseudo, pin)) {
                                    state = GameState::MENU;
                                    renderer.addPopup("Login Successful!", sf::Vector2f(400, 300), sf::Color::Cyan);
                                }
                            } else {
                                renderer.addPopup("Incorrect PIN", sf::Vector2f(400, 500), sf::Color::Red);
                            }
                        } else if (auth == AuthState::PIN_SETUP) {
                            std::string pin = renderer.getLoginPin();
                            if (pin.length() != 4) {
                                renderer.addPopup("Set 4-digit PIN", sf::Vector2f(400, 500), sf::Color::Red);
                            } else {
                                renderer.setAuthState(AuthState::AVATAR_SELECT);
                            }
                        } else if (auth == AuthState::AVATAR_SELECT) {
                            std::string pin = renderer.getLoginPin();
                            if (userManager.loginOrCreate(tempPseudo, pin)) {
                                UserProfile p = userManager.getCurrentUserProfile();
                                int idx = renderer.getAvatarIndex();
                                T_Color c = (T_Color)(idx / 5);
                                T_Shape s = (T_Shape)(idx % 5);
                                std::string cname, sname;
                                switch(c) { case T_Color::BLUE: cname = "blue"; break; case T_Color::YELLOW: cname = "yellow"; break; case T_Color::RED: cname = "red"; break; case T_Color::GREEN: cname = "green"; break; case T_Color::WHITE: cname = "white"; break; }
                                switch(s) { case T_Shape::SQUARE: sname = "square"; break; case T_Shape::DIAMOND: sname = "diamond"; break; case T_Shape::CIRCLE: sname = "circle"; break; case T_Shape::TRIANGLE: sname = "triangle"; break; case T_Shape::STAR: sname = "star"; break; }
                                p.avatarId = cname + "_" + sname;
                                userManager.updateProfile(p);
                                state = GameState::MENU;
                                renderer.addPopup("Profile Finalized!", sf::Vector2f(400, 300), sf::Color::Green);
                            }
                        }
                    } else if (auth == AuthState::AVATAR_SELECT) {
                        int idx = renderer.getAvatarIndex();
                        if (event.key.code == sf::Keyboard::Up) idx = (idx + 20) % 25;
                        else if (event.key.code == sf::Keyboard::Down) idx = (idx + 5) % 25;
                        else if (event.key.code == sf::Keyboard::Left) idx = (idx + 24) % 25;
                        else if (event.key.code == sf::Keyboard::Right) idx = (idx + 1) % 25;
                        renderer.setAvatarIndex(idx);
                    } else if (event.key.code == sf::Keyboard::Escape) {
                        renderer.setAuthState(AuthState::PSEUDO);
                        renderer.clearLoginField();
                    }
                } else if (state == GameState::MENU) {
                    if (event.key.code == sf::Keyboard::Up) renderer.setMenuSelection((renderer.getMenuSelection() + 4) % 5);
                    else if (event.key.code == sf::Keyboard::Down) renderer.setMenuSelection((renderer.getMenuSelection() + 1) % 5);
                    else if (event.key.code == sf::Keyboard::Enter) {
                        int selection = renderer.getMenuSelection();
                        if (selection == 0) { state = GameState::DIFFICULTY_SELECT; }
                        else if (selection == 1) state = GameState::ACHIEVEMENTS;
                        else if (selection == 2) state = GameState::SETTINGS;
                        else if (selection == 3) renderer.toggleAbout();
                        else if (selection == 4) window.close();
                    } else if (event.key.code == sf::Keyboard::A) renderer.toggleAbout();
                } else if (state == GameState::ACHIEVEMENTS || state == GameState::SETTINGS) {
                    if (event.key.code == sf::Keyboard::Escape) state = GameState::MENU;
                    if (state == GameState::SETTINGS) {
                        if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) {
                            float v = std::min(100.0f, assets.getVolume() + 5.0f); assets.setVolume(v); music.setVolume(v);
                        } else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) {
                            float v = std::max(0.0f, assets.getVolume() - 5.0f); assets.setVolume(v); music.setVolume(v);
                        }
                    }
                } else if (state == GameState::DIFFICULTY_SELECT) {
                    if (event.key.code == sf::Keyboard::Up) renderer.setDifficulty((Difficulty)(((int)renderer.getDifficulty() + 2) % 3));
                    else if (event.key.code == sf::Keyboard::Down) renderer.setDifficulty((Difficulty)(((int)renderer.getDifficulty() + 1) % 3));
                    else if (event.key.code == sf::Keyboard::Enter) {
                        initializeNewGame(currentGame, nextPiece);
                        state = GameState::PLAYING;
                    } else if (event.key.code == sf::Keyboard::Escape) state = GameState::MENU;
                } else if (state == GameState::GAME_OVER) { 
                    if (event.key.code == sf::Keyboard::Space) state = GameState::MENU; 
                } else if (state == GameState::PAUSED) { if (event.key.code == sf::Keyboard::Escape) state = GameState::PLAYING; }
                else if (state == GameState::PLAYING) handlePlayingInput(event, window, state, currentGame, nextPiece, renderer, assets, userManager, sound, music);
            }
        }
        if (state == GameState::PLAYING) {
            // Check for game over (capacity exceeded)
            sf::Vector2u winSize = window.getSize();
            float sidebarWidth = 240.0f; float startX = 60.0f; float startY = 60.0f; float spacingX = 60.0f; float spacingY = 80.0f;
            float boardWidth = static_cast<float>(winSize.x) - sidebarWidth - startX;
            int piecesPerRow = static_cast<int>(boardWidth / spacingX); if (piecesPerRow < 1) piecesPerRow = 1;
            int maxRows = static_cast<int>((static_cast<float>(winSize.y) - startY - 25.0f) / spacingY) + 1;
            int capacity = piecesPerRow * maxRows;
            
            // Apply difficulty modifiers
            if (renderer.getDifficulty() == Difficulty::VETERAN) capacity = static_cast<int>(capacity * 0.8);
            else if (renderer.getDifficulty() == Difficulty::ELITE) capacity = static_cast<int>(capacity * 0.6);

            if (currentGame->piecesCount >= capacity) {
                state = GameState::GAME_OVER;
                UserProfile p = userManager.getCurrentUserProfile();
                p.matchesPlayed++;
                // Achievements
                auto hasAch = [&](std::string id) { return std::find(p.achievements.begin(), p.achievements.end(), id) != p.achievements.end(); };
                if (p.matchesPlayed >= 1 && !hasAch("first_match")) { p.achievements.push_back("first_match"); renderer.addPopup("ACHIEVEMENT: PURGE INITIATE", sf::Vector2f(400, 200), sf::Color::Magenta); }
                if (renderer.getDifficulty() == Difficulty::ELITE && !hasAch("elite_op")) { p.achievements.push_back("elite_op"); renderer.addPopup("ACHIEVEMENT: ELITE OPERATIVE", sf::Vector2f(400, 200), sf::Color::Magenta); }
                
                userManager.updateProfile(p);
                userManager.updateRecord(currentGame->score);
            }
        }
        if (state == GameState::PLAYING || state == GameState::PAUSED) renderer.render(*currentGame, nextPiece, state);
        else { static Game dummyGame(0,0); renderer.render(dummyGame, nullptr, state); }
    }
    if (currentGame) delete currentGame;
    nextPiece = nullptr; 
    return 0;
}
