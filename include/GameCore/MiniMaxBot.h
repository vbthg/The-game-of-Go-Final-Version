#pragma once
#include "IBot.h"
#include <vector>
#include <limits>
#include <algorithm>
#include <ctime>
#include <cstdlib>

enum class MMStone { Empty, Black, White };

class MiniMaxBot : public IBot
{
private:
    int m_boardSize;
    int m_depth;
    std::vector<std::vector<MMStone>> m_board;

    const int DX[4] = {0, 0, 1, -1};
    const int DY[4] = {1, -1, 0, 0};

public:
    MiniMaxBot(int size, int depth) : m_boardSize(size), m_depth(depth)
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        setBoardSize(size);
    }

    void init() override
    {
        for(auto& row : m_board)
        {
            std::fill(row.begin(), row.end(), MMStone::Empty);
        }
    }

    void setBoardSize(int size) override
    {
        m_boardSize = size;
        m_board.assign(size, std::vector<MMStone>(size, MMStone::Empty));
    }

    void syncMove(std::string color, int x, int y) override
    {
        if(x >= 0 && x < m_boardSize && y >= 0 && y < m_boardSize)
        {
            m_board[y][x] = (color == "black" ? MMStone::Black : MMStone::White);
        }
    }

    BotMove generateMove(bool isBlackTurn) override;

    std::vector<sf::Vector2i> getDeadStones() override
    {
        return {};
    }

private:
    int evaluate(MMStone myColor);
    int minimax(int depth, bool isMaximizing, MMStone myColor, int alpha, int beta);
    std::vector<sf::Vector2i> getCandidateMoves(MMStone myColor); // [SỬA] Nhận màu để check luật
    int simulateMove(int x, int y, MMStone color, std::vector<std::vector<MMStone>>& backupBoard);

    int countLiberties(int x, int y, MMStone color, std::vector<std::vector<bool>>& visited);

    bool isLegalMove(int x, int y, MMStone myColor);

    bool isValid(int x, int y) const
    {
        return x >= 0 && x < m_boardSize && y >= 0 && y < m_boardSize;
    }
};
