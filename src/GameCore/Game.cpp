#include "Game.h"
#include "MainMenu.h"
#include "NewGame.h"
#include "SizeSelection.h"
#include "GamePlay.h"
#include "Setting.h"
#include "About.h"
#include "Pause.h"
#include "GlobalSetting.h"
#include "SavedGame.h"
#include <filesystem>

const float DELAY_TRANSITION = 0.15f;

Game::Game(unsigned int width, unsigned int height, const std::string& title) :
    m_window(sf::VideoMode(width, height), title),
    m_currentState(nullptr),
    m_overlayState(nullptr),
    m_nextGameMode(GameMode::PlayerVsPlayer),
    m_nextDifficulty(AiDifficulty::Easy),
    m_nextBoardSize(19),
    m_nextLoadFile(""),
    m_currentSlotIndex(-1)
{
    m_window.setFramerateLimit(60);
    m_dimOverlay.setSize(sf::Vector2f((float)width, (float)height));
    m_dimOverlay.setFillColor(sf::Color(0, 0, 0, 150));

    m_currentState = createState(GameStateType::MainMenu);

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    int centerX = (int)(desktop.width - width) / 2;
    int centerY = (int)(desktop.height - height) / 2 - 50;

    if(centerY < 0)
    {
        centerY = 0;
    }
    if(centerX < 0)
    {
        centerX = 0;
    }

    m_window.setPosition(sf::Vector2i(centerX, centerY));

    auto& gs = GlobalSetting::getInstance();
    auto& rm = ResourceManager::getInstance();

    rm.setMusicVolume(gs.musicVolume);

    rm.playMusic(gs.musicThemeIndex);
}

void Game::run()
{
    while(m_window.isOpen())
    {
        sf::Time deltaTime = m_clock.restart();

        handleEvents();
        update(deltaTime);
        draw();
    }
}

void Game::handleEvents()
{
    if(m_pendingRequest != GameStateType::NoChange)
    {
        return;
    }

    sf::Event event;
    while(m_window.pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
        {
            m_window.close();
            return;
        }

        if(m_overlayState)
        {
            m_overlayState->handleEvent(event);
        }
        else if(m_currentState)
        {
            m_currentState->handleEvent(event);
        }
    }
}

void Game::update(sf::Time deltaTime)
{
    float dtSeconds = deltaTime.asSeconds();
    m_delayTimer -= dtSeconds;
    if(m_delayTimer < 0.f)
    {
        m_delayTimer = 0.f;
    }

    if(m_pendingRequest != GameStateType::NoChange)
    {
        if(m_overlayState)
        {
            m_overlayState -> update(dtSeconds);
        }
        if(m_currentState)
        {
            m_currentState -> update(dtSeconds);
        }

        if(m_delayTimer <= 0.f)
        {
            if(m_isFromOverlay)
            {
                handleOverlayRequest(m_pendingRequest);
            }
            else
            {
                handleMainRequest(m_pendingRequest);
            }
            m_pendingRequest = GameStateType::NoChange;
        }
        return;
    }

    if(m_overlayState)
    {
        GameStateType request = m_overlayState->update(dtSeconds);
        if(request != GameStateType::NoChange)
        {
            m_delayTimer = DELAY_TRANSITION;
            m_pendingRequest = request;
            m_isFromOverlay = true;
        }

        return;
    }

    if(m_currentState)
    {
        GameStateType request = m_currentState->update(dtSeconds);
        if(request != GameStateType::NoChange)
        {
            m_delayTimer = DELAY_TRANSITION;
            m_pendingRequest = request;
            m_isFromOverlay = false;
        }
    }
}

void Game::draw()
{
    m_window.clear(sf::Color::Black);

    if(m_currentState)
    {
        m_currentState->draw();
    }

    if(m_overlayState)
    {
        m_window.draw(m_dimOverlay);
        m_overlayState->draw();
    }

    m_window.display();
}

int Game::getNextAvailableSlotIndex()
{
    int idx = 1;
    namespace fs = std::filesystem;
    while(fs::exists("assets/saves/slot_" + std::to_string(idx) + ".txt"))
    {
        idx++;
    }
    return idx;
}

void Game::handleMainRequest(GameStateType request)
{
    if(m_delayTimer > 0.f)
    {
        return;
    }

    if(request == GameStateType::NoChange)
    {
        return;
    }
    if(request == GameStateType::Quit)
    {
        m_window.close();
        return;
    }

    if(request == GameStateType::MainMenu || request == GameStateType::NewGame)
    {
        m_currentState = createState(request);
    }
    else if(request == GameStateType::SizeSelect)
    {
        NewGame* ng = dynamic_cast<NewGame*>(m_currentState.get());
        if(ng)
        {
            m_nextGameMode = ng->getSelectedGameMode();
            transitionPos = ng->getLastClickPos();
        }
        m_currentState = createState(request);
    }
    else if(request == GameStateType::GamePlay)
    {
        SizeSelection* ss = dynamic_cast<SizeSelection*>(m_currentState.get());
        if(ss)
        {
            m_nextBoardSize = ss->getSelectedSize();
            m_nextDifficulty = ss->getSelectedDifficulty();
        }

        m_currentSlotIndex = -1;

        m_currentState = createState(request);
    }
    else if(request == GameStateType::SavedGame)
    {
        SavedGame* saveMenu = dynamic_cast<SavedGame*>(m_currentState.get());

        if(saveMenu)
        {
            m_nextLoadFile = saveMenu->getFileToLoad();

            std::string filename = std::filesystem::path(m_nextLoadFile).stem().string();
            try
            {
                m_currentSlotIndex = std::stoi(filename.substr(5));
            }
            catch(...)
            {
                m_currentSlotIndex = -1;
            }

            auto gp = std::make_unique<GamePlay>(m_window, 19, GameMode::PlayerVsPlayer, AiDifficulty::Easy);
            gp->performLoadGame(m_nextLoadFile);
            m_currentState = std::move(gp);
        }
        else
        {
            m_currentState = createState(GameStateType::SavedGame);
        }
    }
    else if(request == GameStateType::PauseMenu || request == GameStateType::Settings || request == GameStateType::About)
    {
        m_overlayState = createState(request);
    }
}

void Game::handleOverlayRequest(GameStateType request)
{
    if(request == GameStateType::NoChange)
    {
        return;
    }

    if(request == GameStateType::GoBack)
    {
        m_overlayState.reset();
    }
    else if(request == GameStateType::Settings || request == GameStateType::About)
    {
        m_overlayState = createState(request);
    }
    else if(request == GameStateType::MainMenu)
    {
        m_overlayState.reset();
        m_currentState = createState(GameStateType::MainMenu);
    }
    else if(request == GameStateType::Quit)
    {
        m_window.close();
    }
    else if(request == GameStateType::ResetGame)
    {
        m_overlayState.reset();
        m_currentState = createState(GameStateType::GamePlay);
    }
    else if(request == GameStateType::SaveGameRequest)
    {
        GamePlay* gp = dynamic_cast<GamePlay*>(m_currentState.get());
        if(gp)
        {
            if(m_currentSlotIndex == -1)
            {
                m_currentSlotIndex = getNextAvailableSlotIndex();
            }
            gp->performSaveGame(m_currentSlotIndex);
        }
    }
}

std::unique_ptr<GameState> Game::createState(GameStateType state)
{
    switch(state)
    {
        case GameStateType::MainMenu:
            return std::make_unique<MainMenu>(m_window);
        case GameStateType::NewGame:
            return std::make_unique<NewGame>(m_window);
        case GameStateType::SizeSelect:
            return std::make_unique<SizeSelection>(m_window, m_nextGameMode, transitionPos);
        case GameStateType::GamePlay:
        {
            auto gp = std::make_unique<GamePlay>(m_window, m_nextBoardSize, m_nextGameMode, m_nextDifficulty);
            gp->startGameInitialization();
            return gp;
//            return std::make_unique<GamePlay>(m_window, m_nextBoardSize, m_nextGameMode, m_nextDifficulty);
        }
        case GameStateType::SavedGame:
            return std::make_unique<SavedGame>(m_window);

        case GameStateType::Settings:
            return std::make_unique<Setting>(m_window);
        case GameStateType::About:
            return std::make_unique<About>(m_window);
        case GameStateType::PauseMenu:
            return std::make_unique<PauseMenu>(m_window);
        default:
            return std::make_unique<MainMenu>(m_window);
    }
}
