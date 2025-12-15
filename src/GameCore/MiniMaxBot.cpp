#include "MiniMaxBot.h"
#include <iostream>

const int INF = 1000000000;

BotMove MiniMaxBot::generateMove(bool isBlackTurn)
{
    BotMove bestMove;
    bestMove.isPass = true;

    MMStone myColor = isBlackTurn ? MMStone::Black : MMStone::White;

    std::vector<sf::Vector2i> candidates = getCandidateMoves(myColor);

    if(candidates.empty())
    {
        if(m_board[m_boardSize/2][m_boardSize/2] == MMStone::Empty)
        {
            bestMove.x = m_boardSize / 2;
            bestMove.y = m_boardSize / 2;
            bestMove.isPass = false;
            return bestMove;
        }
        return bestMove;
    }

    int alpha = -INF;
    int beta = INF;
    int bestVal = -INF;

    for(const auto& move : candidates)
    {
        if(!isLegalMove(move.x, move.y, myColor)) continue;

        std::vector<std::vector<MMStone>> backup = m_board;

        int capturedBonus = simulateMove(move.x, move.y, myColor, backup) * 1000;

        int moveVal = minimax(m_depth - 1, false, myColor, alpha, beta);
        moveVal += capturedBonus;

        m_board = backup;

        if(moveVal > bestVal)
        {
            bestVal = moveVal;
            bestMove.x = move.x;
            bestMove.y = move.y;
            bestMove.isPass = false;
        }

        if(moveVal == bestVal)
        {
            if(std::rand() % 100 < 30)
            {
                bestMove.x = move.x;
                bestMove.y = move.y;
            }
        }

        alpha = std::max(alpha, bestVal);
    }

    if(!bestMove.isPass)
    {
        if(!isLegalMove(bestMove.x, bestMove.y, myColor))
        {
            bestMove.isPass = true;
        }
    }

    return bestMove;
}

int MiniMaxBot::minimax(int depth, bool isMaximizing, MMStone myColor, int alpha, int beta)
{
    if(depth == 0) return evaluate(myColor);

    MMStone currentPlayer = isMaximizing ? myColor : (myColor == MMStone::Black ? MMStone::White : MMStone::Black);

    std::vector<sf::Vector2i> candidates = getCandidateMoves(currentPlayer);

    if(candidates.empty()) return evaluate(myColor);

    if(isMaximizing)
    {
        int maxEval = -INF;
        for(const auto& move : candidates)
        {
            if(!isLegalMove(move.x, move.y, currentPlayer)) continue;

            std::vector<std::vector<MMStone>> backup = m_board;
            simulateMove(move.x, move.y, currentPlayer, backup);

            int eval = minimax(depth - 1, false, myColor, alpha, beta);
            m_board = backup;

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if(beta <= alpha) break;
        }
        return maxEval;
    }
    else
    {
        int minEval = INF;
        for(const auto& move : candidates)
        {
            if(!isLegalMove(move.x, move.y, currentPlayer)) continue;

            std::vector<std::vector<MMStone>> backup = m_board;
            simulateMove(move.x, move.y, currentPlayer, backup);

            int eval = minimax(depth - 1, true, myColor, alpha, beta);
            m_board = backup;

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if(beta <= alpha) break;
        }
        return minEval;
    }
}

bool MiniMaxBot::isLegalMove(int x, int y, MMStone myColor)
{
    if(!isValid(x, y)) return false;
    if(m_board[y][x] != MMStone::Empty) return false;

    m_board[y][x] = myColor;

    bool capturesEnemy = false;
    MMStone enemyColor = (myColor == MMStone::Black) ? MMStone::White : MMStone::Black;

    for(int i = 0; i < 4; ++i)
    {
        int nx = x + DX[i];
        int ny = y + DY[i];
        if(isValid(nx, ny) && m_board[ny][nx] == enemyColor)
        {
            std::vector<std::vector<bool>> visited(m_boardSize, std::vector<bool>(m_boardSize, false));
            if(countLiberties(nx, ny, enemyColor, visited) == 0)
            {
                capturesEnemy = true;
                break;
            }
        }
    }

    std::vector<std::vector<bool>> visited(m_boardSize, std::vector<bool>(m_boardSize, false));
    int myLibs = countLiberties(x, y, myColor, visited);

    m_board[y][x] = MMStone::Empty;

    if(!capturesEnemy && myLibs == 0)
    {
        return false;
    }

    return true;
}

int MiniMaxBot::evaluate(MMStone myColor)
{
    int score = 0;
    MMStone enemyColor = (myColor == MMStone::Black) ? MMStone::White : MMStone::Black;

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            MMStone s = m_board[y][x];
            if(s == MMStone::Empty) continue;

            std::vector<std::vector<bool>> visited(m_boardSize, std::vector<bool>(m_boardSize, false));
            int liberties = countLiberties(x, y, s, visited);

            if(s == myColor)
            {
                score += 10;
                if(liberties == 1) score -= 500;
                else if(liberties >= 4) score += 50;
            }
            else
            {
                score -= 10;
                if(liberties == 1) score += 600;
            }
        }
    }
    return score;
}

int MiniMaxBot::simulateMove(int x, int y, MMStone color, std::vector<std::vector<MMStone>>& backupBoard)
{
    m_board[y][x] = color;
    MMStone enemy = (color == MMStone::Black) ? MMStone::White : MMStone::Black;
    int capturedTotal = 0;

    for(int i = 0; i < 4; ++i)
    {
        int nx = x + DX[i];
        int ny = y + DY[i];

        if(isValid(nx, ny) && m_board[ny][nx] == enemy)
        {
            std::vector<std::vector<bool>> visited(m_boardSize, std::vector<bool>(m_boardSize, false));
            if(countLiberties(nx, ny, enemy, visited) == 0)
            {
                for(int r = 0; r < m_boardSize; ++r)
                {
                    for(int c = 0; c < m_boardSize; ++c)
                    {
                        if(visited[r][c])
                        {
                            m_board[r][c] = MMStone::Empty;
                            capturedTotal++;
                        }
                    }
                }
            }
        }
    }
    return capturedTotal;
}

int MiniMaxBot::countLiberties(int x, int y, MMStone color, std::vector<std::vector<bool>>& visited)
{
    if(visited[y][x]) return 0;
    visited[y][x] = true;

    int liberties = 0;
    for(int i = 0; i < 4; ++i)
    {
        int nx = x + DX[i];
        int ny = y + DY[i];
        if(!isValid(nx, ny)) continue;

        if(m_board[ny][nx] == MMStone::Empty)
        {
            liberties++;
        }
        else if(m_board[ny][nx] == color && !visited[ny][nx])
        {
            liberties += countLiberties(nx, ny, color, visited);
        }
    }
    return liberties;
}

std::vector<sf::Vector2i> MiniMaxBot::getCandidateMoves(MMStone myColor)
{
    std::vector<sf::Vector2i> moves;
    std::vector<std::vector<bool>> marked(m_boardSize, std::vector<bool>(m_boardSize, false));
    bool hasStone = false;

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            if(m_board[y][x] != MMStone::Empty)
            {
                hasStone = true;
                for(int dy = -1; dy <= 1; ++dy)
                {
                    for(int dx = -1; dx <= 1; ++dx)
                    {
                        int nx = x + dx;
                        int ny = y + dy;
                        if(isValid(nx, ny) && m_board[ny][nx] == MMStone::Empty && !marked[ny][nx])
                        {
                            if(isLegalMove(nx, ny, myColor))
                            {
                                moves.push_back({nx, ny});
                                marked[ny][nx] = true;
                            }
                        }
                    }
                }
            }
        }
    }
    return moves;
}
