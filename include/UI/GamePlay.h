#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>
#include <vector>
#include <stack>

#include "GameState.h"
#include "GameLogic.h"
#include "ResourceManager.h"
#include "Button.h"
#include "HistoryList.h"
#include "Timeline.h"
#include "BoardBreathEffect.h"
#include "SaveDefinition.h"
#include "ScoringOverlay.h"


struct UIActionSnapshot
{
    bool isBlack;
    std::string notation; // Ví dụ: "K10"
    float thinkingTime;   // Ví dụ: 1.5s
};

class GamePlay : public GameState {
public:

    GamePlay(sf::RenderWindow& window, int boardSize, GameMode mode, AiDifficulty difficulty);
    virtual ~GamePlay() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;


    void performSaveGame(int slotIndex);
    void performLoadGame(const std::string& filePath);

private:
    // xu ly input
    void onBoardClick(int mouseX, int mouseY);
    void onPauseClick();
    void onPassClick();
    void onUndoClick(); // Nút Undo
    void onRedoClick(); // Nút Redo


    void handleAiTurn();


    void drawStones();
    sf::Vector2i getBoardCoordsFromMouse(int mouseX, int mouseY);
    std::string convertCoordsToNotation(int x, int y);


    void saveThumbnail(const std::string& filename);
    std::string getCurrentTimestamp();


    void finalizeScore();


    sf::RenderWindow& m_window;
    GameStateType m_requestedState;

    GameLogic m_logic;
    int m_boardSize;
    GameMode m_mode;
    AiDifficulty m_difficulty;


    bool m_aiShouldMove;
    float m_aiThinkTimer;
    bool m_gameHasEnded;
    sf::Clock m_moveTimer;


    sf::Sprite m_background;
    sf::Sprite m_boardSprite;
    sf::Sprite m_turnactionBackground;
    sf::Texture* m_blackStoneTex;
    sf::Texture* m_whiteStoneTex;
    sf::Font& m_font;
    sf::Text m_messageText;
    sf::Text m_turnText;
    sf::Text m_gameOverText;


    UI::Button m_pauseButton;
    UI::Button m_passButton;
    UI::Button m_undoBtn;
    UI::Button m_redoBtn;


    std::unique_ptr<UI::HistoryList> m_historyList;
    std::unique_ptr<UI::Timeline> m_timeline;
    std::unique_ptr<UI::BoardBreathEffect> m_breathEffect;


    std::stack<UIActionSnapshot> m_uiRedoStack;


    std::vector<sf::Sound> m_randomPlaceSounds;

    sf::Sound m_soundPlace;
    sf::Sound m_soundCapture;
    sf::Sound m_soundPass;
    sf::Sound m_soundError;


    float m_boardTopLeftX;
    float m_boardTopLeftY;
    float m_cellSpacing;

    float m_timeLimitBlack; // Thời gian còn lại của Đen (giây)
    float m_timeLimitWhite; // Thời gian còn lại của Trắng (giây)
    bool m_isTimeLimitEnabled; // Có bật giới hạn không

    // hiển thị giờ

    sf::Sprite m_timerPanel;

    sf::Sprite m_timerIconBlack;
    sf::Sprite m_timerIconWhite;
    sf::Text m_timerTextBlack;
    sf::Text m_timerTextWhite;

    std::string formatTime(float seconds);

    sf::RectangleShape m_winPanel;
    sf::Text m_winText;

    std::unique_ptr<ScoringOverlay> m_scoringOverlay;
    bool m_isScoringMode;
};
