#include "GameLogic.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <queue>
#include <sstream>

const int DX[] = {0, 0, 1, -1};
const int DY[] = {1, -1, 0, 0};

GameLogic::GameLogic(int size) : m_boardSize(size)
{
    m_board.resize(m_boardSize, std::vector<StoneType>(m_boardSize, StoneType::Empty));
    m_previousBoard.resize(m_boardSize, std::vector<StoneType>(m_boardSize, StoneType::Empty));
    newGame();
}

void GameLogic::newGame()
{
    for(auto& row : m_board)
        std::fill(row.begin(), row.end(), StoneType::Empty);

    for(auto& row : m_previousBoard)
        std::fill(row.begin(), row.end(), StoneType::Empty);

    m_isBlacksTurn = true;
    m_koPosition = {-1, -1};
    m_lastPlayerPassed = false;

    while(!m_undoStack.empty()) m_undoStack.pop();
    while(!m_redoStack.empty()) m_redoStack.pop();
}

std::vector<TerritoryRegion> GameLogic::getTerritoryRegions(const std::vector<sf::Vector2i>& deadStones) const
{
    std::vector<TerritoryRegion> regions;
    std::vector<std::vector<bool>> visited(m_boardSize, std::vector<bool>(m_boardSize, false));

    std::vector<std::vector<bool>> deadMap(m_boardSize, std::vector<bool>(m_boardSize, false));

    for(const auto& p : deadStones)
    {
        if(p.x >= 0 && p.x < m_boardSize && p.y >= 0 && p.y < m_boardSize)
        {
            deadMap[p.y][p.x] = true;
        }
    }

    auto getEffectiveType = [&](int x, int y) -> StoneType
    {
        if(deadMap[y][x]) return StoneType::Empty;
        return m_board[y][x];
    };

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            if(getEffectiveType(x, y) == StoneType::Empty && !visited[y][x])
            {
                TerritoryRegion region;
                std::vector<sf::Vector2i> q;
                q.reserve(m_boardSize * m_boardSize);
                q.push_back({x, y});

                visited[y][x] = true;

                bool touchBlack = false;
                bool touchWhite = false;
                int head = 0;

                while(head < (int)q.size())
                {
                    sf::Vector2i curr = q[head++];
                    region.points.push_back(curr);

                    for(int i = 0; i < 4; ++i)
                    {
                        int nx = curr.x + DX[i];
                        int ny = curr.y + DY[i];

                        if(nx < 0 || nx >= m_boardSize || ny < 0 || ny >= m_boardSize) continue;

                        StoneType neighborType = getEffectiveType(nx, ny);

                        if(neighborType == StoneType::Black)
                        {
                            touchBlack = true;
                            region.boundaries.push_back({nx, ny});
                        }
                        else if(neighborType == StoneType::White)
                        {
                            touchWhite = true;
                            region.boundaries.push_back({nx, ny});
                        }
                        else if(neighborType == StoneType::Empty && !visited[ny][nx])
                        {
                            visited[ny][nx] = true;
                            q.push_back({nx, ny});
                        }
                    }
                }

                if(touchBlack && !touchWhite) region.owner = TerritoryOwner::Black;
                else if(!touchBlack && touchWhite) region.owner = TerritoryOwner::White;
                else region.owner = TerritoryOwner::Neutral;

                if(!region.points.empty()) regions.push_back(region);
            }
        }
    }
    return regions;
}

GameLogic::StoneCount GameLogic::getStoneCount() const
{
    StoneCount counts;

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            if(m_board[y][x] == StoneType::Black)
            {
                counts.blackStones++;
            }
            else if(m_board[y][x] == StoneType::White)
            {
                counts.whiteStones++;
            }
        }
    }
    return counts;
}

ScoreData GameLogic::calculateScore(const std::vector<DeadStoneInfo>& deadStones, float komi)
{
    ScoreData data;
    data.komi = komi;

    std::vector<std::vector<bool>> isDead(m_boardSize, std::vector<bool>(m_boardSize, false));
    for(const auto& ds : deadStones)
    {
        if(ds.pos.x >= 0 && ds.pos.x < m_boardSize && ds.pos.y >= 0 && ds.pos.y < m_boardSize)
        {
            isDead[ds.pos.y][ds.pos.x] = true;
        }
    }

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            if(m_board[y][x] == StoneType::Black && !isDead[y][x])
            {
                data.blackStones++;
            }
            else if(m_board[y][x] == StoneType::White && !isDead[y][x])
            {
                data.whiteStones++;
            }
        }
    }

    std::vector<sf::Vector2i> simpleDeadStones;
    for(const auto& ds : deadStones)
    {
        simpleDeadStones.push_back(ds.pos);
    }

    std::vector<TerritoryRegion> regions = getTerritoryRegions(simpleDeadStones);

    for(const auto& r : regions)
    {
        if(r.owner == TerritoryOwner::Black)
        {
            data.blackTerritory += r.points.size();
        }
        else if(r.owner == TerritoryOwner::White)
        {
            data.whiteTerritory += r.points.size();
        }
    }

    return data;
}

StoneType GameLogic::getStoneAt(int x, int y) const
{
    if(x >= 0 && x < m_boardSize && y >= 0 && y < m_boardSize)
        return m_board[y][x];
    return StoneType::Empty;
}

bool GameLogic::isBlacksTurn() const
{
    return m_isBlacksTurn;
}

MoveResult GameLogic::attemptPass()
{
    MoveResult result;
    result.success = true;
    result.capturedStones.clear();

    m_undoStack.push(createSnapshot());
    while(!m_redoStack.empty()) m_redoStack.pop();

    if(m_lastPlayerPassed)
    {
        result.message = "Both players passed. Game Over.";
        result.gameEnded = true;
    }
    else
    {
        result.message = "Player passed.";
        result.gameEnded = false;
    }

    m_lastPlayerPassed = true;
    m_isBlacksTurn = !m_isBlacksTurn;

    return result;
}

MoveResult GameLogic::attemptMove(int x, int y)
{
    MoveResult result;
    result.success = false;
    result.gameEnded = false;

    if(x < 0 || x >= m_boardSize || y < 0 || y >= m_boardSize)
    {
        result.message = "Invalid: Out of bounds.";
        return result;
    }
    if(m_board[y][x] != StoneType::Empty)
    {
        result.message = "Invalid: Spot occupied.";
        return result;
    }

    StoneType currentPlayer = m_isBlacksTurn ? StoneType::Black : StoneType::White;

    if(isKo(x, y))
    {
        result.message = "Invalid: Ko rule violation.";
        return result;
    }

    m_board[y][x] = currentPlayer;

    bool willCapture = false;
    StoneType opponent = (currentPlayer == StoneType::Black) ? StoneType::White : StoneType::Black;

    for(int i = 0; i < 4; ++i)
    {
        int nx = x + DX[i];
        int ny = y + DY[i];

        if(nx >= 0 && nx < m_boardSize && ny >= 0 && ny < m_boardSize)
        {
            if(m_board[ny][nx] == opponent)
            {
                std::vector<std::vector<bool>> vStones(m_boardSize, std::vector<bool>(m_boardSize, false));
                std::vector<std::pair<int, int>> group;
                std::vector<std::vector<bool>> vLibs(m_boardSize, std::vector<bool>(m_boardSize, false));

                if(findGroupLiberties(nx, ny, opponent, vStones, group, vLibs) == 0)
                {
                    willCapture = true;
                    break;
                }
            }
        }
    }

    if(!willCapture && isSuicide(x, y, currentPlayer))
    {
        m_board[y][x] = StoneType::Empty;
        result.message = "Invalid: Suicide.";
        return result;
    }

    m_board[y][x] = StoneType::Empty;
    m_undoStack.push(createSnapshot());
    while(!m_redoStack.empty()) m_redoStack.pop();

    m_board[y][x] = currentPlayer;

    result.capturedStones = checkAndRemoveCaptures(x, y, currentPlayer);

    if(result.capturedStones.size() == 1)
    {
        std::vector<std::vector<bool>> visitedStones(m_boardSize, std::vector<bool>(m_boardSize, false));
        std::vector<std::pair<int, int>> group;
        std::vector<std::vector<bool>> visitedLibs(m_boardSize, std::vector<bool>(m_boardSize, false));

        int myLiberties = findGroupLiberties(x, y, currentPlayer, visitedStones, group, visitedLibs);

        if(myLiberties == 1)
        {
            m_koPosition = result.capturedStones[0];
        }
        else
        {
            m_koPosition = {-1, -1};
        }
    }
    else
    {
        m_koPosition = {-1, -1};
    }

    m_lastPlayerPassed = false;
    m_isBlacksTurn = !m_isBlacksTurn;
    result.success = true;
    result.message = "OK";

    return result;
}

void GameLogic::undo()
{
    if(m_undoStack.empty()) return;

    m_redoStack.push(createSnapshot());

    GameStateSnapshot prev = m_undoStack.top();
    m_undoStack.pop();
    restoreState(prev);
}

void GameLogic::redo()
{
    if(m_redoStack.empty()) return;

    m_undoStack.push(createSnapshot());

    GameStateSnapshot next = m_redoStack.top();
    m_redoStack.pop();
    restoreState(next);
}

bool GameLogic::canUndo() const
{
    return !m_undoStack.empty();
}
bool GameLogic::canRedo() const
{
    return !m_redoStack.empty();
}

GameStateSnapshot GameLogic::createSnapshot() const
{
    GameStateSnapshot snap;
    snap.board = m_board;
    snap.isBlacksTurn = m_isBlacksTurn;
    snap.koPosition = m_koPosition;
    snap.lastPlayerPassed = m_lastPlayerPassed;
    return snap;
}

void GameLogic::restoreState(const GameStateSnapshot& state)
{
    m_board = state.board;
    m_isBlacksTurn = state.isBlacksTurn;
    m_koPosition = state.koPosition;
    m_lastPlayerPassed = state.lastPlayerPassed;
}

std::vector<std::pair<int, int>> GameLogic::checkAndRemoveCaptures(int x, int y, StoneType player)
{
    std::vector<std::pair<int, int>> allCaptured;
    StoneType opponent = (player == StoneType::Black) ? StoneType::White : StoneType::Black;

    for(int i = 0; i < 4; ++i)
    {
        int nx = x + DX[i];
        int ny = y + DY[i];

        if(nx >= 0 && nx < m_boardSize && ny >= 0 && ny < m_boardSize)
        {
            if(m_board[ny][nx] == opponent)
            {
                std::vector<std::vector<bool>> visitedStones(m_boardSize, std::vector<bool>(m_boardSize, false));
                std::vector<std::pair<int, int>> group;
                std::vector<std::vector<bool>> visitedLiberties(m_boardSize, std::vector<bool>(m_boardSize, false));

                int libs = findGroupLiberties(nx, ny, opponent, visitedStones, group, visitedLiberties);
                if(libs == 0)
                {
                    removeGroup(group);
                    allCaptured.insert(allCaptured.end(), group.begin(), group.end());
                }
            }
        }
    }
    return allCaptured;
}

int GameLogic::findGroupLiberties(int x, int y, StoneType player,
                                  std::vector<std::vector<bool>>& visited_stones,
                                  std::vector<std::pair<int, int>>& group,
                                  std::vector<std::vector<bool>>& visited_liberties)
{
    if(visited_stones[y][x]) return 0;

    visited_stones[y][x] = true;
    group.push_back({x, y});

    int liberties = 0;

    for(int i = 0; i < 4; ++i)
    {
        int nx = x + DX[i];
        int ny = y + DY[i];

        if(nx >= 0 && nx < m_boardSize && ny >= 0 && ny < m_boardSize)
        {
            if(m_board[ny][nx] == StoneType::Empty)
            {
                if(!visited_liberties[ny][nx])
                {
                    visited_liberties[ny][nx] = true;
                    liberties++;
                }
            }
            else if(m_board[ny][nx] == player)
            {
                if(!visited_stones[ny][nx])
                {
                    liberties += findGroupLiberties(nx, ny, player, visited_stones, group, visited_liberties);
                }
            }
        }
    }
    return liberties;
}

void GameLogic::removeGroup(const std::vector<std::pair<int, int>>& group)
{
    for(const auto& p : group)
    {
        m_board[p.second][p.first] = StoneType::Empty;
    }
}

bool GameLogic::isSuicide(int x, int y, StoneType player)
{
    std::vector<std::vector<bool>> visitedStones(m_boardSize, std::vector<bool>(m_boardSize, false));
    std::vector<std::pair<int, int>> group;
    std::vector<std::vector<bool>> visitedLiberties(m_boardSize, std::vector<bool>(m_boardSize, false));

    int libs = findGroupLiberties(x, y, player, visitedStones, group, visitedLiberties);

    return (libs == 0);
}

bool GameLogic::isKo(int x, int y)
{
    if(x == m_koPosition.first && y == m_koPosition.second)
    {
        return true;
    }
    return false;
}

bool GameLogic::saveToFile(const std::string& filePath, const SaveInfo& info, float timeBlack, float timeWhite, int difficulty) const
{
    std::ofstream file(filePath);
    if(!file.is_open()) return false;

    file << info.userTitle << "|" << info.timestamp << "|" << info.boardSize << "|"
         << info.modeStr << "|" << info.status << "|" << difficulty << "|" << info.endReason << "\n";

    file << m_boardSize << " " << (m_isBlacksTurn ? 1 : 0) << " "
         << m_koPosition.first << " " << m_koPosition.second << " "
         << (m_lastPlayerPassed ? 1 : 0) << " "
         << timeBlack << " " << timeWhite << "\n";

    for(const auto& row : m_board)
    {
        for(const auto& stone : row)
        {
            int val = 0;
            if(stone == StoneType::Black) val = 1;
            else if(stone == StoneType::White) val = 2;
            file << val << " ";
        }
        file << "\n";
    }

    file.close();
    return true;
}

bool GameLogic::loadFromFile(const std::string& filePath, float& timeBlack, float& timeWhite, std::string& modeStr, int& difficulty, std::string& endReason)
{
    std::ifstream file(filePath);
    if(!file.is_open()) return false;

    std::string header;
    std::getline(file, header);

    if(!header.empty())
    {
        std::stringstream ss(header);
        std::string segment;
        std::vector<std::string> parts;

        while(std::getline(ss, segment, '|'))
        {
            parts.push_back(segment);
        }

        if(parts.size() >= 4)
        {
            modeStr = parts[3];
        }

        if(parts.size() >= 6)
        {
            try
            {
                difficulty = std::stoi(parts[5]);
            }
            catch(...)
            {
                difficulty = 1;
            }
        }
        else
        {
            difficulty = 1;
        }

        if(parts.size() >= 7)
        {
            endReason = parts[6];
        }
        else
        {
            endReason = "";
        }

//        std::cout << "END REASON: " << endReason << "\n";
    }

    int size, turn, koX, koY, pass;
    if(!(file >> size >> turn >> koX >> koY >> pass >> timeBlack >> timeWhite))
    {
        return false;
    }

    if(size != m_boardSize)
    {
        m_boardSize = size;
        m_board.assign(size, std::vector<StoneType>(size, StoneType::Empty));
        m_previousBoard.assign(size, std::vector<StoneType>(size, StoneType::Empty));
    }

    m_isBlacksTurn = (turn == 1);
    m_koPosition = {koX, koY};
    m_lastPlayerPassed = (pass == 1);

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            int val;
            file >> val;
            if(val == 1) m_board[y][x] = StoneType::Black;
            else if(val == 2) m_board[y][x] = StoneType::White;
            else m_board[y][x] = StoneType::Empty;
        }
    }

    while(!m_undoStack.empty()) m_undoStack.pop();
    while(!m_redoStack.empty()) m_redoStack.pop();

    return true;
}
