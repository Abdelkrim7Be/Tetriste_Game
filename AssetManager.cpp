#include "AssetManager.h"
#include <iostream>

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
