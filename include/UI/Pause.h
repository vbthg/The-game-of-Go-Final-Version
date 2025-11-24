#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

#include "GameState.h"
#include "Button.h"
#include "IEffect.h"
#include "ResourceManager.h"

class PauseMenu : public GameState
{
public:
    PauseMenu(sf::RenderWindow& window);
    virtual ~PauseMenu() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;

    sf::Sprite& getRenderSprite();

private:
    void createUI();
    void createEffects();


    void onResumeClick();
    void onResetClick();
    void onSaveGameClick();
    void onSettingsClick();
    void onQuitToMenuClick();

    sf::RenderWindow& m_window;
    GameStateType m_requestedState;
    sf::Sprite m_backgroundSprite;


    sf::RenderTexture m_renderTexture;
    sf::Sprite m_renderSprite;


    UI::Button m_resumeBtn;
    UI::Button m_resetBtn;
    UI::Button m_saveGameBtn;
    UI::Button m_settingsBtn;
    UI::Button m_quitToMenuBtn;


    std::vector<UI::Button*> m_allButtons;
    std::vector<std::unique_ptr<IEffect>> m_persistentEffects;
    std::vector<std::unique_ptr<IEffect>> m_oneShotEffects;
};
