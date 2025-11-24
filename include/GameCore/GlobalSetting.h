#pragma once
#include <string>
#include <vector>

class GlobalSetting {
public:
    static GlobalSetting& getInstance();

    float musicVolume; // 0.0 - 100.0
    float sfxVolume;   // 0.0 - 100.0

    int stoneThemeIndex;
    int boardThemeIndex;
    int musicThemeIndex;

    int timeLimitIndex;
    int komiIndex;


    int getTimeLimitInSeconds() const;
    float getKomiValue() const;
    std::string getBoardTextureKey(int size) const;
    std::string getStoneTextureKey(bool isBlack, int size) const;

    void saveToFile(const std::string& filename = "assets/setting.txt");
    void loadFromFile(const std::string& filename = "assets/setting.txt");

private:
    GlobalSetting();

    GlobalSetting(const GlobalSetting&) = delete;
    void operator=(const GlobalSetting&) = delete;
};
