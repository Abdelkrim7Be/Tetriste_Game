#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
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

    bool loadSoundBuffer(std::string name, std::string filename);
    void addSoundBuffer(std::string name, const sf::SoundBuffer& buffer);
    sf::SoundBuffer& getSoundBuffer(std::string name);
    bool hasSoundBuffer(std::string name) const;

    void setVolume(float volume) { this->volume = volume; }
    float getVolume() const { return volume; }

    private:
    std::map<std::string, sf::Texture> textures;
    std::map<std::string, sf::Font> fonts;
    std::map<std::string, sf::SoundBuffer> soundBuffers;
    float volume = 50.0f;
    };
#endif
