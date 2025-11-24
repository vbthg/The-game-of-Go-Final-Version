#include "GameLogic.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <queue>

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
    for (auto& row : m_board)
        std::fill(row.begin(), row.end(), StoneType::Empty);

    for (auto& row : m_previousBoard)
        std::fill(row.begin(), row.end(), StoneType::Empty);

    m_isBlacksTurn = true;
    m_koPosition = {-1, -1};
    m_lastPlayerPassed = false;

    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();
}

GameStateSnapshot GameLogic::createSnapshot() const {
    return { m_board, m_isBlacksTurn, m_koPosition, m_lastPlayerPassed };
}

void GameLogic::restoreState(const GameStateSnapshot& state) {
    m_board = state.board;
    m_isBlacksTurn = state.isBlacksTurn;
    m_koPosition = state.koPosition;
    m_lastPlayerPassed = state.lastPlayerPassed;
}

void GameLogic::undo() {
    if (m_undoStack.empty()) return;

    m_redoStack.push(createSnapshot());

    GameStateSnapshot prevState = m_undoStack.top();
    m_undoStack.pop();

    restoreState(prevState);
}

void GameLogic::redo() {
    if (m_redoStack.empty()) return;

    m_undoStack.push(createSnapshot());

    GameStateSnapshot nextState = m_redoStack.top();
    m_redoStack.pop();

    restoreState(nextState);
}

bool GameLogic::canUndo() const { return !m_undoStack.empty(); }
bool GameLogic::canRedo() const { return !m_redoStack.empty(); }


int GameLogic::findGroupLiberties(int x, int y, StoneType player,
                                  std::vector<std::vector<bool>>& visited_stones,
                                  std::vector<std::pair<int, int>>& group,
                                  std::vector<std::vector<bool>>& visited_liberties)
{
    if (x < 0 || x >= m_boardSize || y < 0 || y >= m_boardSize) return 0;
    if (visited_stones[y][x]) return 0;

    if (m_board[y][x] == StoneType::Empty)
    {
        if (!visited_liberties[y][x])
        {
            visited_liberties[y][x] = true;
            return 1;
        }
        return 0;
    }

    if (m_board[y][x] != player) return 0;

    visited_stones[y][x] = true;
    group.push_back({x, y});

    int totalLibs = 0;
    for (int i = 0; i < 4; ++i)
    {
        totalLibs += findGroupLiberties(x + DX[i], y + DY[i], player, visited_stones, group, visited_liberties);
    }
    return totalLibs;
}

void GameLogic::removeGroup(const std::vector<std::pair<int, int>>& group)
{
    for (const auto& stone : group)
    {
        m_board[stone.second][stone.first] = StoneType::Empty;
    }
}

bool GameLogic::isSuicide(int x, int y, StoneType player)
{
    std::vector<std::vector<bool>> visited_stones_us(m_boardSize, std::vector<bool>(m_boardSize, false));
    std::vector<std::vector<bool>> visited_liberties_us(m_boardSize, std::vector<bool>(m_boardSize, false));
    std::vector<std::pair<int, int>> group_us;

    int myLibs = findGroupLiberties(x, y, player, visited_stones_us, group_us, visited_liberties_us);

    bool capturedOpponent = false;
    StoneType opponent = (player == StoneType::Black) ? StoneType::White : StoneType::Black;

    std::vector<std::vector<bool>> visited_stones_opponent(m_boardSize, std::vector<bool>(m_boardSize, false));

    for (int i = 0; i < 4; ++i)
    {
        int nx = x + DX[i];
        int ny = y + DY[i];
        if (nx < 0 || nx >= m_boardSize || ny < 0 || ny >= m_boardSize) continue;

        if (m_board[ny][nx] == opponent)
        {
            std::vector<std::vector<bool>> visited_liberties_opponent(m_boardSize, std::vector<bool>(m_boardSize, false));
            std::vector<std::pair<int, int>> group_opponent;

            int oppLibs = findGroupLiberties(nx, ny, opponent, visited_stones_opponent, group_opponent, visited_liberties_opponent);

            if (oppLibs == 0)
            {
                capturedOpponent = true;
                break;
            }
        }
    }

    return (myLibs == 0 && !capturedOpponent);
}

std::vector<std::pair<int, int>> GameLogic::checkAndRemoveCaptures(int placedX, int placedY, StoneType player)
{
    std::vector<std::pair<int, int>> totalCaptured;
    StoneType opponent = (player == StoneType::Black) ? StoneType::White : StoneType::Black;
    std::vector<std::vector<bool>> visited_stones_opponent(m_boardSize, std::vector<bool>(m_boardSize, false));

    for (int i = 0; i < 4; ++i)
    {
        int nx = placedX + DX[i];
        int ny = placedY + DY[i];
        if (nx < 0 || nx >= m_boardSize || ny < 0 || ny >= m_boardSize) continue;

        if (m_board[ny][nx] == opponent)
        {
            std::vector<std::vector<bool>> visited_liberties_opponent(m_boardSize, std::vector<bool>(m_boardSize, false));
            std::vector<std::pair<int, int>> group_opponent;

            int oppLibs = findGroupLiberties(nx, ny, opponent, visited_stones_opponent, group_opponent, visited_liberties_opponent);

            if (oppLibs == 0)
            {
                removeGroup(group_opponent);
                totalCaptured.insert(totalCaptured.end(), group_opponent.begin(), group_opponent.end());
            }
        }
    }
    return totalCaptured;
}

bool GameLogic::isKo(int x, int y)
{
    return (m_koPosition.first == x && m_koPosition.second == y);
}

MoveResult GameLogic::attemptMove(int x, int y)
{
    MoveResult result;
    result.success = false;
    result.gameEnded = false;

    if (x < 0 || x >= m_boardSize || y < 0 || y >= m_boardSize || m_board[y][x] != StoneType::Empty)
    {
        result.message = "Invalid position or occupied.";
        return result;
    }

    StoneType currentPlayer = m_isBlacksTurn ? StoneType::Black : StoneType::White;

    if (isKo(x, y))
    {
        result.message = "Invalid: Ko rule violation.";
        return result;
    }

    m_undoStack.push(createSnapshot());
    while (!m_redoStack.empty()) m_redoStack.pop();

    m_board[y][x] = currentPlayer;

    if (isSuicide(x, y, currentPlayer))
    {
        m_board[y][x] = StoneType::Empty;
        m_undoStack.pop();
        result.message = "Invalid: Suicide.";
        return result;
    }

    result.capturedStones = checkAndRemoveCaptures(x, y, currentPlayer);

    if (result.capturedStones.size() == 1)
    {
        m_koPosition = result.capturedStones[0];
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

MoveResult GameLogic::attemptPass()
{
    m_undoStack.push(createSnapshot());
    while (!m_redoStack.empty()) m_redoStack.pop();

    MoveResult result;
    result.success = true;
    result.capturedStones.clear();

    if (m_lastPlayerPassed)
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

StoneType GameLogic::getStoneAt(int x, int y) const
{
    if (x < 0 || x >= m_boardSize || y < 0 || y >= m_boardSize)
        return StoneType::Empty;
    return m_board[y][x];
}

bool GameLogic::isBlacksTurn() const
{
    return m_isBlacksTurn;
}

bool GameLogic::saveToFile(const std::string& filePath, const SaveInfo& info, float timeBlack, float timeWhite) const
{
    std::ofstream file(filePath);
    if (!file.is_open()) return false;

    file << info.userTitle << "|" << info.timestamp << "|" << info.boardSize << "|"
         << info.modeStr << "|" << info.status << "\n";

    file << m_boardSize << " " << (m_isBlacksTurn ? 1 : 0) << " "
         << m_koPosition.first << " " << m_koPosition.second << " "
         << (m_lastPlayerPassed ? 1 : 0) << " "
         << timeBlack << " " << timeWhite << "\n";

    for (const auto& row : m_board)
    {
        for (const auto& stone : row)
        {
            int val = 0;
            if (stone == StoneType::Black) val = 1;
            else if (stone == StoneType::White) val = 2;
            file << val << " ";
        }
        file << "\n";
    }
    file.close();
    return true;
}

bool GameLogic::loadFromFile(const std::string& filePath, float& timeBlack, float& timeWhite)
{
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    std::string headerLine;
    std::getline(file, headerLine);

    int size, turn, koX, koY, pass;

    if (!(file >> size >> turn >> koX >> koY >> pass >> timeBlack >> timeWhite))
    {
        return false;
    }

    if (size != m_boardSize)
    {
        m_boardSize = size;
        m_board.assign(size, std::vector<StoneType>(size, StoneType::Empty));
        m_previousBoard.assign(size, std::vector<StoneType>(size, StoneType::Empty));
    }

    m_isBlacksTurn = (turn == 1);
    m_koPosition = {koX, koY};
    m_lastPlayerPassed = (pass == 1);

    for (int y = 0; y < m_boardSize; ++y)
    {
        for (int x = 0; x < m_boardSize; ++x)
        {
            int val;
            file >> val;
            if (val == 1) m_board[y][x] = StoneType::Black;
            else if (val == 2) m_board[y][x] = StoneType::White;
            else m_board[y][x] = StoneType::Empty;
        }
    }

    while(!m_undoStack.empty()) m_undoStack.pop();
    while(!m_redoStack.empty()) m_redoStack.pop();

    return true;
}

const std::vector<std::vector<StoneType>>& GameLogic::getBoard() const
{
    return m_board;
}

std::vector<std::vector<TerritoryOwner>> GameLogic::calculateTerritory()
{
    std::vector<std::vector<TerritoryOwner>> territory(m_boardSize, std::vector<TerritoryOwner>(m_boardSize, TerritoryOwner::None));

    std::vector<std::vector<bool>> visited(m_boardSize, std::vector<bool>(m_boardSize, false));

    for (int y = 0; y < m_boardSize; ++y)
    {
        for (int x = 0; x < m_boardSize; ++x)
        {

            if (m_board[y][x] == StoneType::Empty && !visited[y][x])
            {
                std::vector<std::pair<int, int>> region;
                std::queue<std::pair<int, int>> q;

                q.push({x, y});
                visited[y][x] = true;

                bool touchBlack = false;
                bool touchWhite = false;

                while(!q.empty()){
                    std::pair<int, int> curr = q.front();
                    q.pop();
                    region.push_back(curr);

                    for (int i = 0; i < 4; ++i)
                    {
                        int nx = curr.first + DX[i];
                        int ny = curr.second + DY[i];

                        if (nx < 0 || nx >= m_boardSize || ny < 0 || ny >= m_boardSize) continue;

                        if (m_board[ny][nx] == StoneType::Black)
                        {
                            touchBlack = true;
                        }
                        else if (m_board[ny][nx] == StoneType::White)
                        {
                            touchWhite = true;
                        }
                        else if (m_board[ny][nx] == StoneType::Empty && !visited[ny][nx])
                        {
                            visited[ny][nx] = true;
                            q.push({nx, ny});
                        }
                    }
                }

                TerritoryOwner owner = TerritoryOwner::Neutral;

                if (touchBlack && !touchWhite)
                {
                    owner = TerritoryOwner::Black;
                }
                else if (!touchBlack && touchWhite)
                {
                    owner = TerritoryOwner::White;
                }

                for (auto& p : region)
                {
                    territory[p.second][p.first] = owner;
                }
            }
        }
    }
    return territory;
}
