#pragma once

#include "GlobalSetting.h"
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

GlobalSetting& GlobalSetting::getInstance()
{
    static GlobalSetting instance;
    return instance;
}

GlobalSetting::GlobalSetting()
    : musicVolume(50.f)
    , sfxVolume(100.f)
    , stoneThemeIndex(0)
    , boardThemeIndex(0)
    , musicThemeIndex(0)
    , timeLimitIndex(1)
    , komiIndex(1)
{
}

int GlobalSetting::getTimeLimitInSeconds() const
{
    int minutes[] = { 5, 10, 20, 30, 45, 60, -1 };
    if(timeLimitIndex >= 0 && timeLimitIndex < 7)
    {
        if(minutes[timeLimitIndex] == -1) return -1;
        return minutes[timeLimitIndex] * 60;
    }
    return 600;
}

float GlobalSetting::getKomiValue() const
{
    float komis[] = { 0.5f, 6.5f, 7.5f };
    if(komiIndex >= 0 && komiIndex < 3)
    {
        return komis[komiIndex];
    }
    return 6.5f;
}

std::string GlobalSetting::getBoardTextureKey(int size) const
{
    std::string base = "gameplay_board";

    base += "_" + std::to_string(size) + "x" + std::to_string(size);

    if(boardThemeIndex == 0) base += "_basic";
    if(boardThemeIndex == 1) base += "_dark";
    if(boardThemeIndex == 2) base += "_light";

    return base;
}

std::string GlobalSetting::getStoneTextureKey(bool isBlack, int size) const
{
    std::string themeName = "basic";
    if(stoneThemeIndex == 1) themeName = "cartoon";
    if(stoneThemeIndex == 2) themeName = "realistic";

    std::string color = isBlack ? "black" : "white";
    std::string base = "gameplay_stone_" + color;
    base = base + "_" + std::to_string(size) + "x" + std::to_string(size);

    base += "_" + themeName;

    return base;
}

std::string GlobalSetting::getSoundKey(const std::string& actionType, bool isBlack) const
{
    std::string themeName = "basic";
    if(stoneThemeIndex == 1) themeName = "cartoon";

    std::string colorSuffix = isBlack ? "black" : "white";

    return actionType + "_" + colorSuffix + "_" + themeName;
}

void GlobalSetting::saveToFile(const std::string& filename)
{
    std::ofstream file(filename);
    if(file.is_open())
    {
        file << musicVolume << "\n";
        file << sfxVolume << "\n";
        file << stoneThemeIndex << "\n";
        file << boardThemeIndex << "\n";
        file << musicThemeIndex << "\n";
        file << timeLimitIndex << "\n";
        file << komiIndex << "\n";

//        std::cout << "[GlobalSetting] Settings saved to " << filename << std::endl;
        file.close();
    }
    else
    {
        std::cerr << "[GlobalSetting] Unable to save settings!" << std::endl;
    }
}

void GlobalSetting::loadFromFile(const std::string& filename)
{
    std::ifstream file(filename);
    if(file.is_open())
    {
        file >> musicVolume;
        file >> sfxVolume;
        file >> stoneThemeIndex;
        file >> boardThemeIndex;
        file >> musicThemeIndex;
        file >> timeLimitIndex;
        file >> komiIndex;

        std::cout << "[GlobalSetting] Settings loaded from " << filename << std::endl;
        file.close();
    }
    else
    {
        std::cout << "[GlobalSetting] No save file found. Using defaults." << std::endl;
    }
}
