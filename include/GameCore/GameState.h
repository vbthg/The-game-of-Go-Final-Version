// include/GameState.h
#pragma once

#include <SFML/Graphics.hpp>

enum class GameMode {
    PlayerVsPlayer,
    PlayerVsAI
};

enum class AiDifficulty {
    Easy,
    Medium,
    Hard
};


enum class GameStateType
{
    NoChange,
    Quit,
    GoBack,

    MainMenu,
    NewGame,
    SizeSelect,
    GamePlay,
    SavedGame,

    ResetGame,
    SaveGameRequest,

    PauseMenu,
    Settings,
    About
};

class GameState
{
public:
    virtual ~GameState() = default;

    virtual void handleEvent(sf::Event& event) = 0;

    virtual GameStateType update(float deltaTime) = 0;

    virtual void draw() = 0;
};
