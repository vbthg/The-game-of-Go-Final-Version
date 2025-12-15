// include/NewGame.h
#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

#include "GameState.h"
#include "Button.h"
#include "IEffect.h"
#include "ResourceManager.h"

class NewGame : public GameState
{
public:
    NewGame(sf::RenderWindow& window);
    virtual ~NewGame() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;

    GameMode getSelectedGameMode() const;

    sf::Vector2f getLastClickPos() const;

private:
    void createUI();
    void createEffects();

    void onpvpClick();
    void onpvbotClick();
    void onbackClick();

    sf::RenderWindow& m_window;
    GameStateType m_requestedState;
    sf::Sprite m_backgroundSprite;

    UI::Button m_pvpBtn;
    UI::Button m_pvbotBtn;
    UI::Button m_backBtn;

    GameMode m_selectedMode;

    sf::Vector2f m_lastClickPos;

    std::vector<UI::Button*> m_allButtons;
    std::vector<std::unique_ptr<IEffect>> m_persistentEffects;
    std::vector<std::unique_ptr<IEffect>> m_oneShotEffects;
};
