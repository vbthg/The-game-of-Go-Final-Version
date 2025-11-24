// include/Game.h
#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include "GameState.h" // Chứa enum GameMode, AiDifficulty

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

    // tìm slot trống cho New Game
    int getNextAvailableSlotIndex();

    sf::RenderWindow m_window;
    sf::Clock m_clock;
    sf::RectangleShape m_dimOverlay;

    std::unique_ptr<GameState> m_currentState; // Main state
    std::unique_ptr<GameState> m_overlayState; // Popup state


    GameMode m_nextGameMode;
    AiDifficulty m_nextDifficulty;
    int m_nextBoardSize;


    std::string m_nextLoadFile;      // Lưu tên file để Load
    int m_currentSlotIndex;          // Lưu slot hiện tại (-1 nếu là New Game)
};
