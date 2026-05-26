#include "AssetManager.h"

void AssetManager::loadTexture(std::string name, std::string filename) {
    sf::Texture tex;
    if (tex.loadFromFile(filename)) {
        this->textures[name] = tex;
    }
}

sf::Texture &AssetManager::getTexture(std::string name) {
    return this->textures.at(name);
}

void AssetManager::loadFont(std::string name, std::string filename) {
    sf::Font font;
    if (font.loadFromFile(filename)) {
        this->fonts[name] = font;
    }
}

sf::Font &AssetManager::getFont(std::string name) {
    return this->fonts.at(name);
}
