#pragma once
#include <string>

struct SaveInfo {
    int slotIndex;
    std::string filename;
    std::string screenshotPath;


    std::string userTitle;
    std::string timestamp;
    int boardSize;
    std::string modeStr;
    std::string status;

    int difficulty = 1;

    std::string endReason = "";
};
