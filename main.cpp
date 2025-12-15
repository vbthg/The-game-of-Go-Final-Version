#include "Game.h"
#include "ResourceManager.h"
#include "GlobalSetting.h"
#include <ctime>
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

void loadAllResources()
{
    auto& rm = ResourceManager::getInstance();

    std::cout << "Loading resources..." << std::endl;



    // --- 1. FONTS ---
    // (cần tải file .ttf về và đặt vào thư mục resources/fonts/)

    rm.loadFont("main_font", "resources/fonts/Montserrat-Light.ttf");
    rm.loadFont("title_font", "resources/fonts/Cinzel-regular.ttf");

    // artegra sans
    // cinzel
    // Montserrat






/**
 * GAMECORE RESOURCES LOADING
**/



/**
 * USER INTERFACE RESOURCES LOADING
**/



    // --- ABOUT RESOURCES ---
    rm.loadTexture("about_back_btn",     "resources/images/GoBackBtn_55x55_curve.png"); // Nút Back
    rm.loadTexture("about_slider_track", "resources/images/About_slidetrack.png"); // Thanh trượt (About)




    // --- GAMEPLAY RESOURCES ---
    rm.loadTexture("gameplay_background", "resources/images/GamePlayBackground_01.png");   // Nền màn hình chơi
    rm.loadTexture("gameplay_board_9x9",  "resources/images/Board_9x9_740x740_bevel.png");   // Ảnh bàn cờ
    rm.loadTexture("gameplay_board_13x13",  "resources/images/board_13x13_740x740_bevel.png");   // Ảnh bàn cờ
    rm.loadTexture("gameplay_board_19x19",  "resources/images/board_19x19_740x740_bevel.png");   // Ảnh bàn cờ
    rm.loadTexture("gameplay_stone_black_9x9","resources/images/blackStone_50x50.png");   // Quân cờ Đen
    rm.loadTexture("gameplay_stone_white_9x9","resources/images/whiteStone_50x50.png");   // Quân cờ Trắng
    rm.loadTexture("gameplay_stone_black_13x13","resources/images/blackStone_38x38.png");   // Quân cờ Đen
    rm.loadTexture("gameplay_stone_white_13x13","resources/images/whiteStone_38x38.png");   // Quân cờ Trắng
    rm.loadTexture("gameplay_stone_black_19x19","resources/images/blackStone_30x30.png");   // Quân cờ Đen
    rm.loadTexture("gameplay_stone_white_19x19","resources/images/whiteStone_30x30.png");   // Quân cờ Trắng

    rm.loadTexture("gameplay_pause_btn",  "resources/images/PauseBtn_55x55.png");
    rm.loadTexture("gameplay_pass_btn",   "resources/images/PassBtn_80x55.png");
    rm.loadTexture("gameplay_undo_btn",   "resources/images/UndoBtn_80x55.png");
    rm.loadTexture("gameplay_redo_btn",   "resources/images/RedoBtn_80x55.png");
    rm.loadTexture("gameplay_turnaction_background", "resources/images/TurnAction_BackGround_360x96.png");   // Background cho nút Pass, Undo, Redo

    rm.loadTexture("gameplay_historylist_background",      "resources/images/HistoryList_background.png");    // Nền History List
    rm.loadTexture("gameplay_historylist_track",  "resources/images/historyboard_slidertrack.png");    // Thanh trượt của HistoryList
    rm.loadTexture("gameplay_historylist_icon_black","resources/images/blackStone_38x38.png");  // Icon quân đen (cho list)
    rm.loadTexture("gameplay_historylist_icon_white","resources/images/whiteStone_38x38.png");  // Icon quân trắng (cho list)
    rm.loadTexture("gameplay_timeline_background", "resources/images/timeline_background_1210x18.png");  // Nền Timeline
    rm.loadTexture("gameplay_timeline_tooltip_bg","resources/images/TimeLine_tooltip_90x45_gray.png"); // Nền Tooltip

    rm.loadTexture("gameplay_timer_bg", "resources/images/timer_board_bg_190x100.png");



    // --- MAINMENU RESOURCES ---
    rm.loadTexture("mainmenu_background", "resources/images/MainMenuBackground.png");
    rm.loadTexture("mainmenu_newgame_btn", "resources/images/NewGameBtn_190x190.png");
    rm.loadTexture("mainmenu_savedgame_btn", "resources/images/SavedGameBtn_190x190.png");
    rm.loadTexture("mainmenu_setting_btn", "resources/images/SettingBtn_190x190.png");
    rm.loadTexture("mainmenu_about_btn", "resources/images/AboutBtn_80x80.png");
    rm.loadTexture("mainmenu_quit_btn", "resources/images/ExitBtn_65x65.png");




    // --- NEWGAME RESOURCES ---
    rm.loadTexture("newgame_background", "resources/images/NewGameBackground.png");
    rm.loadTexture("newgame_pvp_btn", "resources/images/pvpBtn_190x190.png");
    rm.loadTexture("newgame_pvbot_btn", "resources/images/pvbotBtn_190x190.png");
    rm.loadTexture("newgame_back_btn", "resources/images/GoBackBtn_55x55_curve.png");




    // --- PAUSE RESOURCES ---
    rm.loadTexture("pause_background", "resources/images/pause_background.png");
    rm.loadTexture("pause_resume_btn", "resources/images/ResumeBtn_120x83.png");
    rm.loadTexture("pause_setting_btn", "resources/images/SettingBtn_120x83.png");
    rm.loadTexture("pause_savegame_btn", "resources/images/SaveGameBtn_120x83.png");
    rm.loadTexture("pause_reset_btn", "resources/images/resetBtn_120x83.png");
    rm.loadTexture("pause_returnmenu_btn", "resources/images/MenuBtn_120x83.png");




    // --- SAVED GAME RESOURCES ---
    rm.loadTexture("savedgame_background", "resources/images/SavedGameBackground.png");
    rm.loadTexture("savedgame_back_btn", "resources/images/GoBackBtn_55x55_curve.png");
//    /** **/ rm.loadTexture("savedgame_popup_background", "resources/images/");
    rm.loadTexture("savedgame_popupyes_btn", "resources/images/savedgame_popupyes_117x80.png");
    rm.loadTexture("savedgame_popupno_btn", "resources/images/savedgame_popupno_117x80.png");
    rm.loadTexture("savedgame_slidertrack_btn", "resources/images/SavedGame_slidertrack.png");
    rm.loadTexture("savedgame_delete_btn", "resources/images/savedgame_deleteBtn_50x50.png");
    rm.loadTexture("savedgame_load_btn", "resources/images/savedgame_loadBtn_50x50.png");




    // --- SETTING RESOURCES ---
    rm.loadTexture("setting_background", "resources/images/settingBackground.png");
    rm.loadTexture("setting_apply_btn", "resources/images/setting_applyBtn_58x40.png");
    rm.loadTexture("setting_back_btn", "resources/images/setting_backBtn_58x40.png");
    rm.loadTexture("setting_slidertrack_btn", "resources/images/setting_slidertrack_250x10.png");




    // --- SIZESELECTION RESOURCES ---
    rm.loadTexture("sizeselection_background", "resources/images/SizeSelectionBackground.png");
    rm.loadTexture("sizeselection_9x9_btn", "resources/images/SizeBtn_9x9_190x190.png");
    rm.loadTexture("sizeselection_13x13_btn", "resources/images/SizeBtn_13x13_190x190.png");
    rm.loadTexture("sizeselection_19x19_btn", "resources/images/SizeBtn_19x19_190x190.png");
    rm.loadTexture("sizeselection_novice_btn", "resources/images/NoviceBtn_190x190.png");
    rm.loadTexture("sizeselection_adept_btn", "resources/images/AdeptBtn_190x190.png");
    rm.loadTexture("sizeselection_master_btn", "resources/images/MasterBtn_190x190.png");
    rm.loadTexture("sizeselection_apply_btn", "resources/images/sizeselection_startBtn_120x120.png");
    rm.loadTexture("sizeselection_back_btn", "resources/images/GoBackBtn_55x55_curve.png");
    rm.loadTexture("sizeselection_text_background", "resources/images/sizeselection_text_background.png");



    // --- STEPPER RESOURCES ---
    rm.loadTexture("stepper_minus_btn", "resources/images/MinusBtn_40x40.png");
    rm.loadTexture("stepper_plus_btn", "resources/images/PlusBtn_40x40.png");




    // --- TIMELINE RESOURCES ---
//    rm.loadTexture("timeline_background", "resources/images/");
//    rm.loadTexture("timeline_tooltip_background", "resources/images/");




    // --- 5. SOUNDS (Âm thanh) ---
    // (Dùng file .wav cho hiệu ứng ngắn)
    rm.loadSoundBuffer("place_stone_01",   "resources/sounds/place_stone_01.wav");
    rm.loadSoundBuffer("capture_stone", "resources/sounds/capture_sound.wav");
    rm.loadSoundBuffer("pass_move",     "resources/sounds/pass_sound.wav");
    rm.loadSoundBuffer("error_move",    "resources/sounds/error_move_sound.wav");














// --- LOAD CURSORS ---
    rm.loadCursor("cursor_arrow", sf::Cursor::Arrow); // Con trỏ thường
    rm.loadCursor("cursor_hand",  sf::Cursor::Hand);  // Con trỏ bàn tay (khi hover)
    rm.loadCursor("cursor_wait",  sf::Cursor::Wait);  // Đồng hồ cát

    std::cout << "Resources loaded successfully!" << std::endl;
}

int main()
{
    SetProcessDPIAware();

    std::srand(static_cast<unsigned>(std::time(nullptr)));

    GlobalSetting::getInstance().loadFromFile();

    auto& rm = ResourceManager::getInstance();
    auto& gs = GlobalSetting::getInstance();

    rm.setMusicVolume(gs.musicVolume);

    rm.playMusic(gs.musicThemeIndex);

    loadAllResources();

    Game game(1600, 900, "Go Game - First version");

    game.run();

    return 0;
}
