// include/GameLogic.h
#pragma once

#include <vector>
#include <string>
#include <stack>
#include <SFML/System/Vector2.hpp>
#include "SaveDefinition.h"

enum class StoneType
{
    Empty,
    Black,
    White
};

enum class TerritoryOwner
{
    None,    // Chưa xét
    Black,   // Lãnh thổ Đen
    White,   // Lãnh thổ Trắng
    Neutral  // Vùng trung lập
};

struct MoveResult
{
    bool success;
    std::string message;
    std::vector<std::pair<int, int>> capturedStones;
    bool gameEnded;
};

struct GameStateSnapshot
{
    std::vector<std::vector<StoneType>> board;
    bool isBlacksTurn;
    std::pair<int, int> koPosition;
    bool lastPlayerPassed;
};

struct TerritoryRegion
{
    TerritoryOwner owner;
    std::vector<sf::Vector2i> points;
    std::vector<sf::Vector2i> boundaries;
};

struct DeadStoneInfo
{
    sf::Vector2i pos;
    TerritoryOwner owner;
};

struct ScoreData
{
    float blackStones = 0;
    float blackTerritory = 0;
    float whiteStones = 0;
    float whiteTerritory = 0;
    float komi = 0;
};



class GameLogic
{
public:
    struct StoneCount
    {
        int blackStones = 0;
        int whiteStones = 0;
    };

    GameLogic(int size);

    void newGame();
    MoveResult attemptMove(int x, int y);
    MoveResult attemptPass();


    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;


    StoneType getStoneAt(int x, int y) const;
    bool isBlacksTurn() const;
    int getBoardSize() const { return m_boardSize; }


    bool saveToFile(const std::string& filePath, const SaveInfo& info, float timeBlack, float timeWhite, int difficulty) const;
    bool loadFromFile(const std::string& filePath, float& timeBlack, float& timeWhite, std::string& modeStr, int& difficulty, std::string& endReason);

    std::vector<TerritoryRegion> getTerritoryRegions(const std::vector<sf::Vector2i>& deadStones) const;

    const std::vector<std::vector<StoneType>>& getBoard() const { return m_board; }

    ScoreData calculateScore(const std::vector<DeadStoneInfo>& deadStones, float komi);

    StoneCount getStoneCount() const;

private:
    int m_boardSize;

    std::vector<std::vector<StoneType>> m_board;
    std::vector<std::vector<StoneType>> m_previousBoard;
    bool m_isBlacksTurn;
    std::pair<int, int> m_koPosition;
    bool m_lastPlayerPassed;

    std::stack<GameStateSnapshot> m_undoStack;
    std::stack<GameStateSnapshot> m_redoStack;

    GameStateSnapshot createSnapshot() const;
    void restoreState(const GameStateSnapshot& state);

    int findGroupLiberties(int x, int y, StoneType player,
                           std::vector<std::vector<bool>>& visited_stones,
                           std::vector<std::pair<int, int>>& group,
                           std::vector<std::vector<bool>>& visited_liberties);

    void removeGroup(const std::vector<std::pair<int, int>>& group);
    bool isKo(int x, int y);
    bool isSuicide(int x, int y, StoneType player);
    std::vector<std::pair<int, int>> checkAndRemoveCaptures(int x, int y, StoneType player);
};
