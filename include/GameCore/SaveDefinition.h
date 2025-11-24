#pragma once
#include <string>

struct SaveInfo {
    int slotIndex;              // ID slot
    std::string filename;       // Đường dẫn file .txt
    std::string screenshotPath; // Đường dẫn file .png

    // Metadata hiển thị
    std::string userTitle;      // Tên (vd: Game 1)
    std::string timestamp;      // Thời gian lưu
    int boardSize;              // 19
    std::string modeStr;        // PvP / PvE
    std::string status;         // Ongoing / Finished
};
