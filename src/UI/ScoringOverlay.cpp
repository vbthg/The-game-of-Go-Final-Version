#include "ScoringOverlay.h"
#include <queue>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>

const float WAVE_SPEED_FACTOR = 0.25f;
const float DELAY_BETWEEN_SIDES = 0.8f;
const float DELAY_BEFORE_SCOREBOARD = 3.0f;
const float CIRCLE_SIZE_RATIO = 0.45f;
const float TARGET_ALPHA = 100.f;
const float FADE_IN_DURATION = 0.6f;
const float RANDOM_DELAY_AMPLITUDE = 0.05f;



ScoringOverlay::ScoringOverlay(int boardSize, float cellSize, sf::Vector2f boardTopLeft, const sf::Font& font)
    : m_boardSize(boardSize), m_cellSize(cellSize), m_boardTopLeft(boardTopLeft),
      m_dimmerAlpha(0.f), m_timer(0.f), m_font(font)
{
    m_showScoreboard = false;

    m_gridEffects.resize(m_boardSize, std::vector<TileEffect>(m_boardSize));

    float radius = (m_cellSize * CIRCLE_SIZE_RATIO) / 2.f;

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            m_gridEffects[y][x].shape.setRadius(radius);
            m_gridEffects[y][x].shape.setOrigin(sf::Vector2f(std::round(radius), std::round(radius)));
            m_gridEffects[y][x].shape.setPosition
            (
                std::round(m_boardTopLeft.x + x*m_cellSize),
                std::round(m_boardTopLeft.y + y*m_cellSize)
            );
        }
    }


    m_scorePanel.setSize({700.f, 500.f});
    m_scorePanel.setFillColor(sf::Color(40, 40, 40, 235));
    m_scorePanel.setOutlineColor(sf::Color(200, 200, 200));
    m_scorePanel.setOutlineThickness(2.f);


    m_txtTitle.setFont(m_font);
    m_txtTitle.setCharacterSize(36);
    m_txtTitle.setFillColor(sf::Color::Yellow);
    m_txtTitle.setString("GAME RESULTS");


    m_txtResult.setFont(m_font);
    m_txtResult.setCharacterSize(32);
    m_txtResult.setStyle(sf::Text::Bold);


    m_txtInstruction.setFont(m_font);
    m_txtInstruction.setCharacterSize(16);
    m_txtInstruction.setFillColor(sf::Color(180, 180, 180));
    m_txtInstruction.setString("( Click inside this board to return to Menu )");


    m_dimmer.setSize({2000.f, 2000.f});
    m_dimmer.setFillColor(sf::Color(0,0,0,0));
}


bool ScoringOverlay::contains(sf::Vector2f point) const
{
    if(!m_showScoreboard) return false;
    return m_scorePanel.getGlobalBounds().contains(point);
}


void ScoringOverlay::addLine(float x, float y, float w, float h)
{
    sf::RectangleShape line({w, h});
    line.setFillColor(sf::Color(150, 150, 150));
    line.setPosition(x, y);
    m_tableLines.push_back(line);
}


void ScoringOverlay::addCellText(const std::string& str, float x, float y, bool isHeader, sf::Color color)
{
    sf::Text text;
    text.setFont(m_font);
    text.setString(str);
    text.setCharacterSize(isHeader ? 22 : 20);
    text.setFillColor(color);
    if(isHeader) text.setStyle(sf::Text::Bold);


    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
    text.setPosition(x, y);

    m_tableTexts.push_back(text);
}

void ScoringOverlay::startAnimation(const std::vector<std::vector<TerritoryOwner>>& territoryMap,
                                    const std::vector<std::vector<StoneType>>& boardState)
{
    m_currentPhase = Phase::BlackSpreading;
    m_timer = 0.f;
    m_maxBlackDelay = 0.f;
    m_maxWhiteDelay = 0.f;


    std::queue<std::pair<int, int>> q;
    std::vector<std::vector<int>> distance(m_boardSize, std::vector<int>(m_boardSize, -1));

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            if(boardState[y][x] != StoneType::Empty)
            {
                q.push({x, y});
                distance[y][x] = 0;
            }
        }
    }

    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};

    while(!q.empty())
    {
        auto curr = q.front();
        q.pop();

        for(int i = 0; i < 4; ++i)
        {
            int nx = curr.first + dx[i]; int ny = curr.second + dy[i];
            if(nx < 0 || nx >= m_boardSize || ny < 0 || ny >= m_boardSize) continue;

            if(distance[ny][nx] == -1 && territoryMap[ny][nx] != TerritoryOwner::None)
            {
                distance[ny][nx] = distance[curr.second][curr.first] + 1;
                q.push({nx, ny});
            }
        }
    }


    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            TileEffect& fx = m_gridEffects[y][x];
            fx.owner = territoryMap[y][x];
            fx.alpha = 0.f;
            fx.active = false;

            int dist = (distance[y][x] == -1) ? 0 : distance[y][x];
            float calculatedDelay = dist * WAVE_SPEED_FACTOR;

            if(fx.owner == TerritoryOwner::Black)
            {
                fx.shape.setFillColor(sf::Color::Black);
                fx.targetAlpha = TARGET_ALPHA;
                fx.delay = calculatedDelay;
                fx.active = true;
                m_maxBlackDelay = std::max(m_maxBlackDelay, calculatedDelay);
            }

            else if(fx.owner == TerritoryOwner::White)
            {
                fx.shape.setFillColor(sf::Color::White);
                fx.targetAlpha = TARGET_ALPHA;
                fx.delay = calculatedDelay;
                fx.active = true;
                m_maxWhiteDelay = std::max(m_maxWhiteDelay, calculatedDelay);
            }

            else
            {
                fx.active = false;
            }
        }
    }
}


void ScoringOverlay::setScoreData(const ScoreData& data)
{
    m_tableLines.clear();
    m_tableTexts.clear();


    float totalBlack = data.blackStones + data.blackTerritory;
    float totalWhite = data.whiteStones + data.whiteTerritory + data.komi;
    float diff = std::abs(totalBlack - totalWhite);


    std::stringstream ssRes;
    if(totalBlack > totalWhite)
    {
        ssRes << "BLACK WINS! (+" << std::fixed << std::setprecision(1) << diff << ")";
        m_txtResult.setFillColor(sf::Color(100, 255, 100));
    }
    else
    {
        ssRes << "WHITE WINS! (+" << std::fixed << std::setprecision(1) << diff << ")";
        m_txtResult.setFillColor(sf::Color(100, 255, 100));
    }
    m_txtResult.setString(ssRes.str());


    float startY = -80.f;
    float rowH = 40.f;
    float col1W = 180.f;
    float col2W = 150.f;
    float col3W = 150.f;
    float tableW = col1W + col2W + col3W;

    float startX = -tableW / 2.f;


    addCellText("CATEGORY", startX + col1W / 2, startY + rowH / 2, true, sf::Color(255, 200, 0));
    addCellText("BLACK",    startX + col1W + col2W / 2, startY + rowH / 2, true);
    addCellText("WHITE",    startX + col1W + col2W + col3W / 2, startY + rowH / 2, true);


    addLine(startX, startY + rowH, tableW, 2.f);


    float y1 = startY + rowH;
    addCellText("Territory", startX + col1W / 2, y1 + rowH / 2);
    addCellText(std::to_string(data.blackTerritory), startX + col1W + col2W / 2, y1 + rowH / 2);
    addCellText(std::to_string(data.whiteTerritory), startX + col1W + col2W + col3W / 2, y1 + rowH / 2);
    addLine(startX, y1 + rowH, tableW, 1.f);


    float y2 = y1 + rowH;
    addCellText("Stones", startX + col1W / 2, y2 + rowH / 2);
    addCellText(std::to_string(data.blackStones), startX + col1W + col2W / 2, y2 + rowH / 2);
    addCellText(std::to_string(data.whiteStones), startX + col1W + col2W + col3W / 2, y2 + rowH / 2);
    addLine(startX, y2 + rowH, tableW, 1.f);


    float y3 = y2 + rowH;
    addCellText("Komi", startX + col1W / 2, y3 + rowH / 2);
    addCellText("0.0", startX + col1W + col2W / 2, y3 + rowH / 2);

    std::stringstream ssKomi; ssKomi << std::fixed << std::setprecision(1) << data.komi;
    addCellText(ssKomi.str(), startX + col1W + col2W + col3W / 2, y3 + rowH / 2);
    addLine(startX, y3 + rowH, tableW, 2.f);


    float y4 = y3 + rowH;
    addCellText("TOTAL", startX + col1W / 2, y4 + rowH / 2, true, sf::Color::Cyan);

    std::stringstream ssTotalB; ssTotalB << std::fixed << std::setprecision(1) << totalBlack;
    addCellText(ssTotalB.str(), startX + col1W + col2W / 2, y4 + rowH / 2, true, sf::Color::Cyan);

    std::stringstream ssTotalW; ssTotalW << std::fixed << std::setprecision(1) << totalWhite;
    addCellText(ssTotalW.str(), startX + col1W + col2W + col3W / 2, y4 + rowH / 2, true, sf::Color::Cyan);


    float totalH = rowH * 5;
    addLine(startX + col1W, startY, 1.f, totalH);
    addLine(startX + col1W + col2W, startY, 1.f, totalH);
}

void ScoringOverlay::update(float deltaTime)
{
    if(m_currentPhase == Phase::Finished) return;

    m_timer += deltaTime;


    if(m_currentPhase == Phase::BlackSpreading)
    {
        for(auto &row : m_gridEffects)
        {
            for(auto &fx : row)
            {
                if(fx.active && fx.owner == TerritoryOwner::Black)
                {
                    if(m_timer >= fx.delay)
                    {
                        if(fx.alpha < fx.targetAlpha)
                        {
                            fx.alpha += deltaTime * 400.f;
                            if(fx.alpha > fx.targetAlpha) fx.alpha = fx.targetAlpha;
                        }

                        sf::Color c = fx.shape.getFillColor();
                        c.a = (sf::Uint8)fx.alpha;
                        fx.shape.setFillColor(c);
                    }
                }
            }
        }

        if(m_timer > m_maxBlackDelay + 0.5f)
        {
            m_currentPhase = Phase::Intermission;
            m_timer = 0.f;
        }
    }

    else if(m_currentPhase == Phase::Intermission)
    {
        if(m_timer > DELAY_BETWEEN_SIDES)
        {
            m_currentPhase = Phase::WhiteSpreading;
            m_timer = 0.f;
        }
    }

    else if(m_currentPhase == Phase::WhiteSpreading)
    {
        for(auto &row : m_gridEffects)
        {
            for(auto &fx : row)
            {
                if(fx.active && fx.owner == TerritoryOwner::White)
                {
                    if(m_timer >= fx.delay)
                    {
                        if(fx.alpha < fx.targetAlpha)
                        {
                            fx.alpha += deltaTime * 400.f;
                            if(fx.alpha > fx.targetAlpha) fx.alpha = fx.targetAlpha;
                        }
                        sf::Color c = fx.shape.getFillColor();
                        c.a = (sf::Uint8)fx.alpha;
                        fx.shape.setFillColor(c);
                    }
                }
            }
        }
        if(m_timer > m_maxWhiteDelay + 0.5f)
        {
            m_currentPhase = Phase::FinalDelay;
            m_timer = 0.f;
        }
    }
    else if(m_currentPhase == Phase::FinalDelay)
    {
        if(m_timer > DELAY_BEFORE_SCOREBOARD)
        {
            m_currentPhase = Phase::Finished;
            m_showScoreboard = true;
        }
    }
}
void ScoringOverlay::draw(sf::RenderTarget& target)
{
    for(const auto &row : m_gridEffects)
    {
        for(const auto &fx : row)
        {
            if(fx.active && fx.alpha > 0.f)
            {
                target.draw(fx.shape);
            }
        }
    }


    if(m_showScoreboard)
    {
        sf::Vector2u winSize = target.getSize();
        sf::Vector2f center(winSize.x / 2.f, winSize.y / 2.f);


        m_scorePanel.setOrigin(350.f, 250.f);
        m_scorePanel.setPosition(center);
        target.draw(m_scorePanel);


        sf::FloatRect titleB = m_txtTitle.getLocalBounds();
        m_txtTitle.setOrigin(titleB.left + titleB.width / 2.f, titleB.top + titleB.height / 2.f);
        m_txtTitle.setPosition(center.x, center.y - 200.f);
        target.draw(m_txtTitle);


        sf::FloatRect resB = m_txtResult.getLocalBounds();
        m_txtResult.setOrigin(resB.left + resB.width / 2.f, resB.top + resB.height / 2.f);
        m_txtResult.setPosition(center.x, center.y + 160.f);
        target.draw(m_txtResult);


        sf::FloatRect insB = m_txtInstruction.getLocalBounds();
        m_txtInstruction.setOrigin(insB.left + insB.width / 2.f, insB.top + insB.height / 2.f);
        m_txtInstruction.setPosition(center.x, center.y + 220.f);
        target.draw(m_txtInstruction);


        sf::Transform transform;
        transform.translate(center);
        for (const auto &line : m_tableLines) target.draw(line, transform);
        for (const auto &text : m_tableTexts) target.draw(text, transform);
    }
}

bool ScoringOverlay::isAnimationFinished() const
{
    return m_currentPhase == Phase::Finished;
}
bool ScoringOverlay::isScoreboardVisible() const { return m_showScoreboard; }
