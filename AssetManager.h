#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class AssetManager {
public:
    AssetManager() {}
    
    bool loadTexture(std::string name, std::string filename);
    sf::Texture &getTexture(std::string name);
    bool hasTexture(std::string name) const;
    
    bool loadFont(std::string name, std::string filename);
    sf::Font &getFont(std::string name);
    bool hasFont(std::string name) const;

private:
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Font> fonts;
};

#endif
