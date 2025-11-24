#include "NewGame.h"
#include "ResourceManager.h"
#include "ScaleEffect.h"
#include <memory>
#include <algorithm>
#include <functional>
#include <iostream>

NewGame::NewGame(sf::RenderWindow& window):
    m_window(window),
    m_requestedState(GameStateType::NoChange),
    m_backgroundSprite(ResourceManager::getInstance().getTexture("newgame_background")),

    m_pvpBtn(ResourceManager::getInstance().getTexture("newgame_pvp_btn"), {616.f, 450.f}, true),
    m_pvbotBtn(ResourceManager::getInstance().getTexture("newgame_pvbot_btn"), {984.f, 450.f}, true),
    m_backBtn(ResourceManager::getInstance().getTexture("newgame_back_btn"), {62.f, 57.f}, true),
    m_selectedMode(GameMode::PlayerVsPlayer)
{
    createUI();
    createEffects();

    m_allButtons.push_back(&m_pvpBtn);
    m_allButtons.push_back(&m_pvbotBtn);
    m_allButtons.push_back(&m_backBtn);
}

void NewGame::handleEvent(sf::Event& event)
{
    for(auto *p : m_allButtons) p->handleEvent(event, m_window);
}

GameStateType NewGame::update(float deltaTime)
{
    for(auto &effect : m_persistentEffects) effect->update(deltaTime);
    for(auto &effect : m_oneShotEffects) effect->update(deltaTime);
    m_oneShotEffects.erase
    (
        std::remove_if(m_oneShotEffects.begin(), m_oneShotEffects.end(), [](const auto& e)
                       {
                            return e->isFinished();
                       }
                       ), m_oneShotEffects.end()
    );

    for(auto *p : m_allButtons) p -> update(m_window);

    GameStateType stateToReturn = m_requestedState;
    m_requestedState = GameStateType::NoChange;
    return stateToReturn;
}

void NewGame::draw()
{
    m_window.draw(m_backgroundSprite);
    for (auto* button : m_allButtons) button -> draw(m_window);
}

void NewGame::createUI()
{
    auto pvpBounce = std::make_unique<UI::ScaleEffect>(m_pvpBtn.getSprite(), 0.f, 0.15f, 1.15f);
    m_pvpBtn.setOnClick([this, p = pvpBounce.get()]()
                        {
                            p -> trigger();
                            this -> onpvpClick();
                        });

    m_persistentEffects.push_back(std::move(pvpBounce));



    auto botBounce = std::make_unique<UI::ScaleEffect>(m_pvbotBtn.getSprite(), 0.f, 0.15f, 1.15f);
    m_pvbotBtn.setOnClick([this, p = botBounce.get()]()
                          {
                              p -> trigger();
                              this -> onpvbotClick();
                          });

    m_persistentEffects.push_back(std::move(botBounce));



    auto backBounce = std::make_unique<UI::ScaleEffect>(m_backBtn.getSprite(), 0.f, 0.15f, 1.15f);
    m_backBtn.setOnClick([this, p = backBounce.get()]()
                         {
                             p -> trigger();
                             this -> onbackClick();
                         });

    m_persistentEffects.push_back(std::move(backBounce));
}

void NewGame::createEffects()
{
    auto pvpGrow = std::make_unique<UI::ScaleEffect>(m_pvpBtn.getSprite(), 0.3f, 0.f, 1.f); pvpGrow -> pop_init();
    m_oneShotEffects.push_back(std::move(pvpGrow));

    auto botGrow = std::make_unique<UI::ScaleEffect>(m_pvbotBtn.getSprite(), 0.3f, 0.f, 1.f); botGrow -> pop_init();
    m_oneShotEffects.push_back(std::move(botGrow));

    auto backGrow = std::make_unique<UI::ScaleEffect>(m_backBtn.getSprite(), 0.3f, 0.f, 1.f); backGrow -> pop_init();
    m_oneShotEffects.push_back(std::move(backGrow));
}


void NewGame::onpvpClick()
{
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);
    m_lastClickPos = m_window.mapPixelToCoords(pixelPos);

    m_selectedMode = GameMode::PlayerVsPlayer;
    m_requestedState = GameStateType::SizeSelect;
}

void NewGame::onpvbotClick()
{
    sf::Vector2i pixelPos = sf::Mouse::getPosition(m_window);
    m_lastClickPos = m_window.mapPixelToCoords(pixelPos);

    m_selectedMode = GameMode::PlayerVsAI;
    m_requestedState = GameStateType::SizeSelect;
}

void NewGame::onbackClick()
{
    m_requestedState = GameStateType::MainMenu;
}


GameMode NewGame::getSelectedGameMode() const
{
    return m_selectedMode;
}

sf::Vector2f NewGame::getLastClickPos() const
{
    return m_lastClickPos;
}
