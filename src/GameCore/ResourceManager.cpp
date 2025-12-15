#include "ResourceManager.h"
#include <iostream>
#include <cassert>

ResourceManager& ResourceManager::getInstance()
{
    static ResourceManager instance;
    return instance;
}

void ResourceManager::loadTexture(const std::string& key, const std::string& filePath)
{
    sf::Texture texture;
    if(!texture.loadFromFile(filePath))
    {
        std::cerr << "texture " << key << " cannot load!\n";
        assert(false);
    }

    texture.setSmooth(true);
    texture.generateMipmap();

    m_textures[key] = texture;
}

sf::Texture& ResourceManager::getTexture(const std::string& key)
{
    auto it = m_textures.find(key);
    if(it == m_textures.end())
    {
        std::cerr << "texture " << key << " not found!\n";
        assert(false);
    }

    return it -> second;
}

void ResourceManager::loadFont(const std::string& key, const std::string& filePath)
{
    sf::Font font;
    if(!font.loadFromFile(filePath))
    {
        std::cerr << "font " << key << " cannot load!\n";
        assert(false);
    }

    m_fonts[key] = font;
}

sf::Font& ResourceManager::getFont(const std::string& key)
{
    auto it = m_fonts.find(key);
    if(it == m_fonts.end())
    {
        std::cerr << "font " << key << " not found!\n";
        assert(false);
    }
    return it -> second;
}

void ResourceManager::loadSoundBuffer(const std::string& key, const std::string& filePath)
{
    sf::SoundBuffer buffer;
    if(!buffer.loadFromFile(filePath))
    {
        std::cerr << "buffer " << key << " cannot load!\n";
        assert(false);
    }

    m_soundBuffers[key] = buffer;
}

sf::SoundBuffer& ResourceManager::getSoundBuffer(const std::string& key)
{
    auto it = m_soundBuffers.find(key);
    if(it == m_soundBuffers.end())
    {
        std::cerr << "buffer " << key << " not found!\n";
        assert(false);
    }

    return it -> second;
}

void ResourceManager::loadCursor(const std::string& key, sf::Cursor::Type type)
{
    auto cursor = std::make_unique<sf::Cursor>();

    if(cursor->loadFromSystem(type))
    {
        m_cursors[key] = std::move(cursor);
    }
    else
    {
        std::cerr << "Failed to load system cursor type: " << type << "\n";
    }
}

sf::Cursor& ResourceManager::getCursor(const std::string& key)
{
    auto it = m_cursors.find(key);
    if(it == m_cursors.end())
    {
        std::cerr << "Cursor key not found: " << key << "!\n";
        assert(false);
    }
    return *(it->second);
}

void ResourceManager::playMusic(int themeIndex)
{
//    std::cout << themeIndex << " " << m_currentMusicTheme << "\n";

    if(m_currentMusicTheme == themeIndex && m_backgroundMusic.getStatus() == sf::SoundSource::Playing)
    {
        return;
    }

    std::string filename;
    if(themeIndex == 0) filename = "resources/sounds/musicTheme/Chinese Guqin classic music.ogg";
    else if(themeIndex == 1) filename = "resources/sounds/musicTheme/Midnight Vale - Peaceful Fantasy.ogg";
    else filename = "resources/sounds/musicTheme/Classical Music.ogg";

    std::cout << themeIndex << "\n";

    if(m_backgroundMusic.openFromFile(filename))
    {
        m_backgroundMusic.setLoop(true);
        m_backgroundMusic.play();
        m_currentMusicTheme = themeIndex;
    }
    else
    {
        std::cout << "[ERROR] Cannot load music: " << filename << std::endl;
    }
}

void ResourceManager::setMusicVolume(float volume)
{
    m_backgroundMusic.setVolume(volume);
}

void ResourceManager::stopMusic()
{
    m_backgroundMusic.stop();
}

void ResourceManager::playAmbient(const std::string& filePath)
{
    if(m_ambientMusic.getStatus() == sf::SoundSource::Playing)
    {
         return;
    }

    if(m_ambientMusic.openFromFile(filePath))
    {
        m_ambientMusic.setLoop(true);
        m_ambientMusic.play();
//        std::cout << "[ResourceManager] Playing ambient: " << filePath << "\n";
    }
    else
    {
        std::cerr << "[ResourceManager] Error loading ambient: " << filePath << "\n";
    }
}

void ResourceManager::stopAmbient()
{
    m_ambientMusic.stop();
}

void ResourceManager::setAmbientVolume(float volume)
{
    m_ambientMusic.setVolume(volume);
}

void ResourceManager::playSound(const std::string& key, float volume)
{
    m_activeSounds.remove_if([](const sf::Sound& s)
    {
        return s.getStatus() == sf::Sound::Stopped;
    });

    m_activeSounds.emplace_back();
    sf::Sound& s = m_activeSounds.back();

    s.setBuffer(getSoundBuffer(key));
    s.setVolume(volume);
    s.play();
}

ResourceManager::ResourceManager() { };
