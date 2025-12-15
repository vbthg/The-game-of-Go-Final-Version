#include "GamePlay.h"
#include "ResourceManager.h"
#include "GlobalSetting.h"
#include "ScaleEffect.h"
#include "PachiBot.h"
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <filesystem>

const float INITIAL_STONE_SCALE = 1.5f;
const float STONE_SHRINK_SPEED = 8.0f;

const float MARKER_RATIO = 0.15f;

GamePlay::GamePlay(sf::RenderWindow& window, int boardSize, GameMode mode, AiDifficulty difficulty) :
    m_window(window),
    m_requestedState(GameStateType::NoChange),
    m_logic(boardSize),
    m_boardSize(boardSize),
    m_mode(mode),
    m_difficulty(difficulty),
    m_aiShouldMove(false),
    m_aiThinkTimer(0.f),
    m_gameHasEnded(false),
    m_background(ResourceManager::getInstance().getTexture("gameplay_background")),
    m_turnactionBackground(ResourceManager::getInstance().getTexture("gameplay_turnaction_background")),
    m_logBackground(ResourceManager::getInstance().getTexture("gameplay_log_background")),
    m_timerPanel(ResourceManager::getInstance().getTexture("gameplay_timer_bg")),
    m_boardSprite(ResourceManager::getInstance().getTexture(GlobalSetting::getInstance().getBoardTextureKey(boardSize))),
    m_blackStoneTex(&ResourceManager::getInstance().getTexture(GlobalSetting::getInstance().getStoneTextureKey(true, boardSize))),
    m_whiteStoneTex(&ResourceManager::getInstance().getTexture(GlobalSetting::getInstance().getStoneTextureKey(false, boardSize))),
    m_font(ResourceManager::getInstance().getFont("main_font")),
    m_pauseButton(ResourceManager::getInstance().getTexture("gameplay_pause_btn"), {1545.f, 55.f}, true),
    m_passButton(ResourceManager::getInstance().getTexture("gameplay_pass_btn"), {1268.f, 175.f}, true),
    m_undoBtn(ResourceManager::getInstance().getTexture("gameplay_undo_btn"), {1387.f, 175.f}, true),
    m_redoBtn(ResourceManager::getInstance().getTexture("gameplay_redo_btn"), {1506.f, 175.f}, true),
    m_resignButton(ResourceManager::getInstance().getTexture("gameplay_resign_btn"), {55.f, 55.f}, true),
    m_hintButton(ResourceManager::getInstance().getTexture("gameplay_hint_btn"), {110.f, 55.f}, true),
    m_blackUsedHint(false),
    m_whiteUsedHint(false),
    m_hintCoord(-5, -5)
{
    float boardDisplaySize = 740.f;
    float boardDisplayX = 430.f;
    float boardDisplayY = 110.f;
    float paddingTop = 80.f;
    float paddingBot = (m_boardSize == 19 ? 25.f : m_boardSize == 13 ? 40.f : 60.f);

    m_boardSprite.setPosition(boardDisplayX, boardDisplayY);
    m_boardTopLeftX = boardDisplayX + paddingTop;
    m_boardTopLeftY = boardDisplayY + paddingTop;
    float boardClickableSize = boardDisplaySize - (paddingTop + paddingBot);
    m_cellSpacing = boardClickableSize / (m_boardSize - 1);

    m_messageText.setFont(m_font);
    m_messageText.setCharacterSize(20);
    m_messageText.setFillColor(sf::Color::Red);

    m_turnText.setFont(m_font);
    m_turnText.setCharacterSize(24);
    m_turnText.setFillColor(sf::Color::White);
    m_turnText.setPosition(boardDisplayX, 20.f);

    m_gameOverText.setFont(m_font);
    m_gameOverText.setCharacterSize(40);
    m_gameOverText.setFillColor(sf::Color::Yellow);
    m_gameOverText.setString("GAME OVER");
    sf::FloatRect textBounds = m_gameOverText.getLocalBounds();
    m_gameOverText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
    m_gameOverText.setPosition(std::round(m_window.getSize().x / 2.f), std::round(m_window.getSize().y / 2.f));

    m_winPanel.setSize(sf::Vector2f(500.f, 80.f));
    m_winPanel.setFillColor(sf::Color(0, 0, 0, 200));
    m_winPanel.setOutlineColor(sf::Color(200, 200, 200));
    m_winPanel.setOutlineThickness(2.f);

    m_winPanel.setOrigin(250.f, 40.f);
    m_winPanel.setPosition(m_window.getSize().x / 2.f, 450.f);

    m_winText.setFont(m_font);
    m_winText.setCharacterSize(32);
    m_winText.setStyle(sf::Text::Bold);

    m_stoneScaleMatrix.resize(m_boardSize, std::vector<float>(m_boardSize, 1.0f));

    m_lastMoveCoord = {-1, -1};

    float markerSize = m_cellSpacing * MARKER_RATIO;
    m_lastMoveMarker.setSize(sf::Vector2f(markerSize, markerSize));
    m_lastMoveMarker.setFillColor(sf::Color::Red);
    m_lastMoveMarker.setOrigin(markerSize / 2.f, markerSize / 2.f);

    m_historyList = std::make_unique<UI::HistoryList>(
        sf::Vector2f(227.f, 400.f),
        sf::Vector2f(280.f, 563.f),
        ResourceManager::getInstance().getTexture("gameplay_historylist_background"),
        ResourceManager::getInstance().getTexture("gameplay_historylist_track"),
        ResourceManager::getInstance().getTexture("gameplay_historylist_icon_black"),
        ResourceManager::getInstance().getTexture("gameplay_historylist_icon_white"),
        ResourceManager::getInstance().getFont("main_font")
    );

    m_timeline = std::make_unique<UI::Timeline>(
        sf::Vector2f(800.f, 65.f),
        sf::Vector2f(1200.f, 15.f),
        ResourceManager::getInstance().getTexture("gameplay_timeline_background"),
        ResourceManager::getInstance().getTexture("gameplay_timeline_tooltip_bg"),
        ResourceManager::getInstance().getFont("main_font")
    );

    sf::FloatRect bounds = m_turnactionBackground.getLocalBounds();
    m_turnactionBackground.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    m_turnactionBackground.setPosition(sf::Vector2f(1387.f, 175.f));

    m_logBackground.setPosition(sf::Vector2f(520.f, 850));

    m_loadingSprite.setTexture(ResourceManager::getInstance().getTexture("loading_icon"));
    bounds = m_loadingSprite.getLocalBounds();
    m_loadingSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    m_loadingSprite.setPosition(sf::Vector2f(675.f, 870.f));

    int limitSeconds = GlobalSetting::getInstance().getTimeLimitInSeconds();

    if(limitSeconds == -1)
    {
        m_isTimeLimitEnabled = false;
        m_timeLimitBlack = 0;
        m_timeLimitWhite = 0;
    }
    else
    {
        m_isTimeLimitEnabled = true;
        m_timeLimitBlack = (float)limitSeconds;
        m_timeLimitWhite = (float)limitSeconds;
    }

    bounds = m_timerPanel.getLocalBounds();
    m_timerPanel.setOrigin(sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));
    m_timerPanel.setTexture(ResourceManager::getInstance().getTexture("gameplay_timer_bg"));
    m_timerPanel.setPosition(1387.f, 343.f);

    std::string blackKey = GlobalSetting::getInstance().getStoneTextureKey(true, 19);
    std::string whiteKey = GlobalSetting::getInstance().getStoneTextureKey(false, 19);

    m_timerIconBlack.setTexture(ResourceManager::getInstance().getTexture(blackKey));
    m_timerIconWhite.setTexture(ResourceManager::getInstance().getTexture(whiteKey));

    bounds = m_timerIconBlack.getLocalBounds();
    m_timerIconBlack.setOrigin(sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));
    bounds = m_timerIconWhite.getLocalBounds();
    m_timerIconWhite.setOrigin(sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));

    m_timerIconBlack.setPosition(1298.f, 311.f);
    m_timerIconWhite.setPosition(1298.f, 377.f);

    updateTimerDisplay();
    updateOnBoardCount();

    int charSize = 24;

    auto setupText = [&](sf::Text& text, bool isBlack)
    {
        text.setFont(m_font);
        text.setCharacterSize(charSize);
        if(isBlack)
        {
            text.setFillColor(sf::Color::Black);
        }
        else
        {
            text.setFillColor(sf::Color::White);
        }
    };

    setupText(m_timerTextBlack, true);
    setupText(m_timerTextWhite, false);

    setupText(m_onBoardTextBlack, true);
    setupText(m_onBoardTextWhite, false);
    m_onBoardTextBlack.setString("0");
    m_onBoardTextWhite.setString("0");

    auto setCenterOrigin = [](sf::Text& text)
    {
        sf::FloatRect b = text.getLocalBounds();
        text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
    };

    setCenterOrigin(m_onBoardTextBlack);
    setCenterOrigin(m_onBoardTextWhite);
    setCenterOrigin(m_timerTextBlack);
    setCenterOrigin(m_timerTextWhite);

    m_onBoardTextBlack.setPosition(1350.f, 311.f);
    m_onBoardTextWhite.setPosition(1350.f, 377.f);

    m_timerTextBlack.setPosition(1466.f, 311.f);
    m_timerTextWhite.setPosition(1466.f, 377.f);

    m_shouldDrawStatusPanel = true;

    m_onBoardTextBlack.setString("0");
    m_onBoardTextWhite.setString("0");
    m_window.draw(m_onBoardTextBlack);

    m_pauseButton.setOnClick([this]()
    {
        this->onPauseClick();
    });
    m_passButton.setOnClick([this]()
    {
        this->onPassClick();
    });
    m_undoBtn.setOnClick([this]()
    {
        this->onUndoClick();
    });
    m_redoBtn.setOnClick([this]()
    {
        this->onRedoClick();
    });
    m_resignButton.setOnClick([this]()
    {
        this->onResignClick();
    });
    m_hintButton.setOnClick([this]()
    {
        this->onHintClick();
    });

    m_moveTimer.restart();

    m_isScoringMode = false;
    m_scoringOverlay = std::make_unique<UI::ScoringOverlay>(
        m_boardSize, m_cellSpacing,
        sf::Vector2f(m_boardTopLeftX, m_boardTopLeftY),
        m_font
    );

    GlobalSetting::getInstance().setOnThemeChanged([this]()
    {
        this -> updateThemeResources();
    });

    updateThemeResources();

//    startGameInitialization();
}

void GamePlay::startGameInitialization()
{
    m_isInitializing = true;
    showMessage("Starting Engine...", MsgType::Info);

    std::thread([this]()
    {
        std::lock_guard<std::mutex> lock(m_workerMutex);

        BotManager::getInstance().startBackgroundPachi(m_boardSize);

        if(m_mode == GameMode::PlayerVsAI)
        {
            BotManager::getInstance().startBot(m_difficulty, m_boardSize);
            m_bot = BotManager::getInstance().getBot();
        }
        else
        {
            m_bot = nullptr;
        }

        performSyncBoardInternal();

        std::this_thread::sleep_for(std::chrono::milliseconds(1400));

        m_isInitializing = false;

    }).detach();
}

void GamePlay::performSyncBoardInternal()
{
    auto board = m_logic.getBoard();
    int size = m_boardSize;

    auto bgBot = BotManager::getInstance().getBackgroundBot();
    auto oppBot = m_bot;

    auto resetSingleBot = [&](std::shared_ptr<IBot> bot)
    {
        if(!bot) return;
        if(auto pachi = std::dynamic_pointer_cast<PachiBot>(bot))
        {
            pachi->sendCommand("clear_board");
            pachi->setBoardSize(size);
            pachi->sendCommand("boardsize " + std::to_string(size));
        }
        else
        {
            bot->init();
        }

        for(int y = 0; y < size; ++y)
        {
            for(int x = 0; x < size; ++x)
            {
                if(board[y][x] != StoneType::Empty)
                {
                    std::string c = (board[y][x] == StoneType::Black) ? "black" : "white";
                    bot->syncMove(c, x, y);
                }
            }
        }
    };

    resetSingleBot(bgBot);
    if(oppBot && oppBot != bgBot)
    {
        resetSingleBot(oppBot);
    }

    std::cout << "[GamePlay] Board synced to bots successfully.\n";
}

void GamePlay::updateThemeResources()
{
    auto& gs = GlobalSetting::getInstance();
    auto& res = ResourceManager::getInstance();
    int boardSize = m_boardSize;

    std::string blackKey = gs.getStoneTextureKey(true, boardSize);
    std::string whiteKey = gs.getStoneTextureKey(false, boardSize);
    m_blackStoneTex = &res.getTexture(blackKey);
    m_whiteStoneTex = &res.getTexture(whiteKey);

    m_timerIconBlack.setTexture(*m_blackStoneTex);
    m_timerIconWhite.setTexture(*m_whiteStoneTex);

    blackKey = gs.getStoneTextureKey(true, 19);
    whiteKey = gs.getStoneTextureKey(false, 19);

    m_timerIconBlack.setTexture(res.getTexture(blackKey));
    m_timerIconWhite.setTexture(res.getTexture(whiteKey));

    std::string boardKey = gs.getBoardTextureKey(boardSize);
    m_boardSprite.setTexture(res.getTexture(boardKey));

    if(m_historyList)
    {
        m_historyList->updateTheme(&res.getTexture(gs.getStoneTextureKey(true, 13)), &res.getTexture(gs.getStoneTextureKey(false, 13)));
    }

    m_soundCaptureBlack.stop(); m_soundCaptureWhite.stop();
    m_soundErrorBlack.stop();   m_soundErrorWhite.stop();

    m_soundPlaceBlack.stop(); m_soundPlaceWhite.stop();

    std::string capB = gs.getSoundKey("capture", true);
    std::string capW = gs.getSoundKey("capture", false);

    std::string errB = gs.getSoundKey("error", true);
    std::string errW = gs.getSoundKey("error", false);

    std::string plaB = gs.getSoundKey("place", true);
    std::string plaW = gs.getSoundKey("place", false);

    std::string pasB = gs.getSoundKey("pass", true);
    std::string pasW = gs.getSoundKey("pass", false);

    try
    {
        m_soundCaptureBlack.setBuffer(res.getSoundBuffer(capB));
        m_soundCaptureWhite.setBuffer(res.getSoundBuffer(capW));

        m_soundErrorBlack.setBuffer(res.getSoundBuffer(errB));
        m_soundErrorWhite.setBuffer(res.getSoundBuffer(errW));

        m_soundPlaceBlack.setBuffer(res.getSoundBuffer(plaB));
        m_soundPlaceWhite.setBuffer(res.getSoundBuffer(plaW));

        m_soundPassBlack.setBuffer(res.getSoundBuffer(pasB));
        m_soundPassWhite.setBuffer(res.getSoundBuffer(pasW));
    }
    catch(...)
    {
        std::cerr << "Error loading theme sounds, fallback to basic needed.\n";
    }

    float vol = gs.sfxVolume;
    m_soundCaptureBlack.setVolume(vol);
    m_soundCaptureWhite.setVolume(vol);
    m_soundErrorBlack.setVolume(vol);
    m_soundErrorWhite.setVolume(vol);
    m_soundPlaceBlack.setVolume(vol);
    m_soundPlaceWhite.setVolume(vol);
}

void GamePlay::requestHintFromBot()
{
    auto hintBot = BotManager::getInstance().getBackgroundBot();
    if(!hintBot && std::dynamic_pointer_cast<PachiBot>(m_bot))
        hintBot = std::dynamic_pointer_cast<PachiBot>(m_bot);

    if(hintBot)
    {
        m_isCalculatingHint = true;
        showMessage("Analyzing position...", MsgType::Info);

        bool isBlack = m_logic.isBlacksTurn();
        auto currentBoard = m_logic.getBoard();
        int boardSize = m_boardSize;

        std::thread([this, hintBot, isBlack, currentBoard, boardSize]()
        {
            std::lock_guard<std::mutex> lock(m_workerMutex);

            auto pachi = std::dynamic_pointer_cast<PachiBot>(hintBot);
            if(pachi)
            {
                pachi->sendCommand("time_settings 0 8 1");
            }
            else
            {
                hintBot->init();
            }

            BotMove move = hintBot->generateMove(isBlack);

            if(pachi)
            {
                pachi->sendCommand("undo");

                if(m_mode == GameMode::PlayerVsAI)
                {
                    if(m_difficulty == AiDifficulty::Hard)
                        pachi->sendCommand("time_settings 0 8 1");
                    else if(m_difficulty == AiDifficulty::Medium)
                        pachi->sendCommand("time_settings 0 4 1");
                    else
                        pachi->sendCommand("time_settings 0 1 1");
                }
                else
                {
                    pachi->sendCommand("time_settings 0 1 1");
                }
            }

            if(move.isResign)
            {
                m_hintCoord = {-2, -2};
            }
            else if(move.isPass)
            {
                m_hintCoord = {-1, -1};
            }
            else
            {
                m_hintCoord = {move.x, move.y};
            }

            m_isCalculatingHint = false;

        }).detach();
    }
}

void GamePlay::showMessage(const std::string& msg, MsgType type)
{
    m_messageText.setString(msg);
    m_messageText.setStyle(sf::Text::Bold);

    switch(type)
    {
    case MsgType::Error:
        m_messageText.setFillColor(sf::Color(255, 80, 80));
        break;
    case MsgType::Success:
        m_messageText.setFillColor(sf::Color(100, 255, 100));
        break;
    case MsgType::Info:
    default:
        m_messageText.setFillColor(sf::Color::White);
        break;
    }

    placeMessageText(m_messageText);

    m_messageTimer = MESSAGE_DURATION;
}

void GamePlay::handleEvent(sf::Event& event)
{
    if(event.type == sf::Event::Closed)
    {
        m_window.close();
        return;
    }

    if(m_isScoringMode || m_gameHasEnded || m_isInitializing)
    {
        m_pauseButton.handleEvent(event, m_window);
        return;
    }

    m_resignButton.handleEvent(event, m_window);
    m_pauseButton.handleEvent(event, m_window);
    m_passButton.handleEvent(event, m_window);
    m_undoBtn.handleEvent(event, m_window);
    m_redoBtn.handleEvent(event, m_window);
    m_hintButton.handleEvent(event, m_window);
    m_historyList->handleEvent(event, m_window);
    m_timeline->handleEvent(event, m_window);

    if(event.type == sf::Event::MouseButtonPressed)
    {
        if(event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mousePos = m_window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
            if(m_pauseButton.getSprite().getGlobalBounds().contains(mousePos) ||
               m_passButton.getSprite().getGlobalBounds().contains(mousePos) ||
               m_undoBtn.getSprite().getGlobalBounds().contains(mousePos) ||
               m_redoBtn.getSprite().getGlobalBounds().contains(mousePos) ||
               m_resignButton.getSprite().getGlobalBounds().contains(mousePos))
            {
            }
            else
            {
                onBoardClick(event.mouseButton.x, event.mouseButton.y);
            }
        }
    }
}

void GamePlay::syncToAllBots(const std::string& color, int x, int y)
{
    std::thread([this, color, x, y]()
    {
        std::lock_guard<std::mutex> lock(m_workerMutex);

        if(m_bot)
        {
            m_bot->syncMove(color, x, y);
        }

        auto bgBot = BotManager::getInstance().getBackgroundBot();

        if(bgBot && bgBot != m_bot)
        {
            bgBot->syncMove(color, x, y);
        }

    }).detach();
}

void GamePlay::fullResyncBots()
{
    std::thread t([this]()
    {
        std::lock_guard<std::mutex> lock(m_workerMutex);
        performSyncBoardInternal();
    });
    t.detach();
}

void GamePlay::placeMessageText(sf::Text& m_messageText)
{
    m_messageText.setStyle(sf::Text::Bold);
    sf::FloatRect bounds = m_messageText.getLocalBounds();
    m_messageText.setOrigin(std::round(bounds.left + bounds.width / 2.f), std::round(bounds.top + bounds.height / 2.f));
    m_messageText.setPosition(sf::Vector2f(800.f, 875.f));
}

GameStateType GamePlay::update(float deltaTime)
{
    if(!m_bot && m_mode == GameMode::PlayerVsAI && !m_isInitializing)
    {
        if(BotManager::getInstance().isReady())
        {
            m_bot = BotManager::getInstance().getBot();
        }
    }

    if(m_isScoringMode && m_scoringOverlay)
    {
        m_scoringOverlay->update(deltaTime);
    }

    bool isBusy = m_isAiThinkingWorker || m_isCalculatingHint || m_isInitializing;

    if(isBusy)
    {
        m_loadingSprite.rotate(180.f * deltaTime);

        m_messageTimer = 2.0f;

        if(m_isInitializing)
        {
            if(m_messageText.getString() != "Starting Engine...")
                showMessage("Starting Engine...", MsgType::Info);
        }
        if(m_isAiThinkingWorker)
        {
            if(m_messageText.getString() != "Bot is thinking...")
            {
                showMessage("Bot is thinking...", MsgType::Info);
            }
        }
    }

    static bool wasInitializing = false;
    if(wasInitializing && !m_isInitializing)
    {
        showMessage("Ready!", MsgType::Success);

        bool isScoring = (m_endReason.find("SCORING") != std::string::npos);
        if(m_gameHasEnded && isScoring)
        {
            auto bgBot = BotManager::getInstance().getBackgroundBot();
            if(bgBot) m_deadStones = bgBot->getDeadStones();

            finalizeScore();

            std::vector<DeadStoneInfo> displayStones;
            auto currentBoard = m_logic.getBoard();
            for(const auto& p : m_deadStones)
            {
                if(p.x >= 0 && p.x < m_boardSize && p.y >= 0 && p.y < m_boardSize)
                {
                    StoneType type = currentBoard[p.y][p.x];
                    TerritoryOwner owner = (type == StoneType::Black) ? TerritoryOwner::Black : TerritoryOwner::White;
                    displayStones.push_back({ p, owner });
                }
            }
            std::vector<TerritoryRegion> regions = m_logic.getTerritoryRegions(m_deadStones);

            m_isScoringMode = true;
            if(m_scoringOverlay) m_scoringOverlay->startAnimation(regions, displayStones);
        }

        if(m_mode == GameMode::PlayerVsAI && m_bot && !m_logic.isBlacksTurn())
        {
             m_aiShouldMove = true;
        }
    }
    wasInitializing = m_isInitializing;

    static bool wasCalculatingHint = false;

    if(wasCalculatingHint && !m_isCalculatingHint)
    {
        if(m_hintCoord.x == -2 && m_hintCoord.y == -2)
        {
            showMessage("Bot suggests: Resign!", MsgType::Error);
        }
        else if(m_hintCoord.x == -1 && m_hintCoord.y == -1)
        {
            showMessage("Bot suggests: Pass", MsgType::Info);
        }
        else
        {
            showMessage("Hint Ready!", MsgType::Success);
        }
    }
    wasCalculatingHint = m_isCalculatingHint;

    if(m_messageTimer > 0.f)
    {
        m_messageTimer -= deltaTime;
        if(m_messageTimer <= 0.f)
        {
            m_messageText.setString("");
        }
    }

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            if(m_stoneScaleMatrix[y][x] > 1.0f)
            {
                m_stoneScaleMatrix[y][x] -= STONE_SHRINK_SPEED * deltaTime;
                if(m_stoneScaleMatrix[y][x] < 1.0f) m_stoneScaleMatrix[y][x] = 1.0f;
            }
        }
    }

    if(m_isScoringMode || m_gameHasEnded || m_isInitializing)
    {
        m_pauseButton.update(m_window);
        if(m_timeline) m_timeline->update(deltaTime, m_window);

        sf::Cursor cursor;
        if(m_pauseButton.isHoveredAndInteractive())
        {
            if(cursor.loadFromSystem(sf::Cursor::Hand)) m_window.setMouseCursor(cursor);
        }
        else
        {
            if(cursor.loadFromSystem(sf::Cursor::Arrow)) m_window.setMouseCursor(cursor);
        }
    }
    else
    {
        m_pauseButton.update(m_window);
        m_passButton.update(m_window);
        m_undoBtn.update(m_window);
        m_redoBtn.update(m_window);
        m_resignButton.update(m_window);
        m_hintButton.update(m_window);

        bool isBlack = m_logic.isBlacksTurn();
        if((isBlack && m_blackUsedHint) || (!isBlack && m_whiteUsedHint))
        {
            m_hintButton.getSprite().setColor(sf::Color(100, 100, 100));
        }

        if(m_historyList) m_historyList->update(m_window);
        if(m_timeline) m_timeline->update(deltaTime, m_window);

        updateTimerDisplay();
        updateOnBoardCount();

        if(m_isTimeLimitEnabled)
        {
            if(m_logic.isBlacksTurn())
            {
                m_timeLimitBlack -= deltaTime;
                if(m_timeLimitBlack <= 0.f)
                {
                    m_timeLimitBlack = 0.f;

                    m_gameHasEnded = true;
                    m_isScoringMode = true;

                    if(m_scoringOverlay)
                        m_scoringOverlay->showSimpleResult("Time Out!\nWhite Wins!", false);

                    m_turnText.setString("");
                    m_endReason = "Time Out!\nWhite Wins!";
                }
            }
            else
            {
                m_timeLimitWhite -= deltaTime;
                if(m_timeLimitWhite <= 0.f)
                {
                    m_timeLimitWhite = 0.f;

                    m_gameHasEnded = true;
                    m_isScoringMode = true;

                    if(m_scoringOverlay)
                        m_scoringOverlay->showSimpleResult("Time Out!\nBlack Wins!", true);

                    m_turnText.setString("");
                    m_endReason = "Time Out!\nBlack Wins!";
                }
            }
        }

        bool isHovering = (m_pauseButton.isHoveredAndInteractive() || m_passButton.isHoveredAndInteractive() ||
                           m_undoBtn.isHoveredAndInteractive() || m_redoBtn.isHoveredAndInteractive() ||
                           m_resignButton.isHoveredAndInteractive() || m_hintButton.isHoveredAndInteractive());
        sf::Cursor cursor;
        if(isHovering)
        {
            if(cursor.loadFromSystem(sf::Cursor::Hand)) m_window.setMouseCursor(cursor);
        }
        else
        {
            if(cursor.loadFromSystem(sf::Cursor::Arrow)) m_window.setMouseCursor(cursor);
        }
    }

    if(m_mode == GameMode::PlayerVsAI && m_bot && !m_gameHasEnded &&
       !m_logic.isBlacksTurn() && !m_isAiThinkingWorker && !m_isInitializing)
    {
        m_aiShouldMove = true;
    }

    if(!m_gameHasEnded && m_mode == GameMode::PlayerVsAI && m_aiShouldMove && !m_isInitializing)
    {
        if(!m_isAiThinkingWorker)
        {
            showMessage("Bot is thinking...", MsgType::Info);

            m_isAiThinkingWorker = true;
            bool aiIsBlack = m_logic.isBlacksTurn();
            m_moveTimer.restart();

            PendingMove userMove = m_pendingPlayerMove;
            m_pendingPlayerMove.active = false;

            m_aiFuture = std::async(std::launch::async, [this, aiIsBlack, userMove]()
            {
                if(!m_bot) return BotMove();
                std::cout << "[AI] Calling generateMove()...\n";
                BotMove move = m_bot->generateMove(aiIsBlack);
                return move;
            });
        }
        else
        {
            if(m_aiFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            {
                BotMove move = m_aiFuture.get();
                m_isAiThinkingWorker = false;
                m_aiShouldMove = false;

                if(m_messageText.getString() == "Bot is thinking...")
                {
                    m_messageText.setString("");
                    m_messageTimer = 0.f;
                }

                float realThinkingTime = m_moveTimer.getElapsedTime().asSeconds();
                if(realThinkingTime < 0.1f) realThinkingTime = 0.1f;

                if(move.isResign)
                {
                    m_gameHasEnded = true;
                    showMessage("AI RESIGNED!", MsgType::Info);
                    sf::FloatRect b = m_winText.getLocalBounds();
                    m_winText.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
                    m_winText.setPosition(m_winPanel.getPosition());
                    m_turnText.setString("");
                }
                else if(move.isPass)
                {
                    onPassClick(true);
                }
                else
                {
                    MoveResult result = m_logic.attemptMove(move.x, move.y);
                    if(result.success)
                    {
                        m_lastMoveCoord = sf::Vector2i(move.x, move.y);
                        m_stoneScaleMatrix[move.y][move.x] = INITIAL_STONE_SCALE;
                        std::string notation = convertCoordsToNotation(move.x, move.y);
                        bool aiColorIsBlack = !m_logic.isBlacksTurn();
                        std::string aiColorStr = aiColorIsBlack ? "black" : "white";

                        if(std::dynamic_pointer_cast<PachiBot>(m_bot) == nullptr)
                        {
                            syncToAllBots(aiColorStr, move.x, move.y);
                        }
                        else
                        {
                            auto bgBot = BotManager::getInstance().getBackgroundBot();
                            if(bgBot && bgBot != m_bot) bgBot->syncMove(aiColorStr, move.x, move.y);
                        }

                        m_historyList->addMove(aiColorIsBlack, notation);
                        m_timeline->addMove(realThinkingTime, aiColorIsBlack, notation);

                        float currentVol = GlobalSetting::getInstance().sfxVolume;
                        float randomPitch = 0.9f + static_cast<float>(std::rand() % 21) / 100.0f;

                        if(!result.capturedStones.empty())
                        {
                            if(aiColorIsBlack) { m_soundCaptureBlack.setVolume(currentVol); m_soundCaptureBlack.play(); }
                            else { m_soundCaptureWhite.setVolume(currentVol); m_soundCaptureWhite.play(); }
                        }
                        else
                        {
                            if(aiColorIsBlack) { m_soundPlaceBlack.setVolume(currentVol); m_soundPlaceBlack.setPitch(randomPitch); m_soundPlaceBlack.play(); }
                            else { m_soundPlaceWhite.setVolume(currentVol); m_soundPlaceWhite.setPitch(randomPitch); m_soundPlaceWhite.play(); }
                        }
                    }
                    else
                    {
                         std::cout << "CRITICAL: Bot invalid move. Forcing PASS.\n";
                         onPassClick(true);
                    }
                }
            }
        }
    }

    GameStateType stateToReturn = m_requestedState;
    m_requestedState = GameStateType::NoChange;
    return stateToReturn;
}

void GamePlay::draw()
{
    m_window.clear(sf::Color::Black);
    m_window.draw(m_background);
    m_window.draw(m_boardSprite);
    m_window.draw(m_turnactionBackground);
    m_window.draw(m_logBackground);
    drawStones();

    bool isBusy = m_isAiThinkingWorker || m_isCalculatingHint || m_isInitializing;
    if(isBusy && !m_gameHasEnded)
    {
        m_window.draw(m_loadingSprite);
    }

    if(m_shouldDrawStatusPanel)
    {
        m_window.draw(m_timerPanel);

        m_window.draw(m_timerIconBlack);
        m_window.draw(m_timerIconWhite);

        m_window.draw(m_timerTextBlack);
        m_window.draw(m_timerTextWhite);

        m_window.draw(m_onBoardTextBlack);
        m_window.draw(m_onBoardTextWhite);
    }

    if(!m_gameHasEnded && !m_isScoringMode)
    {
        sf::Vector2i mouseGrid = getBoardCoordsFromMouse(sf::Mouse::getPosition(m_window).x, sf::Mouse::getPosition(m_window).y);

        if(mouseGrid.x != -1 && m_logic.getStoneAt(mouseGrid.x, mouseGrid.y) == StoneType::Empty && !m_aiShouldMove)
        {
            sf::Sprite ghostSprite;
            bool isBlack = m_logic.isBlacksTurn();
            ghostSprite.setTexture(isBlack ? *m_blackStoneTex : *m_whiteStoneTex);

            ghostSprite.setColor(sf::Color(255, 255, 255, 120));

            sf::FloatRect texBounds = ghostSprite.getLocalBounds();
            ghostSprite.setOrigin(texBounds.width / 2.f, texBounds.height / 2.f);

            ghostSprite.setScale(0.8f, 0.8f);

            ghostSprite.setPosition(
                std::round(m_boardTopLeftX + (mouseGrid.x * m_cellSpacing)),
                std::round(m_boardTopLeftY + (mouseGrid.y * m_cellSpacing))
            );

            m_window.draw(ghostSprite);
        }
    }

    if(m_isScoringMode)
    {
        m_scoringOverlay->draw(m_window);
    }
    else
    {
        m_window.draw(m_turnText);
    }

    m_pauseButton.draw(m_window);
    m_passButton.draw(m_window);
    m_undoBtn.draw(m_window);
    m_redoBtn.draw(m_window);
    m_resignButton.draw(m_window);
    m_hintButton.draw(m_window);
    m_historyList->draw(m_window);
    m_timeline->draw(m_window);
    m_window.draw(m_messageText);
}

void GamePlay::drawStones()
{
    sf::Sprite stoneSprite;

    std::vector < std::vector<bool> > isDeadStone(m_boardSize, std::vector<bool>(m_boardSize, false));
    for(auto &p : m_deadStones) isDeadStone[p.y][p.x] = true;

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x) if(!isDeadStone[y][x])
        {
            StoneType stone = m_logic.getStoneAt(x, y);
            if(stone == StoneType::Empty && (y != m_hintCoord.y || x != m_hintCoord.x)) continue;
            if(y == m_hintCoord.y && x == m_hintCoord.x)
            {
                stone = (m_logic.isBlacksTurn() ? StoneType::Black : StoneType::White);
            }

            stoneSprite.setTexture((stone == StoneType::Black) ? *m_blackStoneTex : *m_whiteStoneTex);
            sf::FloatRect texBounds = stoneSprite.getLocalBounds();
            stoneSprite.setOrigin(texBounds.width / 2.f, texBounds.height / 2.f);

            float currentScale = m_stoneScaleMatrix[y][x];
            stoneSprite.setScale(currentScale, currentScale);

            stoneSprite.setPosition(std::round(m_boardTopLeftX + (x * m_cellSpacing)), std::round(m_boardTopLeftY + (y * m_cellSpacing)));

            if(y == m_hintCoord.y && x == m_hintCoord.x)
            {
                stoneSprite.setColor(sf::Color(255, 255, 255, 100));
            }
            else
            {
                stoneSprite.setColor(sf::Color::White);
            }

            m_window.draw(stoneSprite);

            auto &gs = GlobalSetting::getInstance();
            if(x == m_lastMoveCoord.x && y == m_lastMoveCoord.y && gs.stoneThemeIndex == 0)
            {
                m_lastMoveMarker.setScale(currentScale, currentScale);
                m_lastMoveMarker.setPosition(std::round(m_boardTopLeftX + (x * m_cellSpacing)), std::round(m_boardTopLeftY + (y * m_cellSpacing)));
                m_window.draw(m_lastMoveMarker);
            }
        }
    }
}

void GamePlay::onBoardClick(int mouseX, int mouseY)
{
    if(m_gameHasEnded) return;
    if(m_mode == GameMode::PlayerVsAI && !m_logic.isBlacksTurn())
    {
        showMessage("Not your turn!", MsgType::Error);
        return;
    }

    sf::Vector2i boardCoords = getBoardCoordsFromMouse(mouseX, mouseY);
    int x = boardCoords.x;
    int y = boardCoords.y;
    if(x == -1 || y == -1) return;

    bool isBlackMove = m_logic.isBlacksTurn();
    float thinkingTime = m_moveTimer.restart().asSeconds();

    MoveResult result = m_logic.attemptMove(x, y);
    float currentVol = GlobalSetting::getInstance().sfxVolume;

    if(result.success)
    {
        m_hintCoord = {-5, -5};

        m_lastMoveCoord = sf::Vector2i(x, y);

        m_stoneScaleMatrix[y][x] = INITIAL_STONE_SCALE;

        while(!m_uiRedoStack.empty()) m_uiRedoStack.pop();

        showMessage("", MsgType::Success);

        std::string notation = convertCoordsToNotation(x, y);

        m_historyList->addMove(isBlackMove, notation);
        m_timeline->addMove(thinkingTime, isBlackMove, notation);

        if(!result.capturedStones.empty())
        {
            float capturePitch = 0.97f + (std::rand() % 9) / 100.f;

            if(isBlackMove)
            {
                m_soundCaptureBlack.setPitch(capturePitch);
                m_soundCaptureBlack.setVolume(currentVol);
                m_soundCaptureBlack.play();
            }

            else
            {
                m_soundCaptureWhite.setPitch(capturePitch);
                m_soundCaptureWhite.setVolume(currentVol);
                m_soundCaptureWhite.play();
            }
        }
        else
        {
            float placePitch = 0.90f + (std::rand() % 20) / 100.f;

            if(isBlackMove)
            {
                m_soundPlaceBlack.setPitch(placePitch);
                m_soundPlaceBlack.setVolume(currentVol);
                m_soundPlaceBlack.play();
            }
            else
            {
                m_soundPlaceWhite.setPitch(placePitch);
                m_soundPlaceWhite.setVolume(currentVol);
                m_soundPlaceWhite.play();
            }
        }

        std::string colorStr = isBlackMove ? "black" : "white";

        if(m_bot)
        {
            m_bot->syncMove(colorStr, x, y);
        }

        auto bgBot = BotManager::getInstance().getBackgroundBot();
        if(bgBot && bgBot != m_bot)
        {
            bgBot->syncMove(colorStr, x, y);
        }

        if(m_bot && m_mode == GameMode::PlayerVsAI && BotManager::getInstance().isReady())
        {
            m_pendingPlayerMove = { colorStr, x, y, true };
            m_aiShouldMove = true;
        }
    }
    else
    {
        showMessage(result.message, MsgType::Error);

        if(isBlackMove)
        {
            m_soundErrorBlack.setVolume(currentVol);
            m_soundErrorBlack.play();
        }
        else
        {
            m_soundErrorWhite.setVolume(currentVol);
            m_soundErrorWhite.play();
        }
    }
}

void GamePlay::onResignClick()
{
    if(m_gameHasEnded || m_isScoringMode) return;

    bool isBlackTurn = m_logic.isBlacksTurn();
    std::string msg;
    bool blackWins;

    if(isBlackTurn)
    {
        msg = "Black Resigned!";
        blackWins = false;
    }
    else
    {
        msg = "White Resigned!";
        blackWins = true;
    }

    m_endReason = msg;

    m_gameHasEnded = true;
    m_shouldDrawStatusPanel = false;

    m_isScoringMode = true;
    if(m_scoringOverlay)
    {
        m_scoringOverlay->showSimpleResult(msg, blackWins);
    }

    m_turnText.setString("");
    m_messageText.setString("");

    m_historyList->addMove(isBlackTurn, "Resign");
    std::cout << "[GAME] " << msg << "\n";
}

void GamePlay::onHintClick()
{
    if(m_gameHasEnded || m_isScoringMode) return;

    bool isBlack = m_logic.isBlacksTurn();

    if(isBlack && m_blackUsedHint)
    {
        showMessage("Black has already used the hint!", MsgType::Error);
        return;
    }
    if(!isBlack && m_whiteUsedHint)
    {
        showMessage("White has already used the hint!", MsgType::Error);
        return;
    }

    if(isBlack)
    {
        m_blackUsedHint = true;
    }
    else
    {
        m_whiteUsedHint = true;
    }

    requestHintFromBot();

    std::cout << "[HINT] Requesting hint for " << (isBlack ? "Black" : "White") << "...\n";
}

void GamePlay::onPassClick(bool isBotAction)
{
    if(m_gameHasEnded) return;

    if(!isBotAction && m_mode == GameMode::PlayerVsAI && !m_logic.isBlacksTurn())
    {
        showMessage("Not your turn!", MsgType::Error);
        return;
    }

    while(!m_uiRedoStack.empty()) m_uiRedoStack.pop();

    bool currentTurnIsBlack = m_logic.isBlacksTurn();

    MoveResult result = m_logic.attemptPass();

    float thinkingTime = isBotAction ? 1.0f : m_moveTimer.restart().asSeconds();

    m_historyList->addMove(currentTurnIsBlack, "Pass");
    m_timeline->addMove(thinkingTime, currentTurnIsBlack, "Pass");

    if(currentTurnIsBlack)
    {
        m_soundPassBlack.play();
        m_messageText.setString(result.message);
        placeMessageText(m_messageText);
    }
    else
    {
        m_soundPassWhite.play();
        if(m_mode == GameMode::PlayerVsAI) showMessage("Bot passed!", MsgType::Info);
        else showMessage(result.message, MsgType::Info);
    }

    if(result.gameEnded)
    {
        m_gameHasEnded = true;
        m_shouldDrawStatusPanel = false;
        showMessage("Calculating dead stones...", MsgType::Info);
        m_endReason = "SCORING";

        sf::FloatRect textBounds = m_gameOverText.getLocalBounds();
        m_gameOverText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
        m_gameOverText.setPosition(m_window.getSize().x / 2.f, m_window.getSize().y / 2.f);

        auto scoringBot = BotManager::getInstance().getBackgroundBot();
        std::shared_ptr<IBot> finalBot = scoringBot;

        std::vector <DeadStoneInfo> displayStones;
        displayStones.clear();

        if(finalBot)
        {
            m_deadStones = finalBot->getDeadStones();
            auto currentBoard = m_logic.getBoard();

            for(const auto& p : m_deadStones)
            {
                if(p.x >= 0 && p.x < m_boardSize && p.y >= 0 && p.y < m_boardSize)
                {
                    StoneType type = currentBoard[p.y][p.x];
                    TerritoryOwner owner = TerritoryOwner::Neutral;

                    if(type == StoneType::Black)       owner = TerritoryOwner::Black;
                    else if(type == StoneType::White) owner = TerritoryOwner::White;

                    if(owner != TerritoryOwner::Neutral)
                    {
                        displayStones.push_back({ p, owner });
                    }
                }
            }
        }

        std::vector<TerritoryRegion> regions = m_logic.getTerritoryRegions(m_deadStones);

        finalizeScore();

        m_isScoringMode = true;
        if(m_scoringOverlay)
        {
            m_scoringOverlay->startAnimation(regions, displayStones);
        }

        m_turnText.setString("");
        placeMessageText(m_messageText);
    }
    else if(m_mode == GameMode::PlayerVsAI)
    {
        if(!isBotAction)
        {
            m_aiShouldMove = true;
        }
        else
        {
            m_aiShouldMove = false;
        }
    }
}

void GamePlay::onUndoClick()
{
    if(m_mode == GameMode::PlayerVsAI && m_aiShouldMove) return;

    auto performSingleUndo = [&]()
    {
        if(!m_logic.canUndo()) return false;

        UIActionSnapshot snapshot;
        snapshot.isBlack = !m_logic.isBlacksTurn();
        snapshot.notation = m_historyList->getLastMoveNotation();
        snapshot.thinkingTime = m_timeline->getLastMoveTime();
        m_uiRedoStack.push(snapshot);

        m_logic.undo();
        m_historyList->removeLastMove();
        m_timeline->removeLastSegment();

        return true;
    };

    if(m_mode == GameMode::PlayerVsAI)
    {
        if(performSingleUndo()) performSingleUndo();
    }
    else
    {
        performSingleUndo();
    }

    if(m_gameHasEnded) m_gameHasEnded = false;

    showMessage("Undo.", MsgType::Info);

    updateLastMoveMarkerFromHistory();

    std::thread([this]()
    {
        std::lock_guard<std::mutex> lock(m_workerMutex);

        if(auto pachi = std::dynamic_pointer_cast<PachiBot>(m_bot)) pachi->sendCommand("undo");

        auto bgBot = BotManager::getInstance().getBackgroundBot();
        if(bgBot && bgBot != m_bot)
        {
             bgBot->sendCommand("undo");
        }

    }).detach();
}

void GamePlay::onRedoClick()
{
    if(m_mode == GameMode::PlayerVsAI && m_aiShouldMove) return;

    auto performSingleRedo = [&]()
    {
        if(!m_logic.canRedo()) return false;
        if(m_uiRedoStack.empty()) return false;

        m_logic.redo();

        UIActionSnapshot snapshot = m_uiRedoStack.top();
        m_uiRedoStack.pop();

        m_historyList->addMove(snapshot.isBlack, snapshot.notation);
        m_timeline->addMove(snapshot.thinkingTime, snapshot.isBlack, snapshot.notation, false);

        return true;
    };

    if(m_mode == GameMode::PlayerVsAI)
    {
        if(performSingleRedo()) performSingleRedo();
    }
    else
    {
        performSingleRedo();
    }

    showMessage("Redo.", MsgType::Info);

    updateLastMoveMarkerFromHistory();

    fullResyncBots();
}

void GamePlay::performSaveGame(int slotIndex)
{
    std::string fileName = "slot_" + std::to_string(slotIndex);
    std::string txtPath = "assets/saves/" + fileName + ".txt";
    std::string pngPath = "assets/saves/" + fileName + ".png";

    namespace fs = std::filesystem;
    if(!fs::exists("assets/saves")) fs::create_directories("assets/saves");

    SaveInfo info;
    info.slotIndex = slotIndex;
    info.userTitle = "Game " + std::to_string(slotIndex);
    info.timestamp = getCurrentTimestamp();
    info.boardSize = m_boardSize;
    info.modeStr = (m_mode == GameMode::PlayerVsPlayer) ? "PvP" : "PvAI";
    info.status = m_gameHasEnded ? "Finished" : "Ongoing";

    std::string safeReason = m_endReason;
    std::replace(safeReason.begin(), safeReason.end(), '\n', '~');

    info.endReason = safeReason;

    int currentDiff = static_cast<int>(m_difficulty);

    if(m_logic.saveToFile(txtPath, info, m_timeLimitBlack, m_timeLimitWhite, currentDiff))
    {
        saveThumbnail(pngPath);
        showMessage("Saved Slot " + std::to_string(slotIndex));
    }
    else
    {
        showMessage("Save Faled", MsgType::Error);
    }
}

void GamePlay::saveThumbnail(const std::string& filename)
{
    sf::RenderTexture rt;
    if(!rt.create(400, 400)) return;

    rt.clear(sf::Color::Transparent);

    sf::Vector2u texSize = m_boardSprite.getTexture()->getSize();
    float originalSize = (float)texSize.x;
    float scaleFactor = 400.f / originalSize;

    sf::Sprite bgCopy = m_boardSprite;
    bgCopy.setPosition(0.f, 0.f);
    bgCopy.setScale(scaleFactor, scaleFactor);
    rt.draw(bgCopy);

    sf::Sprite stoneCopy;
    float originalPaddingTop = 80.f;
    float originalPaddingBot = 25.f;
    float scaledPaddingTop = originalPaddingTop * scaleFactor;
    float scaledPaddingBot = originalPaddingBot * scaleFactor;
    float scaledBoardWidth = 400.f - (scaledPaddingTop + scaledPaddingBot);
    float scaledCellSpace = scaledBoardWidth / (m_boardSize - 1);
    float stoneScale = (scaledCellSpace / m_blackStoneTex->getSize().x) * 0.95f;

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            StoneType s = m_logic.getStoneAt(x, y);
            if(s == StoneType::Empty) continue;

            stoneCopy.setTexture((s == StoneType::Black) ? *m_blackStoneTex : *m_whiteStoneTex);
            sf::FloatRect bounds = stoneCopy.getLocalBounds();
            stoneCopy.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            stoneCopy.setScale(stoneScale, stoneScale);
            stoneCopy.setPosition(
                std::round(scaledPaddingTop + x * scaledCellSpace),
                std::round(scaledPaddingTop + y * scaledCellSpace)
            );
            rt.draw(stoneCopy);
        }
    }

    rt.display();
    rt.getTexture().copyToImage().saveToFile(filename);
}

std::string GamePlay::getCurrentTimestamp()
{
    std::time_t now = std::time(nullptr);
    char buf[80];
    std::strftime(buf, sizeof(buf), "%d %b %Y, %H:%M", std::localtime(&now));
    return std::string(buf);
}

void GamePlay::performLoadGame(const std::string& filePath)
{
    float loadedTimeB = 0.f;
    float loadedTimeW = 0.f;
    std::string loadedModeStr = "";
    int loadedDifficulty = 1;
    std::string loadedReason = "";

    if(m_logic.loadFromFile(filePath, loadedTimeB, loadedTimeW, loadedModeStr, loadedDifficulty, loadedReason))
    {
        m_historyList->clear();
        m_timeline->clear();
        m_boardSize = m_logic.getBoardSize();

        auto &gs = GlobalSetting::getInstance();
        auto &res = ResourceManager::getInstance();

        m_boardSprite.setTexture(ResourceManager::getInstance().getTexture(gs.getBoardTextureKey(m_boardSize)));
        m_blackStoneTex = &res.getTexture(gs.getStoneTextureKey(true, m_boardSize));
        m_whiteStoneTex = &res.getTexture(gs.getStoneTextureKey(false, m_boardSize));

        float boardDisplaySize = 740.f;
        float paddingTop = 80.f;
        float paddingBot = 25.f;
        float boardClickableSize = boardDisplaySize - (paddingTop + paddingBot);

        m_cellSpacing = boardClickableSize / (m_boardSize - 1);

        m_scoringOverlay = std::make_unique<UI::ScoringOverlay>(
            m_boardSize,
            m_cellSpacing,
            sf::Vector2f(m_boardTopLeftX, m_boardTopLeftY),
            m_font
        );

        m_historyList->clear();
        m_timeline->clear();

        m_gameHasEnded = false;
        m_isScoringMode = false;

        m_timeLimitBlack = loadedTimeB;
        m_timeLimitWhite = loadedTimeW;

        if(m_timeLimitBlack > 0 || m_timeLimitWhite > 0)
        {
            m_isTimeLimitEnabled = true;

            std::string blackKey = GlobalSetting::getInstance().getStoneTextureKey(true, 19);
            std::string whiteKey = GlobalSetting::getInstance().getStoneTextureKey(false, 19);

            m_timerIconBlack.setTexture(ResourceManager::getInstance().getTexture(blackKey));
            m_timerIconWhite.setTexture(ResourceManager::getInstance().getTexture(whiteKey));
        }
        else
        {
            m_isTimeLimitEnabled = false;
        }

        m_timerTextBlack.setString(formatTime(m_timeLimitBlack));
        m_timerTextWhite.setString(formatTime(m_timeLimitWhite));

        m_deadStones.clear();

        if(loadedModeStr == "PvAI")
        {
            m_mode = GameMode::PlayerVsAI;
            m_difficulty = static_cast<AiDifficulty>(loadedDifficulty);
        }
        else m_mode = GameMode::PlayerVsPlayer;

        while(!loadedReason.empty() && (loadedReason.back() == '\n' || loadedReason.back() == '\r' || loadedReason.back() == ' '))
        {
            loadedReason.pop_back();
        }
        std::replace(loadedReason.begin(), loadedReason.end(), '~', '\n');

        m_endReason = loadedReason;
        m_gameHasEnded = (!m_endReason.empty() && m_endReason != "Ongoing");

        if(m_gameHasEnded)
        {
            m_shouldDrawStatusPanel = false;

            if(m_endReason != "SCORING")
            {
                bool blackWon = (m_endReason.find("Black Wins") != std::string::npos ||
                                 m_endReason.find("White Resigned") != std::string::npos);
                m_isScoringMode = true;
                m_scoringOverlay->showSimpleResult(m_endReason, blackWon);
            }
            else
            {
                m_isScoringMode = false;
            }
        }
        else
        {
            m_isScoringMode = false;
            m_shouldDrawStatusPanel = true;
        }

        startGameInitialization();

        std::cout << "Game Loaded!" << std::endl;
    }
}

void GamePlay::handleAiTurn()
{
    if(m_gameHasEnded) return;
    while(!m_uiRedoStack.empty()) m_uiRedoStack.pop();

    bool moveMade = false;
    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            bool isBlackMove = m_logic.isBlacksTurn();
            MoveResult result = m_logic.attemptMove(x, y);
            if(result.success)
            {
                m_lastMoveCoord = sf::Vector2i(x, y);

                m_stoneScaleMatrix[y][x] = INITIAL_STONE_SCALE;

                float vol = GlobalSetting::getInstance().sfxVolume;

                float aiThinkingTime = m_moveTimer.restart().asSeconds();
                std::string notation = convertCoordsToNotation(x, y);
                m_historyList->addMove(isBlackMove, notation);
                m_timeline->addMove(aiThinkingTime, isBlackMove, notation);
                m_messageText.setString("");
                if(!result.capturedStones.empty())
                {
                    float capturePitch = 0.98f + (std::rand() % 5) / 100.f;
                    if(isBlackMove)
                    {
                        m_soundCaptureBlack.setPitch(capturePitch);
                        m_soundCaptureBlack.setVolume(vol);
                        m_soundCaptureBlack.play();
                    }
                    else
                    {
                        m_soundCaptureWhite.setPitch(capturePitch);
                        m_soundCaptureWhite.setVolume(vol);
                        m_soundCaptureWhite.play();
                    }
                }
                else
                {
                    float placePitch = 0.95f + (std::rand() % 11) / 100.f;

                    if(isBlackMove)
                    {
                        m_soundPlaceBlack.setPitch(placePitch);
                        m_soundPlaceBlack.setVolume(vol);
                        m_soundPlaceBlack.play();
                    }
                    else
                    {
                        m_soundPlaceWhite.setPitch(placePitch);
                        m_soundPlaceWhite.setVolume(vol);
                        m_soundPlaceWhite.play();
                    }
                }
                moveMade = true;
                break;
            }
        }
        if(moveMade) break;
    }
    if(!moveMade)
    {
        bool isBlackMove = m_logic.isBlacksTurn();
        MoveResult result = m_logic.attemptPass();
        float aiThinkingTime = m_moveTimer.restart().asSeconds();
        m_historyList->addMove(isBlackMove, "Pass");
        m_timeline->addMove(aiThinkingTime, isBlackMove, "Pass");
        showMessage(result.message, MsgType::Info);
        if(isBlackMove) m_soundPassBlack.play();
        else m_soundPassWhite.play();

        if(result.gameEnded)
        {
            m_gameHasEnded = true;
            m_turnText.setString("");
        }
    }
}

sf::Vector2i GamePlay::getBoardCoordsFromMouse(int mouseX, int mouseY)
{
    sf::Vector2f mousePos = m_window.mapPixelToCoords({mouseX, mouseY});
    float relativeX = mousePos.x - m_boardTopLeftX;
    float relativeY = mousePos.y - m_boardTopLeftY;
    int x = static_cast<int>(std::round(relativeX / m_cellSpacing));
    int y = static_cast<int>(std::round(relativeY / m_cellSpacing));
    if(x < 0 || x >= m_boardSize || y < 0 || y >= m_boardSize) return {-1, -1};
    float clickRadius = m_cellSpacing / 2.f;
    float distSq = std::pow(relativeX - (x * m_cellSpacing), 2) + std::pow(relativeY - (y * m_cellSpacing), 2);
    if(distSq > std::pow(clickRadius, 2)) return {-1, -1};
    return {x, y};
}

std::string GamePlay::convertCoordsToNotation(int x, int y)
{
    const std::string COLS = "ABCDEFGHJKLMNOPQRST";
    if(x < 0 || x >= COLS.length()) return "??";
    char colChar = COLS[x];
    std::string rowStr = std::to_string(y + 1);
    return colChar + rowStr;
}

void GamePlay::onPauseClick()
{
    m_requestedState = GameStateType::PauseMenu;
}

void GamePlay::finalizeScore()
{
    int blackStones = 0;
    int whiteStones = 0;
    int blackTerritory = 0;
    int whiteTerritory = 0;

    float komi = GlobalSetting::getInstance().getKomiValue();

    const auto& board = m_logic.getBoard();

    for(int y = 0; y < m_boardSize; ++y)
    {
        for(int x = 0; x < m_boardSize; ++x)
        {
            if(board[y][x] == StoneType::Black)
            {
                blackStones++;
            }
            else if(board[y][x] == StoneType::White)
            {
                whiteStones++;
            }
        }
    }

    std::vector<TerritoryRegion> regions = m_logic.getTerritoryRegions(m_deadStones);

    for(const auto& region : regions)
    {
        if(region.owner == TerritoryOwner::Black)
        {
            blackTerritory += region.points.size();
        }
        else if(region.owner == TerritoryOwner::White)
        {
            whiteTerritory += region.points.size();
        }
    }

    ScoreData data;
    data.blackStones = blackStones;
    data.blackTerritory = blackTerritory;
    data.whiteStones = whiteStones;
    data.whiteTerritory = whiteTerritory;
    data.komi = komi;

    if(m_scoringOverlay)
    {
        m_scoringOverlay -> setScoreData(data);
    }

    m_gameOverText.setString("");
}

std::string GamePlay::formatTime(float seconds)
{
    if(seconds == -1.f)
    {
        return "--:--";
    }

    int totalSec = (int)seconds;
    int m = totalSec / 60;
    int s = totalSec % 60;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << m << ":"
       << std::setfill('0') << std::setw(2) << s;
    return ss.str();
}

void GamePlay::updateTimerDisplay()
{
    bool isNoLimit = GlobalSetting::getInstance().getTimeLimitInSeconds() == -1;

    float blackTime = isNoLimit ? -1.f : m_timeLimitBlack;
    float whiteTime = isNoLimit ? -1.f : m_timeLimitWhite;

    m_timerTextBlack.setString(formatTime(blackTime));
    m_timerTextWhite.setString(formatTime(whiteTime));
}

void GamePlay::updateOnBoardCount()
{
    GameLogic::StoneCount counts = m_logic.getStoneCount();

    m_onBoardTextBlack.setString(std::to_string(counts.blackStones));
    m_onBoardTextWhite.setString(std::to_string(counts.whiteStones));
}

sf::Vector2i GamePlay::parseNotationToCoords(const std::string& notation)
{
    if(notation.length() < 2 || notation == "Pass") return {-1, -1};

    const std::string COLS = "ABCDEFGHJKLMNOPQRST";
    char colChar = notation[0];
    std::string rowStr = notation.substr(1);

    int x = COLS.find(colChar);
    int y = -1;
    try
    {
        y = std::stoi(rowStr) - 1;
    }
    catch(...)
    {
        return {-1, -1};
    }

    if(x != std::string::npos && y >= 0 && y < m_boardSize)
    {
        return {x, y};
    }
    return {-1, -1};
}

void GamePlay::updateLastMoveMarkerFromHistory()
{
    std::string lastMoveNote = m_historyList->getLastMoveNotation();

    if(lastMoveNote.empty() || lastMoveNote == "Pass")
    {
        m_lastMoveCoord = {-1, -1};
    }
    else
    {
        m_lastMoveCoord = parseNotationToCoords(lastMoveNote);
    }
}
