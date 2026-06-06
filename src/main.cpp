#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <algorithm>
#include "gameDeclaration.h"
#include "pieceDeclaration.h"
#include "AssetManager.h"
#include "Renderer.h"
#include "UserManager.h"

static std::string resolveAssetsDir() {
    const char* candidates[] = {"assets", "build/assets", "../assets"};
    for (const char* dir : candidates) {
        std::string probe = std::string(dir) + "/blue_square.png";
        std::ifstream f(probe);
        if (f.good()) return dir;
    }
    return "assets";
}

void generatePlaceholderAssets(AssetManager& assets) {
    std::string assetsDir = resolveAssetsDir();
#ifdef _WIN32
    system(("if not exist " + assetsDir + " mkdir " + assetsDir).c_str());
#else
    system(("mkdir -p " + assetsDir).c_str());
#endif
    const unsigned texSize = UI::PieceTexPx;
    sf::RenderTexture rt;
    rt.create(texSize, texSize);
    rt.setSmooth(false);
    const float cx = texSize / 2.0f, cy = texSize / 2.0f;
    const float body = (float)texSize * 0.88f;
    const float half = body * 0.5f;
    const float outline = 1.5f;
    const sf::Color edge(220, 235, 245, 200);
    for (int c = 0; c < 5; ++c) {
        for (int s = 0; s < 5; ++s) {
            rt.clear(sf::Color::Transparent);
            T_Color color = static_cast<T_Color>(c);
            T_Shape shape = static_cast<T_Shape>(s);
            sf::Color sfColor;
            switch(color) {
                case T_Color::BLUE: sfColor = sf::Color(30, 120, 255); break;
                case T_Color::YELLOW: sfColor = sf::Color(255, 220, 40); break;
                case T_Color::RED: sfColor = sf::Color(255, 60, 60); break;
                case T_Color::GREEN: sfColor = sf::Color(50, 220, 80); break;
                case T_Color::WHITE: sfColor = sf::Color(240, 240, 240); break;
            }
            sf::CircleShape cs;
            sf::RectangleShape rs;
            sf::ConvexShape convex;
            switch(shape) {
                case T_Shape::CIRCLE:
                    cs.setRadius(half);
                    cs.setOrigin(half, half);
                    cs.setPosition(cx, cy);
                    cs.setFillColor(sfColor);
                    cs.setOutlineThickness(outline);
                    cs.setOutlineColor(edge);
                    rt.draw(cs);
                    break;
                case T_Shape::SQUARE:
                    rs.setSize(sf::Vector2f(body, body));
                    rs.setOrigin(half, half);
                    rs.setPosition(cx, cy);
                    rs.setFillColor(sfColor);
                    rs.setOutlineThickness(outline);
                    rs.setOutlineColor(edge);
                    rt.draw(rs);
                    break;
                case T_Shape::TRIANGLE:
                    convex.setPointCount(3);
                    convex.setPoint(0, sf::Vector2f(cx, cy - half));
                    convex.setPoint(1, sf::Vector2f(cx - half, cy + half * 0.82f));
                    convex.setPoint(2, sf::Vector2f(cx + half, cy + half * 0.82f));
                    convex.setFillColor(sfColor);
                    convex.setOutlineThickness(outline);
                    convex.setOutlineColor(edge);
                    rt.draw(convex);
                    break;
                case T_Shape::DIAMOND:
                    convex.setPointCount(4);
                    convex.setPoint(0, sf::Vector2f(cx, cy - half));
                    convex.setPoint(1, sf::Vector2f(cx + half, cy));
                    convex.setPoint(2, sf::Vector2f(cx, cy + half));
                    convex.setPoint(3, sf::Vector2f(cx - half, cy));
                    convex.setFillColor(sfColor);
                    convex.setOutlineThickness(outline);
                    convex.setOutlineColor(edge);
                    rt.draw(convex);
                    break;
                case T_Shape::STAR:
                    convex.setPointCount(10);
                    for (int i = 0; i < 10; ++i) {
                        float angle = -3.14159f / 2.0f + i * 2 * 3.14159f / 10;
                        float r = (i % 2 == 0) ? half : half * 0.42f;
                        convex.setPoint(i, sf::Vector2f(cx + r * std::cos(angle), cy + r * std::sin(angle)));
                    }
                    convex.setFillColor(sfColor);
                    convex.setOutlineThickness(outline);
                    convex.setOutlineColor(edge);
                    rt.draw(convex);
                    break;
            }
            rt.display();
            std::string name, sname;
            switch(color) { case T_Color::BLUE: name = "blue"; break; case T_Color::YELLOW: name = "yellow"; break; case T_Color::RED: name = "red"; break; case T_Color::GREEN: name = "green"; break; case T_Color::WHITE: name = "white"; break; }
            switch(shape) { case T_Shape::SQUARE: sname = "square"; break; case T_Shape::DIAMOND: sname = "diamond"; break; case T_Shape::CIRCLE: sname = "circle"; break; case T_Shape::TRIANGLE: sname = "triangle"; break; case T_Shape::STAR: sname = "star"; break; }
            sf::Image img = rt.getTexture().copyToImage();
            std::string filename = assetsDir + "/" + name + "_" + sname + ".png";
            img.saveToFile(filename);
            if (!assets.loadTexture(name + "_" + sname, filename))
                std::cerr << "Failed to load texture: " << filename << std::endl;
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

static int computeBoardCapacity(sf::Vector2u winSize, Difficulty difficulty) {
    const float hudW = UI::PlayHudW;
    const float playX = 12.0f;
    const float playY = 40.0f;
    float u = UI::uiScaleFrom((float)winSize.x, (float)winSize.y);
    float spacing = UI::pieceSpacing(u);
    float playW = (float)winSize.x - hudW - playX - 8.0f;
    float playH = (float)winSize.y - playY - 48.0f - 8.0f;
    float gridOx, gridOy;
    UI::playGridOrigin(playX, playY, gridOx, gridOy);
    int pPerRow = std::max(1, (int)((playW - UI::PlayGridMarginX * 2) / spacing));
    float gridH = playH - UI::PlayLabelBand - UI::PlayGridMarginY - 6.0f;
    int mRows = std::max(1, (int)(gridH / spacing));
    int cap = pPerRow * mRows;
    if (difficulty == Difficulty::VETERAN) cap = (int)(cap * 0.8f);
    else if (difficulty == Difficulty::ELITE) cap = (int)(cap * 0.6f);
    return std::max(cap, 12);
}

void initializeNewGame(Game*& currentGame, Piece*& nextPiece) {
    nextPiece = nullptr;
    if (currentGame) {
        delete currentGame;
        currentGame = nullptr;
    }
    static const int bootColors[] = {0, 1, 2, 3, 4, 0, 1, 2};
    static const int bootShapes[] = {0, 1, 2, 3, 4, 1, 2, 3};
    currentGame = Game(0, 0).initializeGame(bootColors[0], bootShapes[0]);
    for (int i = 1; i < 8; ++i) {
        Piece* p = currentGame->drawPiece(bootColors[i], bootShapes[i]);
        currentGame->insertPieceInRight(currentGame, p);
    }
    nextPiece = currentGame->drawPiece(rand() % 5, rand() % 5);
}

static void finalizeActiveSession(UserManager& userManager, Renderer& renderer, Game* currentGame, bool& sessionActive) {
    if (!sessionActive || !currentGame || !userManager.isLoggedIn()) return;

    UserProfile profile = userManager.getCurrentUserProfile();
    auto hasAchievement = [&](const std::string& id) {
        return std::find(profile.achievements.begin(), profile.achievements.end(), id) != profile.achievements.end();
    };
    if (renderer.getDifficulty() == Difficulty::ELITE && !hasAchievement("elite_op")) {
        profile.achievements.push_back("elite_op");
        userManager.updateProfile(profile);
        renderer.addPopup("ACHIEVEMENT: ELITE OPERATIVE", sf::Vector2f(400, 200), sf::Color::Magenta);
    }

    userManager.endSession(currentGame->score);
    sessionActive = false;
}

void handlePlayingInput(sf::Event& event, sf::RenderWindow& window, GameState& state, Game* currentGame, Piece*& nextPiece, Renderer& renderer, AssetManager& assets, UserManager& userManager, sf::Sound& sound, sf::Sound& music, bool& sessionActive) {
    if (!currentGame) return;
    if (event.key.code == sf::Keyboard::P) { renderer.toggleAbout(); return; }
    if (event.key.code == sf::Keyboard::Escape) { state = GameState::PAUSED; return; }
    if (event.key.code == sf::Keyboard::Add || event.key.code == sf::Keyboard::Equal) {
        float v = std::min(100.0f, assets.getVolume() + 5.0f); assets.setVolume(v); music.setVolume(v);
        renderer.addPopup("Volume: " + std::to_string((int)v), sf::Vector2f(window.getSize().x - 100.0f, 30.0f));
    } else if (event.key.code == sf::Keyboard::Subtract || event.key.code == sf::Keyboard::Dash) {
        float v = std::max(0.0f, assets.getVolume() - 5.0f); assets.setVolume(v); music.setVolume(v);
        renderer.addPopup("Volume: " + std::to_string((int)v), sf::Vector2f(window.getSize().x - 100.0f, 30.0f));
    }

    int capacity = computeBoardCapacity(window.getSize(), renderer.getDifficulty());
    bool actionTaken = false;
    bool shifted = false;

    auto tryInsertLeft = [&]() {
        if (!nextPiece) return;
        if (currentGame->piecesCount >= capacity) {
            state = GameState::GAME_OVER;
            finalizeActiveSession(userManager, renderer, currentGame, sessionActive);
            return;
        }
        if (currentGame->insertPieceInLeft(currentGame, nextPiece)) {
            actionTaken = true;
            nextPiece = nullptr;
            renderer.triggerInsertionEffect();
        }
    };
    auto tryInsertRight = [&]() {
        if (!nextPiece) return;
        if (currentGame->piecesCount >= capacity) {
            state = GameState::GAME_OVER;
            finalizeActiveSession(userManager, renderer, currentGame, sessionActive);
            return;
        }
        if (currentGame->insertPieceInRight(currentGame, nextPiece)) {
            actionTaken = true;
            nextPiece = nullptr;
            renderer.triggerInsertionEffect();
        }
    };

    if (event.key.code == sf::Keyboard::J || event.key.code == sf::Keyboard::A)
        tryInsertLeft();
    else if (event.key.code == sf::Keyboard::K || event.key.code == sf::Keyboard::D)
        tryInsertRight();
    else if (event.key.code == sf::Keyboard::C && nextPiece) {
        currentGame->colorShifting(currentGame, nextPiece->color, 0);
        shifted = true;
        renderer.triggerFlash();
    } else if (event.key.code == sf::Keyboard::S && nextPiece) {
        currentGame->shapeShifting(currentGame, nextPiece->shape, 0);
        shifted = true;
        renderer.triggerFlash();
    }

    auto resolveMatches = [&](bool spawnNextPiece, bool playPlaceSound) {
        renderer.enableMatchHighlight();
        if (playPlaceSound && assets.hasSoundBuffer("place")) {
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
            std::string popupText = "+" + std::to_string(scoreChange);
            if (currentGame->globalComboMultiplier > 1)
                popupText += " (COMBO x" + std::to_string(currentGame->globalComboMultiplier) + "!)";
            renderer.addPopup(popupText, sf::Vector2f(window.getSize().x / 2.0f, 100.0f));

            userManager.addNodesPurged(currentGame->nodesPurgedInLastUpdate);
            UserProfile profile = userManager.getCurrentUserProfile();
            auto hasAchievement = [&](const std::string& id) {
                return std::find(profile.achievements.begin(), profile.achievements.end(), id) != profile.achievements.end();
            };
            bool profileDirty = false;
            if (profile.nodesPurged > 0 && !hasAchievement("first_match")) {
                profile.achievements.push_back("first_match");
                profileDirty = true;
                renderer.addPopup("ACHIEVEMENT: PURGE INITIATE", sf::Vector2f(400, 200), sf::Color::Magenta);
            }
            if (profile.nodesPurged >= 100 && !hasAchievement("vet_purger")) {
                profile.achievements.push_back("vet_purger");
                profileDirty = true;
                renderer.addPopup("ACHIEVEMENT: VETERAN PURGER", sf::Vector2f(400, 200), sf::Color::Magenta);
            }
            if (profileDirty) userManager.updateProfile(profile);
            currentGame->globalComboMultiplier++;
        } else if (scoreChange < 0 || currentGame->piecesCount == 0) {
            renderer.addPopup("BOARD CLEARED!", sf::Vector2f(window.getSize().x / 2.0f, window.getSize().y / 2.0f), sf::Color::Green);
            renderer.triggerFlash();
            currentGame->globalComboMultiplier = 1;
            if (currentGame->piecesCount == 0) {
                Piece *seed = currentGame->drawPiece(rand() % 5, rand() % 5);
                currentGame->insertPieceInRight(currentGame, seed);
                renderer.resetMatchHighlight();
            }
        } else {
            currentGame->globalComboMultiplier = 1;
        }
        if (spawnNextPiece)
            nextPiece = currentGame->drawPiece(rand() % 5, rand() % 5);
    };

    if (actionTaken)
        resolveMatches(true, true);
    else if (shifted) {
        if (assets.hasSoundBuffer("shift")) {
            sound.setBuffer(assets.getSoundBuffer("shift"));
            sound.setVolume(assets.getVolume());
            sound.play();
        }
        resolveMatches(false, false);
    }
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    const std::string assetsDir = resolveAssetsDir();
    UserManager userManager(assetsDir + "/users.json");
    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    unsigned winW = desktop.width;
    unsigned winH = desktop.height > 56 ? desktop.height - 56 : desktop.height;
    sf::VideoMode screenSize(winW, winH);
    const sf::Uint32 windowStyle = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;
    sf::RenderWindow window(screenSize, "Tetriste", windowStyle);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
    AssetManager assets(assetsDir);
#ifdef _WIN32
    system(("if not exist " + assetsDir + " mkdir " + assetsDir + " & if not exist " + assetsDir + "\\fonts mkdir " + assetsDir + "\\fonts").c_str());
#else
    system(("mkdir -p " + assetsDir + "/fonts").c_str());
#endif
    generatePlaceholderAssets(assets);
    generatePlaceholderAudio(assets);
    std::vector<std::string> fontPaths = {
        assetsDir + "/fonts/JetBrainsMono-Regular.ttf",
        "assets/fonts/JetBrainsMono-Regular.ttf",
        "build/assets/fonts/JetBrainsMono-Regular.ttf",
        "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "assets/font.ttf"
    };
    bool fontLoaded = false;
    for (const auto& path : fontPaths) {
        if (assets.loadFont("main", path)) { fontLoaded = true; break; }
    }
    if (!fontLoaded) std::cerr << "Warning: No UI font found (expected assets/fonts/JetBrainsMono-Regular.ttf)." << std::endl;
    assets.loadFont("bold", assetsDir + "/fonts/JetBrainsMono-Bold.ttf");
    if (!assets.hasTexture("blue_square"))
        std::cerr << "Warning: piece textures missing. Run from build/ or ensure assets/ exists." << std::endl;
    Renderer renderer(window, assets, userManager);
    sf::Sound sound, music;
    if (assets.hasSoundBuffer("music")) { music.setBuffer(assets.getSoundBuffer("music")); music.setLoop(true); music.setVolume(50.0f); music.play(); }
    GameState state = GameState::LOGIN; Game *currentGame = nullptr; Piece *nextPiece = nullptr;
    std::string tempPseudo = "";
    bool maximized = true;
    bool sessionActive = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                finalizeActiveSession(userManager, renderer, currentGame, sessionActive);
                window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11) {
                maximized = !maximized;
                if (maximized)
                    window.create(screenSize, "Tetriste", windowStyle);
                else
                    window.create(sf::VideoMode(1280, 720), "Tetriste", windowStyle);
                window.setFramerateLimit(60);
                window.setVerticalSyncEnabled(true);
            }
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
                        if (selection == 0) state = GameState::DIFFICULTY_SELECT;
                        else if (selection == 1) state = GameState::PROTOCOLS;
                        else if (selection == 2) state = GameState::ACHIEVEMENTS;
                        else if (selection == 3) state = GameState::SETTINGS;
                        else if (selection == 4) window.close();
                    }
                } else if (state == GameState::PROTOCOLS) {
                    if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::A)
                        state = GameState::MENU;
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
                        renderer.resetMatchHighlight();
                        userManager.beginSession();
                        sessionActive = true;
                        state = GameState::PLAYING;
                        {
                            auto sz = window.getSize();
                            renderer.addPopup("BUFFER: 8 NODES LOADED",
                                sf::Vector2f(sz.x * 0.5f, sz.y * 0.14f), UI::CyberPrimaryFixed);
                            renderer.addPopup("J/A: LEFT  K/D: RIGHT  C/S: SHIFT",
                                sf::Vector2f(sz.x * 0.5f, sz.y * 0.2f), UI::CyberPrimaryFixed);
                        }
                    } else if (event.key.code == sf::Keyboard::Escape) state = GameState::MENU;
                } else if (state == GameState::GAME_OVER) { 
                    if (event.key.code == sf::Keyboard::Space) state = GameState::MENU; 
                } else if (state == GameState::PAUSED) { if (event.key.code == sf::Keyboard::Escape) state = GameState::PLAYING; }
                else if (state == GameState::PLAYING) handlePlayingInput(event, window, state, currentGame, nextPiece, renderer, assets, userManager, sound, music, sessionActive);
            }
        }
        if (state == GameState::PLAYING && currentGame) {
            int capacity = computeBoardCapacity(window.getSize(), renderer.getDifficulty());
            if (currentGame->piecesCount > capacity) {
                state = GameState::GAME_OVER;
                finalizeActiveSession(userManager, renderer, currentGame, sessionActive);
            }
        }
        if (currentGame && (state == GameState::PLAYING || state == GameState::PAUSED || state == GameState::GAME_OVER))
            renderer.render(*currentGame, nextPiece, state);
        else { static Game dummyGame(0,0); renderer.render(dummyGame, nullptr, state); }
    }
    nextPiece = nullptr;
    if (currentGame) delete currentGame;
    currentGame = nullptr; 
    return 0;
}
