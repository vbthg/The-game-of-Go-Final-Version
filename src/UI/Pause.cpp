#include "Pause.h"
#include "ResourceManager.h"
#include "ScaleEffect.h"
#include <memory>
#include <algorithm>
#include <iostream>

const float BOUNCE_SCALE = 1.1f;
const float BOUNCE_DURATION = 0.1f;

PauseMenu::PauseMenu(sf::RenderWindow& window) :
    m_window(window),
    m_requestedState(GameStateType::NoChange),
    m_backgroundSprite(ResourceManager::getInstance().getTexture("pause_background")),

    m_resumeBtn(ResourceManager::getInstance().getTexture("pause_resume_btn"),       {476.f, 450.f}, true),
    m_resetBtn(ResourceManager::getInstance().getTexture("pause_reset_btn"),         {800.f, 450.f}, true),
    m_saveGameBtn(ResourceManager::getInstance().getTexture("pause_savegame_btn"),  {962.f, 450.f}, true),
    m_settingsBtn(ResourceManager::getInstance().getTexture("pause_setting_btn"),    {638.f, 450.f}, true),
    m_quitToMenuBtn(ResourceManager::getInstance().getTexture("pause_returnmenu_btn"),{1124.f, 450.f}, true)
{
    sf::FloatRect bgBounds = m_backgroundSprite.getLocalBounds();
    m_backgroundSprite.setOrigin(bgBounds.width / 2.f, bgBounds.height / 2.f);
    m_backgroundSprite.setPosition(m_window.getSize().x / 2.f, m_window.getSize().y / 2.f);

    createUI();

    createEffects();

    m_renderTexture.create(m_window.getSize().x, m_window.getSize().y);

    m_allButtons.push_back(&m_resumeBtn);
    m_allButtons.push_back(&m_resetBtn);
    m_allButtons.push_back(&m_saveGameBtn);
    m_allButtons.push_back(&m_settingsBtn);
    m_allButtons.push_back(&m_quitToMenuBtn);
}

void PauseMenu::handleEvent(sf::Event& event)
{
    for(auto* btn : m_allButtons)
    {
        btn->handleEvent(event, m_window);
    }
}

GameStateType PauseMenu::update(float deltaTime)
{
    for(auto* btn : m_allButtons)
    {
        btn->update(m_window);
    }

    for(auto& effect : m_persistentEffects)
    {
        effect->update(deltaTime);
    }
    for(auto& effect : m_oneShotEffects)
    {
        effect->update(deltaTime);
    }

    m_oneShotEffects.erase(std::remove_if(m_oneShotEffects.begin(), m_oneShotEffects.end(),
        [](const auto& effect)
        {
            return effect->isFinished();
        }), m_oneShotEffects.end());

    GameStateType stateToReturn = m_requestedState;
    m_requestedState = GameStateType::NoChange;
    return stateToReturn;
}

void PauseMenu::draw()
{
    m_renderTexture.clear(sf::Color::Transparent);
    m_renderTexture.draw(m_backgroundSprite);
    for(auto* btn : m_allButtons)
    {
        btn->draw(m_renderTexture);
    }
    m_renderTexture.display();

    m_renderSprite.setTexture(m_renderTexture.getTexture());

    m_window.draw(m_renderSprite);
}

sf::Sprite& PauseMenu::getRenderSprite()
{
    return m_renderSprite;
}

void PauseMenu::createUI()
{
    auto resumeBounce = std::make_unique<UI::ScaleEffect>(m_resumeBtn.getSprite(), 0.f, BOUNCE_DURATION, BOUNCE_SCALE);
    m_resumeBtn.setOnClick([this, ptr = resumeBounce.get()]()
    {
        ptr->trigger();
        this->onResumeClick();
    });
    m_persistentEffects.push_back(std::move(resumeBounce));

    auto resetBounce = std::make_unique<UI::ScaleEffect>(m_resetBtn.getSprite(), 0.f, BOUNCE_DURATION, BOUNCE_SCALE);
    m_resetBtn.setOnClick([this, ptr = resetBounce.get()]()
    {
        ptr->trigger();
        this->onResetClick();
    });
    m_persistentEffects.push_back(std::move(resetBounce));

    auto saveBounce = std::make_unique<UI::ScaleEffect>(m_saveGameBtn.getSprite(), 0.f, BOUNCE_DURATION, BOUNCE_SCALE);
    m_saveGameBtn.setOnClick([this, ptr = saveBounce.get()]()
    {
        ptr->trigger();
        this->onSaveGameClick();
    });
    m_persistentEffects.push_back(std::move(saveBounce));

    auto settingsBounce = std::make_unique<UI::ScaleEffect>(m_settingsBtn.getSprite(), 0.f, BOUNCE_DURATION, BOUNCE_SCALE);
    m_settingsBtn.setOnClick([this, ptr = settingsBounce.get()]()
    {
        ptr->trigger();
        this->onSettingsClick();
    });
    m_persistentEffects.push_back(std::move(settingsBounce));

    auto quitBounce = std::make_unique<UI::ScaleEffect>(m_quitToMenuBtn.getSprite(), 0.f, BOUNCE_DURATION, BOUNCE_SCALE);
    m_quitToMenuBtn.setOnClick([this, ptr = quitBounce.get()]()
    {
        ptr->trigger();
        this->onQuitToMenuClick();
    });
    m_persistentEffects.push_back(std::move(quitBounce));
}

void PauseMenu::createEffects()
{
    for(auto* btn : m_allButtons)
    {
        auto growEffect = std::make_unique<UI::ScaleEffect>(btn->getSprite(), 0.3f, 0.f, 1.f);
        growEffect->pop_init();
        m_oneShotEffects.push_back(std::move(growEffect));
    }
}

void PauseMenu::onResumeClick()
{
    m_requestedState = GameStateType::GoBack;
}
void PauseMenu::onResetClick()
{
    m_requestedState = GameStateType::ResetGame;
}
void PauseMenu::onSaveGameClick()
{
    m_requestedState = GameStateType::SaveGameRequest;
}
void PauseMenu::onSettingsClick()
{
    m_requestedState = GameStateType::Settings;
}
void PauseMenu::onQuitToMenuClick()
{

    m_requestedState = GameStateType::MainMenu;
}
