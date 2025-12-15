// include/GamePlay.h
#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System/Clock.hpp>
#include <memory>
#include <vector>
#include <stack> // <-- Cần cho Undo/Redo
#include <future>
#include <mutex>

#include "GameState.h"
#include "GameLogic.h"
#include "ResourceManager.h"
#include "Button.h"
#include "HistoryList.h"
#include "Timeline.h"
#include "BoardBreathEffect.h"
#include "SaveDefinition.h" // <-- Cần cho Save/Load info
#include "ScoringOverlay.h"
#include "IBot.h"
#include "BotManager.h"

// Struct để lưu dữ liệu UI phục vụ Redo
struct UIActionSnapshot {
    bool isBlack;
    std::string notation; // Ví dụ: "K10"
    float thinkingTime;   // Ví dụ: 1.5s
};

struct PendingMove {
    std::string color;
    int x;
    int y;
    bool active = false;
};

enum class MsgType { Info, Error, Success };


class GamePlay : public GameState {
public:
    /**
     * @brief Hàm khởi tạo nhận đầy đủ thông tin cài đặt.
     */
    GamePlay(sf::RenderWindow& window, int boardSize, GameMode mode, AiDifficulty difficulty);
    virtual ~GamePlay() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;

    // --- Các hàm Public để Game.cpp gọi ---
    void performSaveGame(int slotIndex);
    void performLoadGame(const std::string& filePath);
    void startGameInitialization();

private:

    // [THÊM] Sprite Loading
    sf::Sprite m_loadingSprite;
    // [THÊM] Cờ theo dõi trạng thái Hint đang chạy
    std::atomic<bool> m_isCalculatingHint{false};

    const float MESSAGE_DURATION = 2.0f;
    float m_messageTimer = 0;

    std::mutex m_workerMutex; // [THÊM] Khóa để bảo vệ Bot Pachi

    // --- Xử lý Input (Callbacks) ---
    void onBoardClick(int mouseX, int mouseY);
    void onPauseClick();
    void onPassClick(bool isBotAction = false);
    void onUndoClick(); // Nút Undo
    void onRedoClick(); // Nút Redo
    void onResignClick();
    void onHintClick();

    void showMessage(const std::string& msg, MsgType type = MsgType::Info);

    void requestHintFromBot();

    // Hàm đồng bộ nước đi cho tất cả bot (tránh gửi trùng nếu 2 bot là 1)
    void syncToAllBots(const std::string& color, int x, int y);

    // [MỚI] Hàm đồng bộ lại toàn bộ bàn cờ (dùng cho Undo/Redo/Load)
    void fullResyncBots();

    // Hàm cập nhật marker dựa trên lịch sử (dùng cho Undo/Redo)
    void updateLastMoveMarkerFromHistory();

    // Hàm parse string "D4" thành tọa độ (3, 3)
    sf::Vector2i parseNotationToCoords(const std::string& notation);

    // --- Logic & AI ---
    void handleAiTurn();

    // --- Helper ---
    void drawStones();
    sf::Vector2i getBoardCoordsFromMouse(int mouseX, int mouseY);
    std::string convertCoordsToNotation(int x, int y);
    void placeMessageText(sf::Text& m_messageText);

    // --- Helper Save/Load ---
    void saveThumbnail(const std::string& filename);
    std::string getCurrentTimestamp();

    void updateThemeResources();


    void finalizeScore(); // <--- THÊM HÀM NÀY

    // --- Thành viên chính ---
    sf::RenderWindow& m_window;
    GameStateType m_requestedState;

    GameLogic m_logic;
    int m_boardSize;
    GameMode m_mode;
    AiDifficulty m_difficulty;

    // --- Trạng thái Game ---
    bool m_aiShouldMove;
    float m_aiThinkTimer;
    bool m_gameHasEnded;
    sf::Clock m_moveTimer;

    // --- UI Components (Sprite & Text) ---
    sf::Sprite m_background;
    sf::Sprite m_boardSprite;
    sf::Sprite m_turnactionBackground;
    sf::Sprite m_logBackground;
    sf::Texture* m_blackStoneTex;
    sf::Texture* m_whiteStoneTex;
    sf::Font& m_font;
    sf::Text m_messageText;
    sf::Text m_turnText;
    sf::Text m_gameOverText;

    // --- UI Components (Buttons) ---
    UI::Button m_pauseButton;
    UI::Button m_passButton;
    UI::Button m_undoBtn;
    UI::Button m_redoBtn;
    UI::Button m_resignButton;
    // --- HINT SYSTEM ---
    UI::Button m_hintButton;
    bool m_blackUsedHint;
    bool m_whiteUsedHint;

    // Tọa độ gợi ý. Quy ước:
    // (-5, -5): None
    // (-1, -1): Pass
    // (-2, -2): Resign
    // >= 0: Tọa độ thực
    sf::Vector2i m_hintCoord;
    // Dùng Sprite thay vì CircleShape để vẽ quân cờ thật
    sf::Sprite m_hintStoneSprite;

    // --- UI Components (Complex) ---
    std::unique_ptr<UI::HistoryList> m_historyList;
    std::unique_ptr<UI::Timeline> m_timeline;
    std::unique_ptr<UI::BoardBreathEffect> m_breathEffect;

    // --- Undo/Redo Data ---
    std::stack<UIActionSnapshot> m_uiRedoStack;

    // --- Âm thanh ---
//    std::vector<sf::Sound> m_randomPlaceSounds;


    // Ma trận lưu scale hiện tại của từng ô (để làm hiệu ứng thu nhỏ)
    std::vector<std::vector<float>> m_stoneScaleMatrix;

    // Tọa độ nước đi cuối cùng (-1, -1 nếu chưa đi hoặc vừa Undo)
    sf::Vector2i m_lastMoveCoord;

    // Hình vuông đỏ đánh dấu
    sf::RectangleShape m_lastMoveMarker;

    sf::Sound m_soundPlaceBlack; // Đặt quân (có thể dùng chung hoặc tách tùy bạn)
    sf::Sound m_soundPlaceWhite; // Đặt quân (có thể dùng chung hoặc tách tùy bạn)

    // Capture riêng
    sf::Sound m_soundCaptureBlack;
    sf::Sound m_soundCaptureWhite;

    // Error riêng
    sf::Sound m_soundErrorBlack;
    sf::Sound m_soundErrorWhite;

    sf::Sound m_soundPassBlack;
    sf::Sound m_soundPassWhite;

    // --- Biến tính toán vị trí ---
    float m_boardTopLeftX;
    float m_boardTopLeftY;
    float m_cellSpacing;

    float m_timeLimitBlack; // Thời gian còn lại của Đen (giây)
    float m_timeLimitWhite; // Thời gian còn lại của Trắng (giây)
    bool m_isTimeLimitEnabled; // Có bật giới hạn không?

    // UI hiển thị giờ (nếu muốn)

    // 1. Nền bảng giờ
    sf::Sprite m_timerPanel;

    // 2. Icon đại diện (Quân cờ nhỏ)
    sf::Sprite m_timerIconBlack;
    sf::Sprite m_timerIconWhite;
    sf::Text m_timerTextBlack;
    sf::Text m_timerTextWhite;

    // [THÊM MỚI] HIỂN THỊ SỐ QUÂN BỊ BẮT
    sf::Text m_onBoardTextBlack; // Số quân bị bắt của Đen
    sf::Text m_onBoardTextWhite; // Số quân bị bắt của Trắng

    // Hàm helper định dạng giây thành "10:00"
    std::string formatTime(float seconds);

    // [THÊM MỚI] Hàm cập nhật hiển thị
    void updateTimerDisplay();
    void updateOnBoardCount(); // Cần gọi hàm này trong update()

    bool m_shouldDrawStatusPanel;

    // [THÊM MỚI] UI Kết quả trận đấu (Win Banner)
    sf::RectangleShape m_winPanel;
    sf::Text m_winText;

    std::unique_ptr<UI::ScoringOverlay> m_scoringOverlay;

    // Hàm xử lý logic khi ván đấu kết thúc
    void endGameLogic();

    bool m_isScoringMode; // Cờ đánh dấu đang tính điểm

    // [THÊM DÒNG NÀY VÀO]
    std::string m_endReason;


    // [THÊM MỚI] --- BOT & THREADING ---
    // [SỬA] Đổi thành shared_ptr
    std::shared_ptr<IBot> m_bot;
    std::future<BotMove> m_aiFuture;
    bool m_isAiThinkingWorker = false;

    std::vector<sf::Vector2i> m_deadStones;

    // [THÊM] Biến lưu nước đi chờ đồng bộ
    PendingMove m_pendingPlayerMove;

    // Hàm helper
    void initBot();

    // [THÊM] Cờ đánh dấu đang khởi tạo game (Loading đầu game)
    std::atomic<bool> m_isInitializing{false};

    // [THÊM] Hàm helper để chạy logic khởi tạo


    void performSyncBoardInternal();
};
