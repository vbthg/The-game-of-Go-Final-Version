#include "GamePlay.h"
#include "ResourceManager.h"
#include "GlobalSetting.h"
#include "ScaleEffect.h"
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <filesystem>

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
    m_timerPanel(ResourceManager::getInstance().getTexture("gameplay_timer_bg")),


    m_boardSprite(ResourceManager::getInstance().getTexture( GlobalSetting::getInstance().getBoardTextureKey(boardSize) )),
    m_blackStoneTex(&ResourceManager::getInstance().getTexture( GlobalSetting::getInstance().getStoneTextureKey(true, boardSize) )),
    m_whiteStoneTex(&ResourceManager::getInstance().getTexture( GlobalSetting::getInstance().getStoneTextureKey(false, boardSize) )),


    m_font(ResourceManager::getInstance().getFont("main_font")),
    m_pauseButton(ResourceManager::getInstance().getTexture("gameplay_pause_btn"), {1542.f, 54.f}, true),
    m_passButton(ResourceManager::getInstance().getTexture("gameplay_pass_btn"), {1256.f, 209.f}, true),
    m_undoBtn(ResourceManager::getInstance().getTexture("gameplay_undo_btn"), {1375.f, 209.f}, true),
    m_redoBtn(ResourceManager::getInstance().getTexture("gameplay_redo_btn"), {1494.f, 209.f}, true)
{

    float boardDisplaySize = 740.f;
    float boardDisplayX = 430.f;
    float boardDisplayY = 123.f;
    float paddingTop = 80.f;
    float paddingBot = 25.f;

    m_boardSprite.setPosition(boardDisplayX, boardDisplayY);
    m_boardTopLeftX = boardDisplayX + paddingTop;
    m_boardTopLeftY = boardDisplayY + paddingTop;
    float boardClickableSize = boardDisplaySize - (paddingTop + paddingBot);
    m_cellSpacing = boardClickableSize / (m_boardSize - 1);


    m_messageText.setFont(m_font);
    m_messageText.setCharacterSize(20);
    m_messageText.setFillColor(sf::Color::Red);
    m_messageText.setPosition(boardDisplayX, boardDisplayY + boardDisplaySize + 10.f);

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





    m_historyList = std::make_unique<UI::HistoryList>(
        sf::Vector2f(214.f, 440.f),
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
    m_turnactionBackground.setPosition(sf::Vector2f(1375.f, 209.f));

    m_soundPlace.setBuffer(ResourceManager::getInstance().getSoundBuffer("place_stone_01"));
    m_soundCapture.setBuffer(ResourceManager::getInstance().getSoundBuffer("capture_stone"));
    m_soundPass.setBuffer(ResourceManager::getInstance().getSoundBuffer("pass_move"));
    m_soundError.setBuffer(ResourceManager::getInstance().getSoundBuffer("error_move"));

    sf::Sound s1, s2, s3;
    s1.setBuffer(ResourceManager::getInstance().getSoundBuffer("place_stone_01"));

    m_randomPlaceSounds.push_back(s1);
//    m_randomPlaceSounds.push_back(s2);
//    m_randomPlaceSounds.push_back(s3);

    float vol = GlobalSetting::getInstance().sfxVolume;
    m_soundPlace.setVolume(vol);
    m_soundCapture.setVolume(vol);
    m_soundPass.setVolume(vol);
    m_soundError.setVolume(vol);

    for(auto& s : m_randomPlaceSounds) s.setVolume(vol);


    int limitSeconds = GlobalSetting::getInstance().getTimeLimitInSeconds();

    if (limitSeconds == -1) {
        m_isTimeLimitEnabled = false;
        m_timeLimitBlack = 0;
        m_timeLimitWhite = 0;
    } else {
        m_isTimeLimitEnabled = true;
        m_timeLimitBlack = (float)limitSeconds;
        m_timeLimitWhite = (float)limitSeconds;
    }


    bounds = m_timerPanel.getLocalBounds();
    m_timerPanel.setOrigin(sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));
    m_timerPanel.setTexture(ResourceManager::getInstance().getTexture("gameplay_timer_bg"));
    m_timerPanel.setPosition(1373.f, 427.f);


    std::string blackKey = GlobalSetting::getInstance().getStoneTextureKey(true, 19);
    std::string whiteKey = GlobalSetting::getInstance().getStoneTextureKey(false, 19);

    m_timerIconBlack.setTexture(ResourceManager::getInstance().getTexture(blackKey));
    m_timerIconWhite.setTexture(ResourceManager::getInstance().getTexture(whiteKey));

    bounds = m_timerIconBlack.getLocalBounds();
    m_timerIconBlack.setOrigin(sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));
    bounds = m_timerIconWhite.getLocalBounds();
    m_timerIconWhite.setOrigin(sf::Vector2f(bounds.width / 2.f, bounds.height / 2.f));

    m_timerIconBlack.setPosition(1368.f, 408.f);
    m_timerIconWhite.setPosition(1368.f, 447.f);


    m_timerTextBlack.setFont(m_font);
    m_timerTextBlack.setCharacterSize(20);
    m_timerTextBlack.setFillColor(sf::Color::White);
    m_timerTextBlack.setPosition(1396.f, 395.f);

    m_timerTextWhite.setFont(m_font);
    m_timerTextWhite.setCharacterSize(20);
    m_timerTextWhite.setFillColor(sf::Color::White);
    m_timerTextWhite.setPosition(1396.f, 435.f);


    m_pauseButton.setOnClick([this]() { this->onPauseClick(); });
    m_passButton.setOnClick([this]() { this->onPassClick(); });
    m_undoBtn.setOnClick([this]() { this->onUndoClick(); });
    m_redoBtn.setOnClick([this]() { this->onRedoClick(); });

    m_moveTimer.restart();


    m_isScoringMode = false;
    m_scoringOverlay = std::make_unique<ScoringOverlay>
    (
        m_boardSize, m_cellSpacing,
        sf::Vector2f(m_boardTopLeftX, m_boardTopLeftY),
        m_font
    );
}


void GamePlay::handleEvent(sf::Event& event)
{
    if (event.type == sf::Event::Closed)
    {
        m_window.close();
        return;
    }

    if (m_isScoringMode && m_scoringOverlay && m_scoringOverlay->isScoreboardVisible())
    {
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mousePos = m_window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});

            if (m_scoringOverlay->contains(mousePos))
            {
                m_requestedState = GameStateType::MainMenu;
            }
        }
        return;
    }

    if (m_gameHasEnded)
    {

        sf::Vector2f mousePos = m_window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
        sf::FloatRect bounds = m_winText.getGlobalBounds();

        if(bounds.contains(mousePos.x, mousePos.y))
        {
            m_requestedState = GameStateType::MainMenu;
        }

        return;
    }

    m_pauseButton.handleEvent(event, m_window);
    m_passButton.handleEvent(event, m_window);
    m_undoBtn.handleEvent(event, m_window);
    m_redoBtn.handleEvent(event, m_window);
    m_historyList->handleEvent(event, m_window);
    m_timeline->handleEvent(event, m_window);

    if (event.type == sf::Event::MouseButtonPressed)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mousePos = m_window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});

            if (m_pauseButton.getSprite().getGlobalBounds().contains(mousePos) ||
                m_passButton.getSprite().getGlobalBounds().contains(mousePos) ||
                m_undoBtn.getSprite().getGlobalBounds().contains(mousePos) ||
                m_redoBtn.getSprite().getGlobalBounds().contains(mousePos))
            { }
            else
            {
                onBoardClick(event.mouseButton.x, event.mouseButton.y);
            }
        }
    }
}


GameStateType GamePlay::update(float deltaTime)
{
    if (m_isScoringMode && m_scoringOverlay)
    {
        m_scoringOverlay->update(deltaTime);

        if (m_scoringOverlay->isAnimationFinished())
        {
            if (m_gameOverText.getString() == "CALCULATING TERRITORY...")
            {
                finalizeScore();
            }
        }
    }

    if (!m_gameHasEnded && m_isTimeLimitEnabled && !m_isScoringMode)
    {
        if (m_logic.isBlacksTurn())
        {
            m_timeLimitBlack -= deltaTime;
            if (m_timeLimitBlack <= 0.f)
            {
                m_timeLimitBlack = 0.f;
                m_gameHasEnded = true;

                m_winText.setString("TIME OUT! WHITE WINS!");
                m_winText.setFillColor(sf::Color::Red);

                sf::FloatRect b = m_winText.getLocalBounds();
                m_winText.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
                m_winText.setPosition(m_winPanel.getPosition());

                m_turnText.setString("");
            }
        }
        else
        {
            m_timeLimitWhite -= deltaTime;
            if (m_timeLimitWhite <= 0.f)
            {
                m_timeLimitWhite = 0.f;
                m_gameHasEnded = true;

                m_winText.setString("TIME OUT! BLACK WINS!");
                m_winText.setFillColor(sf::Color::Red);

                sf::FloatRect b = m_winText.getLocalBounds();
                m_winText.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
                m_winText.setPosition(m_winPanel.getPosition());

                m_turnText.setString("");
            }
        }

        m_timerTextBlack.setString(formatTime(m_timeLimitBlack));
        m_timerTextWhite.setString(formatTime(m_timeLimitWhite));
    }


    if (!m_isScoringMode || (m_scoringOverlay && !m_scoringOverlay->isScoreboardVisible()))
    {
        m_pauseButton.update(m_window);
        m_passButton.update(m_window);
        m_undoBtn.update(m_window);
        m_redoBtn.update(m_window);
        if(m_historyList) m_historyList->update(m_window);
    }
    if(m_timeline) m_timeline->update(deltaTime, m_window);


    if (!m_gameHasEnded)
    {
        m_turnText.setString(m_logic.isBlacksTurn() ? "Turn: Black" : "Turn: White");
    }


    if (!m_isScoringMode)
    {
        bool isHovering = false;
        if (m_pauseButton.isHoveredAndInteractive() || m_passButton.isHoveredAndInteractive() ||
            m_undoBtn.isHoveredAndInteractive() || m_redoBtn.isHoveredAndInteractive()) isHovering = true;

        sf::Cursor cursor;
        if (isHovering) { if (cursor.loadFromSystem(sf::Cursor::Hand)) m_window.setMouseCursor(cursor); }
        else { if (cursor.loadFromSystem(sf::Cursor::Arrow)) m_window.setMouseCursor(cursor); }
    }

    // 6. Logic AI
    if (!m_gameHasEnded && m_mode == GameMode::PlayerVsAI && m_aiShouldMove)
    {
        m_aiThinkTimer += deltaTime;
        if (m_aiThinkTimer > 0.5f)
        {
            m_aiThinkTimer = 0.f;
            m_aiShouldMove = false;
            handleAiTurn();
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
    drawStones();

    if (m_isTimeLimitEnabled)
    {
        m_window.draw(m_timerPanel);

        m_window.draw(m_timerIconBlack);
        m_window.draw(m_timerIconWhite);

        m_window.draw(m_timerTextBlack);
        m_window.draw(m_timerTextWhite);
    }

    if (m_isScoringMode)
    {
        m_scoringOverlay -> draw(m_window);
    }
    else if (m_gameHasEnded)
    {
        m_window.draw(m_winPanel);
        m_window.draw(m_winText);
    }
    else
    {
        m_window.draw(m_turnText);
    }


    m_pauseButton.draw(m_window);
    m_passButton.draw(m_window);
    m_undoBtn.draw(m_window);
    m_redoBtn.draw(m_window);
    m_historyList -> draw(m_window);
    m_timeline -> draw(m_window);
    m_window.draw(m_messageText);
}

void GamePlay::drawStones()
{
    sf::Sprite stoneSprite;

    for (int y = 0; y < m_boardSize; ++y)
    {
        for (int x = 0; x < m_boardSize; ++x)
        {
            StoneType stone = m_logic.getStoneAt(x, y);
            if (stone == StoneType::Empty) continue;

            stoneSprite.setTexture((stone == StoneType::Black) ? *m_blackStoneTex : *m_whiteStoneTex);
            sf::FloatRect texBounds = stoneSprite.getLocalBounds();
            stoneSprite.setOrigin(texBounds.width / 2.f, texBounds.height / 2.f);
            stoneSprite.setPosition(std::round(m_boardTopLeftX + (x * m_cellSpacing)), std::round(m_boardTopLeftY + (y * m_cellSpacing)));
            m_window.draw(stoneSprite);
        }
    }
}


void GamePlay::onBoardClick(int mouseX, int mouseY)
{
    if (m_gameHasEnded) return;
    if (m_mode == GameMode::PlayerVsAI && !m_logic.isBlacksTurn())
    {
         m_messageText.setString("Not your turn!");
         return;
    }

    sf::Vector2i boardCoords = getBoardCoordsFromMouse(mouseX, mouseY);
    int x = boardCoords.x;
    int y = boardCoords.y;
    if (x == -1 || y == -1) return;

    bool isBlackMove = m_logic.isBlacksTurn();
    float thinkingTime = m_moveTimer.restart().asSeconds();

    MoveResult result = m_logic.attemptMove(x, y);
    float currentVol = GlobalSetting::getInstance().sfxVolume;

    if (result.success)
    {
        while (!m_uiRedoStack.empty()) m_uiRedoStack.pop();

        m_messageText.setString("");
        std::string notation = convertCoordsToNotation(x, y);

        m_historyList -> addMove(isBlackMove, notation);
        m_timeline -> addMove(thinkingTime, isBlackMove, notation);

        if (!result.capturedStones.empty())
        {
            float capturePitch = 0.90f + (std::rand() % 15) / 100.f;
            m_soundCapture.setPitch(capturePitch);
            m_soundCapture.setVolume(currentVol);
            m_soundCapture.play();
        }
        else
        {
            if (!m_randomPlaceSounds.empty())
            {
                int randomIndex = rand() % m_randomPlaceSounds.size();
                m_randomPlaceSounds[randomIndex].setPitch(0.90f + (rand() % 20) / 100.f);
                m_randomPlaceSounds[randomIndex].setVolume(currentVol);
                m_randomPlaceSounds[randomIndex].play();
            }
            else
            {
                float placePitch = 0.90f + (std::rand() % 20) / 100.f;
                m_soundPlace.setPitch(placePitch);
                m_soundPlace.setVolume(currentVol);
                m_soundPlace.play();
            }
        }

        if (m_mode == GameMode::PlayerVsAI) m_aiShouldMove = true;
    }
    else
    {
        m_messageText.setString(result.message);
        m_soundError.setVolume(currentVol);
        m_soundError.play();
    }
}

void GamePlay::onPassClick()
{
    if (m_gameHasEnded) return;
    if (m_mode == GameMode::PlayerVsAI && !m_logic.isBlacksTurn())
    {
         m_messageText.setString("Not your turn!");
         return;
    }

    while (!m_uiRedoStack.empty()) m_uiRedoStack.pop();

    bool isBlackMove = m_logic.isBlacksTurn();
    float thinkingTime = m_moveTimer.restart().asSeconds();
    MoveResult result = m_logic.attemptPass();

    m_historyList -> addMove(isBlackMove, "Pass");
    m_timeline -> addMove(thinkingTime, isBlackMove, "Pass");
    m_messageText.setString(result.message);
    m_soundPass.play();

    if (result.gameEnded)
    {
        m_gameHasEnded = true;
        m_gameOverText.setString("CALCULATING TERRITORY...");

        sf::FloatRect textBounds = m_gameOverText.getLocalBounds();
        m_gameOverText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
        m_gameOverText.setPosition(m_window.getSize().x / 2.f, m_window.getSize().y / 2.f);

        m_isScoringMode = true;

        auto territoryMap = m_logic.calculateTerritory();

        auto currentBoard = m_logic.getBoard();

        if (m_scoringOverlay)
        {
            m_scoringOverlay -> startAnimation(territoryMap, currentBoard);
        }

        m_turnText.setString("");
    }
    else if (m_mode == GameMode::PlayerVsAI)
    {
        m_aiShouldMove = true;
    }
}

void GamePlay::onUndoClick()
{
    if (m_mode == GameMode::PlayerVsAI && m_aiShouldMove) return;

    auto performSingleUndo = [&]()
    {
        if (!m_logic.canUndo()) return false;

        UIActionSnapshot snapshot;
        snapshot.isBlack = !m_logic.isBlacksTurn();
        snapshot.notation = m_historyList->getLastMoveNotation();
        snapshot.thinkingTime = m_timeline->getLastMoveTime();
        m_uiRedoStack.push(snapshot);

        m_logic.undo();
        m_historyList -> removeLastMove();
        m_timeline -> removeLastSegment();

        return true;
    };

    if (m_mode == GameMode::PlayerVsAI)
    {
        if (performSingleUndo()) performSingleUndo();
    }
    else
    {
        performSingleUndo();
    }

    if (m_gameHasEnded) m_gameHasEnded = false;
    m_messageText.setString("Undo.");
}

void GamePlay::onRedoClick()
{
    if (m_mode == GameMode::PlayerVsAI && m_aiShouldMove) return;

    auto performSingleRedo = [&]()
    {
        if (!m_logic.canRedo()) return false;
        if (m_uiRedoStack.empty()) return false;

        m_logic.redo();

        UIActionSnapshot snapshot = m_uiRedoStack.top();
        m_uiRedoStack.pop();

        m_historyList->addMove(snapshot.isBlack, snapshot.notation);
        m_timeline->addMove(snapshot.thinkingTime, snapshot.isBlack, snapshot.notation, false);

        return true;
    };

    if (m_mode == GameMode::PlayerVsAI)
    {
        if (performSingleRedo()) performSingleRedo();
    }
    else
    {
        performSingleRedo();
    }
    m_messageText.setString("Redo.");
}


void GamePlay::performSaveGame(int slotIndex)
{
    std::string fileName = "slot_" + std::to_string(slotIndex);
    std::string txtPath = "assets/saves/" + fileName + ".txt";
    std::string pngPath = "assets/saves/" + fileName + ".png";

    namespace fs = std::filesystem;
    if (!fs::exists("assets/saves")) fs::create_directories("assets/saves");

    SaveInfo info;
    info.slotIndex = slotIndex;
    info.userTitle = "Game " + std::to_string(slotIndex);
    info.timestamp = getCurrentTimestamp();
    info.boardSize = m_boardSize;
    info.modeStr = (m_mode == GameMode::PlayerVsPlayer) ? "PvP" : "PvAI";
    info.status = m_gameHasEnded ? "Finished" : "Ongoing";

    if (m_logic.saveToFile(txtPath, info, m_timeLimitBlack, m_timeLimitWhite))
    {
        saveThumbnail(pngPath);
        m_messageText.setString("Saved Slot " + std::to_string(slotIndex));
        std::cout << "Saved: " << txtPath << std::endl;
    }
    else
    {
        m_messageText.setString("Save Failed!");
    }
}

void GamePlay::saveThumbnail(const std::string& filename)
{
    sf::RenderTexture rt;
    if (!rt.create(400, 400)) return;

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

    for (int y = 0; y < m_boardSize; ++y)
    {
        for (int x = 0; x < m_boardSize; ++x)
        {
            StoneType s = m_logic.getStoneAt(x, y);
            if (s == StoneType::Empty) continue;

            stoneCopy.setTexture((s == StoneType::Black) ? *m_blackStoneTex : *m_whiteStoneTex);
            sf::FloatRect bounds = stoneCopy.getLocalBounds();
            stoneCopy.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
            stoneCopy.setScale(stoneScale, stoneScale);
            stoneCopy.setPosition
            (
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

    if (m_logic.loadFromFile(filePath, loadedTimeB, loadedTimeW))
    {
        m_historyList -> clear();
        m_timeline -> clear();
        m_boardSize = m_logic.getBoardSize();

        std::string suffix = "_" + std::to_string(m_boardSize) + "x" + std::to_string(m_boardSize);
        m_boardSprite.setTexture(ResourceManager::getInstance().getTexture("gameplay_board" + suffix));
        m_blackStoneTex = &ResourceManager::getInstance().getTexture("gameplay_stone_black" + suffix);
        m_whiteStoneTex = &ResourceManager::getInstance().getTexture("gameplay_stone_white" + suffix);

        float boardDisplaySize = 740.f;
        float paddingTop = 80.f;
        float paddingBot = 25.f;
        float boardClickableSize = boardDisplaySize - (paddingTop + paddingBot);

        m_cellSpacing = boardClickableSize / (m_boardSize - 1);

        m_scoringOverlay = std::make_unique<ScoringOverlay>
        (
            m_boardSize,
            m_cellSpacing,
            sf::Vector2f(m_boardTopLeftX, m_boardTopLeftY),
            m_font
        );

        m_historyList -> clear();
        m_timeline -> clear();

        m_gameHasEnded = false;
        m_isScoringMode = false;
        m_turnText.setString(m_logic.isBlacksTurn() ? "Turn: Black" : "Turn: White");
        m_gameOverText.setString("");

        m_timeLimitBlack = loadedTimeB;
        m_timeLimitWhite = loadedTimeW;

        if (m_timeLimitBlack > 0 || m_timeLimitWhite > 0)
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

        std::cout << "Game Loaded!" << std::endl;
    }
}

void GamePlay::handleAiTurn()
{
    if (m_gameHasEnded) return;
    while (!m_uiRedoStack.empty()) m_uiRedoStack.pop();

    bool moveMade = false;
    for (int y = 0; y < m_boardSize; ++y) {
        for (int x = 0; x < m_boardSize; ++x) {
            bool isBlackMove = m_logic.isBlacksTurn();
            MoveResult result = m_logic.attemptMove(x, y);
            if (result.success) {
                float aiThinkingTime = m_moveTimer.restart().asSeconds();
                std::string notation = convertCoordsToNotation(x, y);
                m_historyList->addMove(isBlackMove, notation);
                m_timeline->addMove(aiThinkingTime, isBlackMove, notation);
                m_messageText.setString("");
                if (!result.capturedStones.empty())
                {
                    float capturePitch = 0.98f + (std::rand() % 5) / 100.f;
                    m_soundCapture.setPitch(capturePitch);
                    m_soundCapture.play();
                }
                else
                {
                    float placePitch = 0.95f + (std::rand() % 11) / 100.f;

                    if (!m_randomPlaceSounds.empty())
                    {
                        int idx = std::rand() % m_randomPlaceSounds.size();
                        m_randomPlaceSounds[idx].setPitch(placePitch);
                        m_randomPlaceSounds[idx].play();
                    }
                    else
                    {
                        m_soundPlace.setPitch(placePitch);
                        m_soundPlace.play();
                    }
                }
                moveMade = true;
                break;
            }
        }
        if (moveMade) break;
    }
    if (!moveMade)
    {
        bool isBlackMove = m_logic.isBlacksTurn();
        MoveResult result = m_logic.attemptPass();
        float aiThinkingTime = m_moveTimer.restart().asSeconds();
        m_historyList -> addMove(isBlackMove, "Pass");
        m_timeline -> addMove(aiThinkingTime, isBlackMove, "Pass");
        m_messageText.setString(result.message);
        m_soundPass.play();
        if (result.gameEnded)
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
    if (x < 0 || x >= m_boardSize || y < 0 || y >= m_boardSize) return {-1, -1};
    float clickRadius = m_cellSpacing / 2.f;
    float distSq = std::pow(relativeX - (x * m_cellSpacing), 2) + std::pow(relativeY - (y * m_cellSpacing), 2);
    if (distSq > std::pow(clickRadius, 2)) return {-1, -1};
    return {x, y};
}

std::string GamePlay::convertCoordsToNotation(int x, int y)
{
    const std::string COLS = "ABCDEFGHJKLMNOPQRST";
    if (x < 0 || x >= COLS.length()) return "??";
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
    int blackScore = 0;
    int whiteScore = 0;
    int blackTerritory = 0;
    int whiteTerritory = 0;

    float komi = GlobalSetting::getInstance().getKomiValue();

    const auto& board = m_logic.getBoard();
    auto territoryMap = m_logic.calculateTerritory();

    for (int y = 0; y < m_boardSize; ++y)
    {
        for (int x = 0; x < m_boardSize; ++x)
        {
            if (board[y][x] == StoneType::Black) blackScore++;
            else if (board[y][x] == StoneType::White) whiteScore++;

            if (territoryMap[y][x] == TerritoryOwner::Black) blackTerritory++;
            else if (territoryMap[y][x] == TerritoryOwner::White) whiteTerritory++;
        }
    }

    ScoreData data;
    data.blackStones = blackScore;
    data.blackTerritory = blackTerritory;
    data.whiteStones = whiteScore;
    data.whiteTerritory = whiteTerritory;
    data.komi = komi;

    if (m_scoringOverlay)
    {
        m_scoringOverlay -> setScoreData(data);
    }

    m_gameOverText.setString("");
}

std::string GamePlay::formatTime(float seconds)
{
    int totalSec = (int)seconds;
    int m = totalSec / 60;
    int s = totalSec % 60;

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << m << ":"
       << std::setfill('0') << std::setw(2) << s;
    return ss.str();
}
