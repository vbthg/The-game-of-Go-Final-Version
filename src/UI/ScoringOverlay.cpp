#include "ScoringOverlay.h"
#include "ResourceManager.h"
#include "GlobalSetting.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

namespace UI
{
const float EDGE_GROW_DURATION = 0.20f;
const float BORDER_OFFSET_RATIO = 0.5f;
const float DELAY_BETWEEN_SIDES = 0.25f;
const float BORDER_ALPHA = 180.f;

const float DEAD_STONE_DURATION = 1.5f;
const float DEAD_STONE_DELAY = 1.0f;
const sf::Uint8 FINAL_ALPHA = 80;
const float FINAL_SCALE = 0.75f;
const float PI = 3.14159265359f;

const int dy[] = { -1, +0, +0, +1 };
const int dx[] = { +0, +1, -1, +0 };

ScoringOverlay::ScoringOverlay(int boardSize, float cellSize, sf::Vector2f boardTopLeft, const sf::Font& font)
    : m_boardSize(boardSize)
    , m_cellSize(cellSize)
    , m_boardTopLeft(boardTopLeft)
    , m_font(font)
    , m_texGradientLine(ResourceManager::getInstance().getTexture("territory_line"))
    , m_currentPhase(Phase::Idle)
    , m_timer(0.f)
    , m_animTimer(0.f)
    , m_soundTimer(0.f)
{
    auto& gs = GlobalSetting::getInstance();
    auto& rm = ResourceManager::getInstance();

    m_boardSprite.setTexture(rm.getTexture("scoreboard"));
    sf::FloatRect b = m_boardSprite.getLocalBounds();
    m_boardSprite.setOrigin(b.width / 2.f, b.height / 2.f);
    m_boardSprite.setColor(sf::Color(255, 255, 255, 0));

    m_stampSprite.setColor(sf::Color(255, 255, 255, 0));

    std::string blackKey = gs.getStoneTextureKey(true, 13);
    std::string whiteKey = gs.getStoneTextureKey(false, 13);

    m_iconBlackStone.setTexture(rm.getTexture(blackKey));
    m_iconWhiteStone.setTexture(rm.getTexture(whiteKey));

    sf::FloatRect bounds = m_iconBlackStone.getLocalBounds();
    m_iconBlackStone.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    bounds = m_iconWhiteStone.getLocalBounds();
    m_iconWhiteStone.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    for(int i = 0; i < 4; ++i)
    {
        m_txtScoreBlack[i].setFont(m_font);
        m_txtScoreBlack[i].setCharacterSize(28);
        m_txtScoreBlack[i].setFillColor(sf::Color::Black);

        m_txtScoreWhite[i].setFont(m_font);
        m_txtScoreWhite[i].setCharacterSize(28);
        m_txtScoreWhite[i].setFillColor(sf::Color::White);

        m_curScoreBlack[i] = 0.f;
        m_curScoreWhite[i] = 0.f;
    }

    m_soundTick.setBuffer(rm.getSoundBuffer("count_tick"));
    m_soundStamp.setBuffer(rm.getSoundBuffer("stamp_impact"));

    float vol = GlobalSetting::getInstance().sfxVolume;
    m_soundTick.setVolume(vol);
    m_soundStamp.setVolume(vol);
}

void ScoringOverlay::setScoreData(const ScoreData& data)
{
    m_data = data;
    auto& rm = ResourceManager::getInstance();

    for(int i = 0; i < 4; ++i)
    {
        m_curScoreBlack[i] = 0.f;
        m_curScoreWhite[i] = 0.f;
    }

    float totalB = data.blackStones + data.blackTerritory;
    float totalW = data.whiteStones + data.whiteTerritory + data.komi;

    if(totalB > totalW)
    {
        m_stampSprite.setTexture(rm.getTexture("stamp_black_wins"));
    }
    else
    {
        m_stampSprite.setTexture(rm.getTexture("stamp_white_wins"));
    }

    sf::FloatRect sb = m_stampSprite.getLocalBounds();
    m_stampSprite.setOrigin(sb.width / 2.f, sb.height / 2.f);

    double angle = rand() % 40 - 10;
    m_stampSprite.rotate(angle);

    float diff = std::abs(totalB - totalW);
    m_countDuration = std::max(1.0f, std::min(diff * 0.05f, 2.5f));
}

void ScoringOverlay::showSimpleResult(const std::string& message, bool blackWon)
{
    m_timer = 0.f;
    m_animTimer = 0.f;
    m_isSimpleMode = true;

    auto& rm = ResourceManager::getInstance();
    m_boardSprite.setTexture(rm.getTexture("game_result_board"), true);

    sf::FloatRect b = m_boardSprite.getLocalBounds();
    m_boardSprite.setOrigin(b.width / 2.f, b.height / 2.f);

    m_simpleResultText.setFont(m_font);
    m_simpleResultText.setString(message);
    m_simpleResultText.setCharacterSize(30);
    m_simpleResultText.setFillColor(sf::Color::Black);
    m_simpleResultText.setStyle(sf::Text::Bold);

    sf::FloatRect tb = m_simpleResultText.getLocalBounds();
    m_simpleResultText.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);

    if(blackWon) m_stampSprite.setTexture(rm.getTexture("stamp_black_wins"));
    else m_stampSprite.setTexture(rm.getTexture("stamp_white_wins"));

    sf::FloatRect sb = m_stampSprite.getLocalBounds();
    m_stampSprite.setOrigin(sb.width / 2.f, sb.height / 2.f);

    double angle = rand() % 40 - 10;
    m_stampSprite.rotate(angle);

    m_currentPhase = Phase::BoardAppear;
    m_boardSprite.setScale(2.0f, 2.0f);
    m_boardSprite.setColor(sf::Color(255, 255, 255, 0));
}

void ScoringOverlay::update(float deltaTime)
{
    if(m_currentPhase == Phase::Finished || m_currentPhase == Phase::Idle) return;

    m_timer += deltaTime;

    if(m_currentPhase == Phase::DeadStoneAnim)
    {
        if(m_timer >= DEAD_STONE_DURATION)
        {
            m_currentPhase = Phase::DeadStoneDelay;
            m_timer = 0.f;
        }
    }
    else if(m_currentPhase == Phase::DeadStoneDelay)
    {
        if(m_timer >= DEAD_STONE_DELAY)
        {
            m_currentPhase = Phase::BlackWave;
            m_timer = 0.f;
        }
    }
    else if(m_currentPhase == Phase::BlackWave)
    {
        bool done = updatePhaseLogic(deltaTime, TerritoryOwner::Black, m_maxDistBlack);
        if(done)
        {
            m_currentPhase = Phase::Intermission;
            m_timer = 0.f;
        }
    }
    else if(m_currentPhase == Phase::Intermission)
    {
        if(m_timer > DELAY_BETWEEN_SIDES)
        {
            m_currentPhase = Phase::WhiteWave;
            m_timer = 0.f;
        }
    }
    else if(m_currentPhase == Phase::WhiteWave)
    {
        bool done = updatePhaseLogic(deltaTime, TerritoryOwner::White, m_maxDistWhite);
        if(done)
        {
            m_currentPhase = Phase::FinalDelay;
            m_timer = 0.f;
        }
    }
    else if(m_currentPhase == Phase::FinalDelay)
    {
        float fadeDuration = 2.0f;

        float progress = m_timer / fadeDuration;

        updateDisappear(progress);

        if(progress >= 1.0f)
        {
            updateDisappear(1.0f);

            m_currentPhase = Phase::BoardAppear;
            m_animTimer = 0.f;

            m_boardSprite.setScale(2.0f, 2.0f);
            m_boardSprite.setColor(sf::Color(255, 255, 255, 0));
        }
    }

    else if(m_currentPhase == Phase::BoardAppear)
    {
        m_animTimer += deltaTime;
        updateBoardAppear(deltaTime);
    }
    else if(m_currentPhase == Phase::Counting)
    {
        if(m_isSimpleMode)
        {
            m_currentPhase = Phase::StampAppear;
            m_animTimer = 0.f;
            m_soundStamp.setVolume(GlobalSetting::getInstance().sfxVolume);
            m_soundStamp.play();
        }
        else
        {
            m_animTimer += deltaTime;
            updateCounting(deltaTime);
        }
    }
    else if(m_currentPhase == Phase::StampAppear)
    {
        m_animTimer += deltaTime;
        updateStampAppear(deltaTime);
    }
}

float ScoringOverlay::easeOutBack(float x)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.f;
    return 1.f + c3 * std::pow(x - 1.f, 3.f) + c1 * std::pow(x - 1.f, 2.f);
}

void ScoringOverlay::updateBoardAppear(float dt)
{
    float duration = 0.5f;
    float progress = m_animTimer / duration;

    if(progress >= 1.f)
    {
        m_boardSprite.setScale(1.f, 1.f);
        m_boardSprite.setColor(sf::Color::White);

        m_currentPhase = Phase::Counting;
        m_animTimer = 0.f;
        return;
    }

    float scale = 2.0f - 1.0f * easeOutBack(progress);
    m_boardSprite.setScale(scale, scale);

    float alpha = std::min(255.f, progress * 4.f * 255.f);
    m_boardSprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
}

void ScoringOverlay::updateCounting(float dt)
{
    float progress = m_animTimer / m_countDuration;
    if(progress >= 1.f) progress = 1.f;

    float ratio = 1.f - std::pow(1.f - progress, 3.f);

    m_curScoreBlack[0] = m_data.blackStones * ratio;
    m_curScoreBlack[1] = m_data.blackTerritory * ratio;
    m_curScoreBlack[2] = 0.f * ratio;
    m_curScoreBlack[3] = (m_data.blackStones + m_data.blackTerritory) * ratio;

    m_curScoreWhite[0] = m_data.whiteStones * ratio;
    m_curScoreWhite[1] = m_data.whiteTerritory * ratio;
    m_curScoreWhite[2] = m_data.komi * ratio;
    m_curScoreWhite[3] = (m_data.whiteStones + m_data.whiteTerritory + m_data.komi) * ratio;

    m_soundTimer += dt;
    if(progress < 1.f && m_soundTimer > 0.08f)
    {
        m_soundTimer = 0.f;

        m_soundTick.setPitch(0.95f + (std::rand() % 10) / 100.f);
        m_soundTick.play();
    }

    if(progress >= 1.f)
    {
        m_currentPhase = Phase::StampAppear;
        m_animTimer = 0.f;

        m_soundStamp.setVolume(GlobalSetting::getInstance().sfxVolume);
        m_soundStamp.play();
    }
}

void ScoringOverlay::updateStampAppear(float dt)
{
    float duration = 0.4f;
    float progress = m_animTimer / duration;

    if(progress >= 1.f)
    {
        m_stampSprite.setScale(1.f, 1.f);
        m_stampSprite.setColor(sf::Color::White);
        m_currentPhase = Phase::Finished;
        return;
    }

    float scale = 3.0f - 2.0f * easeOutBack(progress);
    m_stampSprite.setScale(scale, scale);

    float alpha = std::min(255.f, progress * 5.f * 255.f);
    m_stampSprite.setColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
}

void ScoringOverlay::updateDisappear(float progress)
{
    float coef = std::sin(progress * PI / 2.f);
    float newAlpha = BORDER_ALPHA * (1.f - coef);
    if(newAlpha < 0.f) newAlpha = 0.f;

    sf::Color cBlack(0, 0, 0, (sf::Uint8)newAlpha);
    sf::Color cWhite(255, 255, 255, (sf::Uint8)newAlpha);

    for(auto& comps : m_ConnectedComponents)
    {
        for(auto& edge : comps.activeEdges)
        {
            if(comps.owner == TerritoryOwner::Black) edge.sprite.setColor(cBlack);
            else edge.sprite.setColor(cWhite);
        }
    }
}

void ScoringOverlay::draw(sf::RenderTarget& target)
{
    renderDeadStones(target);

    if(m_currentPhase != Phase::BoardAppear &&
       m_currentPhase != Phase::Counting &&
       m_currentPhase != Phase::StampAppear &&
       m_currentPhase != Phase::Finished)
    {
//        std::cout << "draw did draw\n";
        for(const auto& comps : m_ConnectedComponents)
        {
            if(!comps.isActive) continue;
            for(const auto& edge : comps.activeEdges)
            {
                if(edge.sprite.getScale().y > 0.001f)
                {
                    target.draw(edge.sprite);
                }
            }
        }
    }

    if(m_currentPhase == Phase::BoardAppear ||
       m_currentPhase == Phase::Counting ||
       m_currentPhase == Phase::StampAppear ||
       m_currentPhase == Phase::Finished)
    {
        sf::Vector2u winSize = target.getSize();
        sf::Vector2f center(winSize.x / 2.f, winSize.y / 2.f);

        if(m_isSimpleMode) m_boardSprite.setPosition(sf::Vector2f(1387.f, 462.f + 80.f));
        else m_boardSprite.setPosition(sf::Vector2f(1387.f, 462.f));
        target.draw(m_boardSprite);

        if(m_currentPhase != Phase::BoardAppear)
        {
            if(m_isSimpleMode)
            {
                m_simpleResultText.setPosition(1387.f, 460 + 80.f);
                target.draw(m_simpleResultText);
            }

            else
            {
                auto drawText = [&](sf::Text& txt, float val, float x, float y)
                {
                    std::stringstream ss;
                    ss << std::fixed << std::setprecision(1) << val;
                    txt.setString(ss.str());

                    sf::FloatRect b = txt.getLocalBounds();
                    txt.setOrigin(b.width / 2.f, b.height / 2.f);
                    txt.setPosition(x, y);
                    target.draw(txt);
                };

                const float leftX = 1387.f;
                const float disX = 103.f;
                const float topY = 405.f;
                const float disY = 58.f;

                m_iconBlackStone.setPosition(leftX, 336.f);
                m_iconWhiteStone.setPosition(leftX + disX, 336.f);

                target.draw(m_iconBlackStone);
                target.draw(m_iconWhiteStone);

                drawText(m_txtScoreBlack[0], m_curScoreBlack[0], leftX, topY);
                drawText(m_txtScoreBlack[1], m_curScoreBlack[1], leftX, topY + disY);
                drawText(m_txtScoreBlack[2], m_curScoreBlack[2], leftX, topY + 2 * disY);
                drawText(m_txtScoreBlack[3], m_curScoreBlack[3], leftX, topY + 3 * disY);

                drawText(m_txtScoreWhite[0], m_curScoreWhite[0], leftX + disX, topY);
                drawText(m_txtScoreWhite[1], m_curScoreWhite[1], leftX + disX, topY + disY);
                drawText(m_txtScoreWhite[2], m_curScoreWhite[2], leftX + disX, topY + 2 * disY);
                drawText(m_txtScoreWhite[3], m_curScoreWhite[3], leftX + disX, topY + 3 * disY);
            }
        }

        if(m_currentPhase == Phase::StampAppear || m_currentPhase == Phase::Finished)
        {
            if(m_isSimpleMode) m_stampSprite.setPosition(sf::Vector2f(1238.f, 380.f + 80.f));
            else m_stampSprite.setPosition(sf::Vector2f(1285.f, 330.f));
            m_stampSprite.setColor(sf::Color::Red);
            target.draw(m_stampSprite);
        }
    }
}

void ScoringOverlay::startAnimation(const std::vector<TerritoryRegion>& regions, const std::vector<DeadStoneInfo>& deadStones)
{
    std::vector < std::vector <bool> > visited(m_boardSize + 1, std::vector <bool> (m_boardSize + 1, false));
    m_adjNodes.clear();
    m_adjNodes.assign(m_boardSize + 1, std::vector<std::vector<Node>>(m_boardSize + 1));

    m_ConnectedComponents.clear();
    m_timer = 0.f;
    m_deadStones = deadStones;
    m_maxDistBlack = 0.f;
    m_maxDistWhite = 0.f;

    auto PixelDistance = [*this](sf::Vector2i pivot, sf::Vector2i p) -> int
    {
        float x = std::abs(pivot.x - p.x) * m_cellSize;
        float y = std::abs(pivot.y - p.y) * m_cellSize;
        x = x * x;
        y = y * y;
        return std::sqrt(x + y);
    };

    for(const auto &r : regions)
    {
        if(r.owner != TerritoryOwner::Black && r.owner != TerritoryOwner::White)
        {
            continue;
        }
        for(auto &p : r.points)
        {
            int y = p.y, x = p.x;

            for(int dir = 0; dir < 4; ++dir)
            {
                int ny = y + dy[dir], nx = x + dx[dir];

                bool found = false;
                for(auto &q : r.points) if(q.y == ny && q.x == nx)
                {
                    found = true;
                    break;
                }

                if(!found)
                {
                    sf::Vector2i u = sf::Vector2i(x, y);
                    sf::Vector2i v = sf::Vector2i(x + 1, y + 1);

                    if(!(dir & 1)) v.x += dx[dir], v.y += dy[dir];
                    else u.x += dx[dir], u.y += dy[dir];

                    m_adjNodes[u.y][u.x].push_back(Node(v, sf::Vector2i(-dx[dir], -dy[dir]), m_adjNodes[v.y][v.x].size()));
                    m_adjNodes[v.y][v.x].push_back(Node(u, sf::Vector2i(-dx[dir], -dy[dir]), m_adjNodes[u.y][u.x].size() - 1));

//                    std::cout << "edge found!: " << u.y << " " << u.x << " -> " << v.y << " " << v.x << "\n";
                }
            }
        }

        m_ConnectedComponents.emplace_back();
        auto &curComp = m_ConnectedComponents.back();

        for(auto &p : r.points) if(!visited[p.y][p.x])
        {
            std::deque <sf::Vector2i> que; que.clear();
            que.push_back(p);

            while(!que.empty())
            {
                sf::Vector2i u = que.front();
                que.pop_front();

                curComp.allPoints.push_back(u);

                for(auto &adj : m_adjNodes[u.y][u.x]) if(!visited[adj.v.y][adj.v.x])
                {
                    visited[adj.v.y][adj.v.x] = true;
                    que.push_back(adj.v);
                }
            }
        }

        curComp.owner = r.owner;

        sf::Vector2i pivot;
        if(curComp.owner == TerritoryOwner::Black) pivot = sf::Vector2i(0, 0);
        else pivot = sf::Vector2i(m_boardSize, m_boardSize);

        bool hasPoint = false;

        for(auto &p : curComp.allPoints)
        {
            hasPoint = true;

            if(curComp.anchorDistance > PixelDistance(pivot, p))
            {
                curComp.anchorDistance = PixelDistance(pivot, p);
                curComp.anchorPoint = p;

//                std::cout << "anchor Point candidate: " << p.y << " " << p.x << "\n";
            }
        }

        if(!hasPoint)
        {
            curComp.isFinished = true;
            curComp.isActive = true;
            curComp.anchorDistance = 0.f;
        }

        if(curComp.owner == TerritoryOwner::Black)
            m_maxDistBlack = std::max(m_maxDistBlack, curComp.anchorDistance + 100.f);
        else
            m_maxDistWhite = std::max(m_maxDistWhite, curComp.anchorDistance + 100.f);
    }

    std::sort(m_ConnectedComponents.begin(), m_ConnectedComponents.end(),
    [](const auto& a, const auto& b)
    {
        return a.anchorDistance < b.anchorDistance;
    }
    );



//    for(auto &comps : m_ConnectedComponents)
//    {
//        std::cout << (comps.owner == TerritoryOwner::Black ? "Belong to Black:\n" : "Belong to White:\n");
//        std::cout << "anchor Point: " << comps.anchorPoint.y << " " << comps.anchorPoint.x << "\n";
//        std::cout << "list of points:\n";
//        for(auto &p : comps.allPoints) std::cout << p.y << " " << p.x << "\n";
//        std::cout << "\n";
//    }


    if(!m_deadStones.empty()) m_currentPhase = Phase::DeadStoneAnim;
    else m_currentPhase = Phase::BlackWave;
}

void ScoringOverlay::spawnEdge(VirtualConnectedComponent& curComp, const sf::Vector2i &parent)
{
    auto RadToDeg = [*this](const float a) -> float
    {
        return a * 180.f / PI;
    };

    auto DegToRad = [*this](const float a) -> float
    {
        return a * PI / 180.f;
    };

    auto BoardToPixelCoord = [*this](sf::Vector2i p) -> sf::Vector2f
    {
        return sf::Vector2f(std::round(m_boardTopLeft.x + p.x * m_cellSize - m_cellSize * BORDER_OFFSET_RATIO),
                            std::round(m_boardTopLeft.y + p.y * m_cellSize - m_cellSize * BORDER_OFFSET_RATIO));
    };

    for(auto &adj : m_adjNodes[parent.y][parent.x])
    {
        int y = adj.v.y, x = adj.v.x;
        sf::Vector2i dir = adj.dir;

        if(y < 0 || y > m_boardSize || x < 0 || x > m_boardSize) continue;

        if(adj.isDrawn) continue;
        adj.isDrawn = true;
        m_adjNodes[y][x][adj.ind].isDrawn = true;

        int dy = y - parent.y, dx = x - parent.x;

        ActiveEdge edge;
        edge.sprite.setTexture(m_texGradientLine);

        if(curComp.owner == TerritoryOwner::Black) edge.sprite.setColor(sf::Color(0, 0, 0, BORDER_ALPHA));
        else edge.sprite.setColor(sf::Color(255, 255, 255, BORDER_ALPHA));

        sf::FloatRect bounds = edge.sprite.getLocalBounds();
        edge.targetScaleY = (m_cellSize + 0.3f) / bounds.height;

        float angleByOx = RadToDeg(std::atan2(-dy, dx));
        float angleByOriginal = 270.f - angleByOx;

        edge.sprite.rotate(angleByOriginal);

        sf::Vector2i graDirection = sf::Vector2i(1, 0);
        graDirection = sf::Vector2i(graDirection.x * std::cos(DegToRad(angleByOriginal)) + graDirection.y * std::sin(DegToRad(angleByOriginal)),
                                    -graDirection.x * std::sin(DegToRad(angleByOriginal)) + graDirection.y * std::cos(DegToRad(angleByOriginal)));

        graDirection.y = -graDirection.y;

        float scaleX = m_cellSize / bounds.width;

        if(graDirection != dir) scaleX = -scaleX;

        edge.sprite.setScale(sf::Vector2f(scaleX, 0.f));

        edge.sprite.setPosition(BoardToPixelCoord(parent));

        edge.endNode = sf::Vector2i(x, y);

//        std::cout << "New Edge Spawned!: " << parent.y << " " << parent.x << " -> " << y << " " << x << "\n";

        curComp.activeEdges.push_back(edge);
    }
}

bool ScoringOverlay::updatePhaseLogic(float deltaTime, TerritoryOwner targetOwner, float maxDist)
{
    auto BoardToPixelCoord = [*this](sf::Vector2i p) -> sf::Vector2i
    {
        return sf::Vector2i(m_boardTopLeft.x + p.x * m_cellSize - m_cellSize * BORDER_OFFSET_RATIO,
                            m_boardTopLeft.y + p.y * m_cellSize - m_cellSize * BORDER_OFFSET_RATIO);
    };

    bool allComponentsDone = true;

    for(auto &comps : m_ConnectedComponents)
    {
        if(comps.owner != targetOwner) continue;
        if(comps.isFinished) continue;

        allComponentsDone = false;
        bool changeMade = false;
//        std::cout << changeMade << " ";

        if(!comps.isActive)
        {
            comps.isActive = true;

            sf::Vector2i &parent = comps.anchorPoint;

            spawnEdge(comps, comps.anchorPoint);
        }

        size_t n = comps.activeEdges.size();
        for(size_t i = 0; i < n; ++i)
        {
            ActiveEdge& edge = comps.activeEdges[i];
            if(edge.isTriggered) continue;

            edge.progress += deltaTime / EDGE_GROW_DURATION;
            if(edge.progress >= 1.f) edge.progress = 1.f;

            float curScaleY = edge.targetScaleY * edge.progress;
            sf::Vector2f sScale = edge.sprite.getScale();
            edge.sprite.setScale(sScale.x, curScaleY);

            changeMade = true;

            if(edge.progress >= 1.f && !edge.isTriggered)
            {
                edge.isTriggered = true;
                spawnEdge(comps, edge.endNode);
            }
        }

        if(!changeMade)
        {
            comps.isFinished = true;
//            std::cout << "yes we complete this one! ";
        }
//        std::cout << changeMade << " : " << comps.isFinished << "\n";
        break;
    }

//    std::cout << "\n";
    return allComponentsDone;
}

bool ScoringOverlay::isAnimationFinished() const
{
    return m_currentPhase == Phase::Finished;
}
bool ScoringOverlay::isScoreboardVisible() const
{
    return m_currentPhase == Phase::Finished;
}

bool ScoringOverlay::contains(const sf::Vector2f& point, sf::Vector2u windowSize) const
{
    if(m_currentPhase != Phase::Finished) return false;

    sf::FloatRect bounds = m_boardSprite.getGlobalBounds();
    return bounds.contains(point);
}

void ScoringOverlay::renderDeadStones(sf::RenderTarget& target)
{
    if(m_deadStones.empty()) return;

    float alpha = 255.f;
    float scale = 1.0f;

    if(m_currentPhase == Phase::DeadStoneAnim)
    {
        float progress = m_timer / DEAD_STONE_DURATION;
        if(progress > 1.0f) progress = 1.0f;

        float smooth = std::sin(progress * PI / 2.0f);

        alpha = 255.f + smooth * (FINAL_ALPHA - 255.f);

        scale = 1.0f + smooth * (FINAL_SCALE - 1.0f);
    }
    else if(m_currentPhase == Phase::Idle)
    {
        alpha = 255.f;
        scale = 1.0f;
    }
    else
    {
        alpha = (float)FINAL_ALPHA;
        scale = FINAL_SCALE;
    }

    const sf::Texture& blackTex = ResourceManager::getInstance().getTexture(GlobalSetting::getInstance().getStoneTextureKey(true, m_boardSize));
    const sf::Texture& whiteTex = ResourceManager::getInstance().getTexture(GlobalSetting::getInstance().getStoneTextureKey(false, m_boardSize));

    sf::Uint8 finalAlphaUint = static_cast<sf::Uint8>(alpha);

    for(const auto& stone : m_deadStones)
    {
        sf::Sprite s;
        s.setTexture(stone.owner == TerritoryOwner::Black ? blackTex : whiteTex);

        float px = m_boardTopLeft.x + stone.pos.x * m_cellSize;
        float py = m_boardTopLeft.y + stone.pos.y * m_cellSize;

        sf::FloatRect b = s.getLocalBounds();
        s.setOrigin(b.width / 2.f, b.height / 2.f);
        s.setPosition(px, py);

        s.setScale(scale, scale);

        s.setColor(sf::Color(255, 255, 255, finalAlphaUint));

        target.draw(s);
    }
}

}
