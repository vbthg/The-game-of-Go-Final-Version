#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

#include "GameState.h"
#include "Button.h"
#include "IEffect.h"
#include "ResourceManager.h"

class MainMenu : public GameState
{
public:
    MainMenu(sf::RenderWindow& window);
    virtual ~MainMenu() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;

private:
    void createUI();
    void createEffects();


    void onNewGameClick();
    void onSavedGameClick();
    void onExitClick();


    void onSettingClick();
    void onAboutClick();


    sf::RenderWindow& m_window;
    GameStateType m_requestedState;
    sf::Sprite m_backgroundSprite;


    UI::Button m_NewGameBtn;
    UI::Button m_SavedGameBtn;
    UI::Button m_ExitBtn;


    UI::Button m_SettingBtn;
    UI::Button m_AboutBtn;


    std::vector<UI::Button*> m_allButtons;
    std::vector<std::unique_ptr<IEffect>> m_persistentEffects;
    std::vector<std::unique_ptr<IEffect>> m_oneShotEffects;
};
