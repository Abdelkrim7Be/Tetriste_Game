#include "AssetManager.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

AssetManager::AssetManager() {
    loadConfig();
}

void AssetManager::loadConfig() {
    std::ifstream file("assets/config.json");
    if (!file.is_open()) return;
    try {
        json j;
        file >> j;
        if (j.contains("volume")) {
            this->volume = j["volume"];
        }
    } catch (...) {}
    file.close();
}

void AssetManager::saveConfig() {
    json j;
    j["volume"] = this->volume;
    std::ofstream file("assets/config.json");
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    }
}

bool AssetManager::loadTexture(std::string name, std::string filename) {
    sf::Texture tex;
    if (tex.loadFromFile(filename)) {
        this->textures[name] = tex;
        return true;
    }
    std::cerr << "Error: Failed to load texture '" << filename << "'" << std::endl;
    return false;
}

sf::Texture &AssetManager::getTexture(std::string name) {
    return this->textures.at(name);
}

bool AssetManager::hasTexture(std::string name) const {
    return textures.find(name) != textures.end();
}

bool AssetManager::loadFont(std::string name, std::string filename) {
    sf::Font font;
    if (font.loadFromFile(filename)) {
        this->fonts[name] = font;
        return true;
    }
    std::cerr << "Error: Failed to load font '" << filename << "'" << std::endl;
    return false;
}

sf::Font &AssetManager::getFont(std::string name) {
    return this->fonts.at(name);
}

bool AssetManager::hasFont(std::string name) const {
    return fonts.find(name) != fonts.end();
}

bool AssetManager::loadSoundBuffer(std::string name, std::string filename) {
    sf::SoundBuffer buffer;
    if (buffer.loadFromFile(filename)) {
        this->soundBuffers[name] = buffer;
        return true;
    }
    std::cerr << "Error: Failed to load sound buffer '" << filename << "'" << std::endl;
    return false;
}

void AssetManager::addSoundBuffer(std::string name, const sf::SoundBuffer& buffer) {
    this->soundBuffers[name] = buffer;
}

sf::SoundBuffer &AssetManager::getSoundBuffer(std::string name) {
    return this->soundBuffers.at(name);
}

bool AssetManager::hasSoundBuffer(std::string name) const {
    return soundBuffers.find(name) != soundBuffers.end();
}
