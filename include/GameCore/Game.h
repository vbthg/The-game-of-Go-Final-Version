#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include "GameState.h"

class Game
{
public:
    Game(unsigned int width, unsigned int height, const std::string& title);
    virtual ~Game() = default;
    void run();

private:
    void handleEvents();
    void update(sf::Time deltaTime);
    void draw();

    sf::Vector2f transitionPos;

    std::unique_ptr<GameState> createState(GameStateType state);
    void handleMainRequest(GameStateType request);
    void handleOverlayRequest(GameStateType request);

    GameStateType m_pendingRequest;
    bool m_isFromOverlay;

    int getNextAvailableSlotIndex();

    sf::RenderWindow m_window;
    sf::Clock m_clock;
    sf::RectangleShape m_dimOverlay;

    std::unique_ptr<GameState> m_currentState;
    std::unique_ptr<GameState> m_overlayState;

    GameMode m_nextGameMode;
    AiDifficulty m_nextDifficulty;
    int m_nextBoardSize;

    std::string m_nextLoadFile;
    int m_currentSlotIndex;

    float m_delayTimer = 0.0f;
};
