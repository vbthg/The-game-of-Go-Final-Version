#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <deque>
#include "GameLogic.h"

namespace UI
{

struct ActiveEdge
{
    sf::Sprite sprite;
    float targetScaleY;
    sf::Vector2i endNode;
    float progress;
    bool isTriggered;

    ActiveEdge() { progress = 0, isTriggered = 0; }
};

struct VirtualConnectedComponent
{
    TerritoryOwner owner;
    std::vector<sf::Vector2i> allPoints;
    sf::Vector2i anchorPoint;
    float anchorDistance = 1e9;
    bool isActive = false;
    bool isFinished = false;
    std::deque<ActiveEdge> activeEdges;
};

struct Node
{
    sf::Vector2i v;
    sf::Vector2i dir;
    int ind;
    bool isDrawn;

    Node() { ind = 0; isDrawn = false; }
    Node(sf::Vector2i _v, sf::Vector2i _dir, int _ind) : v(_v), dir(_dir), ind(_ind), isDrawn(false) {}
};

class ScoringOverlay
{
public:
    enum class Phase
    {
        Idle,
        DeadStoneAnim,
        DeadStoneDelay,
        BlackWave,
        Intermission,
        WhiteWave,
        FinalDelay,
        BoardAppear,
        Counting,
        StampAppear,
        Finished
    };

    ScoringOverlay(int boardSize, float cellSize, sf::Vector2f boardTopLeft, const sf::Font& font);

    void setScoreData(const ScoreData& data);

    void startAnimation(const std::vector<TerritoryRegion>& regions, const std::vector<DeadStoneInfo>& deadStones);

    void update(float deltaTime);
    void draw(sf::RenderTarget& target);

    bool isFinished() const { return m_currentPhase == Phase::Finished; }

    bool isAnimationFinished() const;
    bool isScoreboardVisible() const;

    bool contains(const sf::Vector2f& point, sf::Vector2u windowSize) const;

    void showSimpleResult(const std::string& message, bool blackWon);

private:
    int m_boardSize;
    float m_cellSize;
    sf::Vector2f m_boardTopLeft;
    sf::Font m_font;
    sf::Texture& m_texGradientLine;

    Phase m_currentPhase;
    float m_timer;

    std::vector<DeadStoneInfo> m_deadStones;
    std::vector<VirtualConnectedComponent> m_ConnectedComponents;
    std::vector<std::vector<std::vector<Node>>> m_adjNodes;
    float m_maxDistBlack;
    float m_maxDistWhite;

    sf::Sprite m_boardSprite;
    sf::Sprite m_stampSprite;

    sf::Text m_txtScoreBlack[4];
    sf::Text m_txtScoreWhite[4];

    ScoreData m_data;

    sf::Sound m_soundTick;
    sf::Sound m_soundStamp;

    float m_animTimer;
    float m_countDuration;
    float m_soundTimer;

    float m_curScoreBlack[4];
    float m_curScoreWhite[4];

    sf::Sprite m_iconBlackStone;
    sf::Sprite m_iconWhiteStone;

    void spawnEdge(VirtualConnectedComponent& curComp, const sf::Vector2i& parent);
    bool updatePhaseLogic(float deltaTime, TerritoryOwner targetOwner, float maxDist);
    void updateDisappear(float progress);
    void renderDeadStones(sf::RenderTarget& target);

    void updateBoardAppear(float dt);
    void updateCounting(float dt);
    void updateStampAppear(float dt);
    float easeOutBack(float x);

    bool m_isSimpleMode = false;
    sf::Text m_simpleResultText;
};

}
