#pragma once
#include <string>
#include <vector>
#include <functional>

class GlobalSetting
{
public:
    static GlobalSetting& getInstance();

    float musicVolume;
    float sfxVolume;

    int stoneThemeIndex;
    int boardThemeIndex;
    int musicThemeIndex;

    int timeLimitIndex;
    int komiIndex;

    int getTimeLimitInSeconds() const;
    float getKomiValue() const;
    std::string getBoardTextureKey(int size) const;
    std::string getStoneTextureKey(bool isBlack, int size) const;
    std::string getSoundKey(const std::string& actionType, bool isBlack) const;

    void setOnThemeChanged(std::function<void()> callback)
    {
        m_themeChangedCallback = callback;
    }

    void notifyThemeChanged()
    {
        if (m_themeChangedCallback) m_themeChangedCallback();
    }

    void saveToFile(const std::string& filename = "assets/setting.txt");
    void loadFromFile(const std::string& filename = "assets/setting.txt");

private:
    GlobalSetting();

    GlobalSetting(const GlobalSetting&) = delete;
    void operator=(const GlobalSetting&) = delete;


    std::function<void()> m_themeChangedCallback;
};
