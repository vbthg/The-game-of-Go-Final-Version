#pragma once
#include <map>
#include <string>
#include <memory>
#include <list>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Cursor.hpp>

class ResourceManager
{
public:
    static ResourceManager& getInstance();

    ResourceManager(ResourceManager const&) = delete;
    void operator=(ResourceManager const&) = delete;

    void loadTexture(const std::string& key, const std::string& filePath);

    sf::Texture& getTexture(const std::string& key);

    void loadFont(const std::string& key, const std::string& filePath);

    sf::Font& getFont(const std::string& key);

    void loadSoundBuffer(const std::string& key, const std::string& filePath);

    sf::SoundBuffer& getSoundBuffer(const std::string& key);

    void loadCursor(const std::string& key, sf::Cursor::Type type);
    sf::Cursor& getCursor(const std::string& key);

    void playMusic(int themeIndex);
    void setMusicVolume(float volume);
    void stopMusic();


    void playAmbient(const std::string &filePath);
    void stopAmbient();
    void setAmbientVolume(float volume);

    void playSound(const std::string& key, float volume = 100.f);

private:
    ResourceManager();

    std::map<std::string, sf::Texture> m_textures;
    std::map<std::string, sf::Font> m_fonts;
    std::map<std::string, sf::SoundBuffer> m_soundBuffers;
    std::map<std::string, std::unique_ptr<sf::Cursor>> m_cursors;

    sf::Music m_backgroundMusic;
    int m_currentMusicTheme = -1;

    sf::Music m_ambientMusic;

    std::list<sf::Sound> m_activeSounds;
};

