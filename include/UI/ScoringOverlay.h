#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "GameLogic.h"

struct ScoreData {
    int blackTerritory;
    int blackStones;
    int whiteTerritory;
    int whiteStones;
    float komi;
};

class ScoringOverlay {
public:
    ScoringOverlay(int boardSize, float cellSize, sf::Vector2f boardTopLeft, const sf::Font& font);

    // Gọi hàm này khi Game over
    void startAnimation(const std::vector<std::vector<TerritoryOwner>>& territoryMap,
                        const std::vector<std::vector<StoneType>>& boardState);

    void update(float deltaTime);
    void draw(sf::RenderTarget& target);

    bool isAnimationFinished() const;

    void setScoreData(const ScoreData& data);

    bool isScoreboardVisible() const; // để GamePlay biết khi nào cho phép click thoát

    bool contains(sf::Vector2f point) const;


private:

    // tạo dòng kẻ
    void addLine(float x, float y, float w, float h);
    // tạo text trong ô
    void addCellText(const std::string& str, float x, float y, bool isHeader = false, sf::Color color = sf::Color::White);

    // Các giai đoạn của hiệu ứng
    enum class Phase {
        Idle,
        BlackSpreading, // Đen lan ra
        Intermission,   // Nghỉ giữa hiệp
        WhiteSpreading, // Trắng lan ra
        FinalDelay,     // Chờ 3s trước khi hiện bảng
        Finished        // Hiện bảng điểm
    };

    struct TileEffect {
        sf::CircleShape shape; // Dùng hình tròn
        TerritoryOwner owner;  // Lưu chủ sở hữu để biết khi nào cần hiện
        float alpha = 0.f;     // Độ trong suốt hiện tại
        float targetAlpha = 0.f; // Độ trong suốt đích (VD: 150)
        float delay = 0.f;     // Thời gian chờ trước khi hiện
        bool active = false;
    };


    int m_boardSize;
    float m_cellSize;
    sf::Vector2f m_boardTopLeft;


    const sf::Font& m_font;
    sf::RectangleShape m_scorePanel;
    sf::Text m_txtTitle;
    sf::Text m_txtBlackDetail;
    sf::Text m_txtWhiteDetail;
    sf::Text m_txtResult;
    sf::Text m_txtInstruction; // "Nhấn để thoát"
    bool m_showScoreboard;
    float m_scoreboardAlpha;


    std::vector<sf::RectangleShape> m_tableLines;
    std::vector<sf::Text> m_tableTexts;

    std::vector<std::vector<TileEffect>> m_gridEffects;


    sf::RectangleShape m_dimmer;
    float m_dimmerAlpha;


    Phase m_currentPhase;
    float m_timer;          // Timer chạy trong từng Phase
    float m_maxBlackDelay;  // Thời gian lan lâu nhất của Đen
    float m_maxWhiteDelay;  // Thời gian lan lâu nhất của Trắng
};
