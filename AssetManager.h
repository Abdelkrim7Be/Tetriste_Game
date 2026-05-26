#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class AssetManager {
public:
    AssetManager() {}
    
    void loadTexture(std::string name, std::string filename);
    sf::Texture &getTexture(std::string name);
    
    void loadFont(std::string name, std::string filename);
    sf::Font &getFont(std::string name);

private:
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Font> fonts;
};

#endif
