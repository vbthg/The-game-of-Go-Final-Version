#pragma once

#include <vector>
#include <string>
#include <stack>
#include "SaveDefinition.h"

enum class StoneType {
    Empty,
    Black,
    White
};

enum class TerritoryOwner
{
    None,
    Black,
    White,
    Neutral
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

class GameLogic
{
public:
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

    const std::vector<std::vector<StoneType>>& getBoard() const;
    std::vector<std::vector<TerritoryOwner>> calculateTerritory();

    bool saveToFile(const std::string& filePath, const SaveInfo& info, float timeBlack, float timeWhite) const;
    bool loadFromFile(const std::string& filePath, float& timeBlack, float& timeWhite);

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
    bool isSuicide(int x, int y, StoneType player);
    bool isKo(int x, int y);
    std::vector<std::pair<int, int>> checkAndRemoveCaptures(int placedX, int placedY, StoneType player);
};
