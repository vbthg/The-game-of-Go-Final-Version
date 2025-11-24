#include "MainMenu.h"
#include "ResourceManager.h"
#include "ScaleEffect.h"
#include <memory>
#include <algorithm>
#include <functional>


MainMenu::MainMenu(sf::RenderWindow& window) :
    m_window(window),
    m_requestedState(GameStateType::NoChange),
    m_backgroundSprite(ResourceManager::getInstance().getTexture("mainmenu_background")),

    m_NewGameBtn(ResourceManager::getInstance().getTexture("mainmenu_newgame_btn"), {476.f, 660.f}, true),
    m_SavedGameBtn(ResourceManager::getInstance().getTexture("mainmenu_savedgame_btn"), {800.f, 660.f}, true),
    m_SettingBtn(ResourceManager::getInstance().getTexture("mainmenu_setting_btn"), {1124.f, 660.f}, true),

    m_ExitBtn(ResourceManager::getInstance().getTexture("mainmenu_quit_btn"), {1543.f, 57.f}, true),
    m_AboutBtn(ResourceManager::getInstance().getTexture("mainmenu_about_btn"), {1423.f, 60.f}, true)
{

    createUI();
    createEffects();

    m_allButtons.push_back(&m_NewGameBtn);
    m_allButtons.push_back(&m_SavedGameBtn);
    m_allButtons.push_back(&m_ExitBtn);
    m_allButtons.push_back(&m_SettingBtn);
    m_allButtons.push_back(&m_AboutBtn);
}


void MainMenu::handleEvent(sf::Event& event)
{
    for (auto* p : m_allButtons)
    {
        p -> handleEvent(event, m_window);
    }
}


GameStateType MainMenu::update(float deltaTime)
{
    for (auto &effect : m_persistentEffects)
    {
        effect -> update(deltaTime);
    }

    for (auto &effect : m_oneShotEffects)
    {
        effect -> update(deltaTime);
    }
    m_oneShotEffects.erase
    (
        std::remove_if(
            m_oneShotEffects.begin(),
            m_oneShotEffects.end(),
            [](const auto& effect) { return effect -> isFinished(); }
        ),
        m_oneShotEffects.end()
    );

    for (auto* p : m_allButtons)
    {
        p -> update(m_window);
    }

    GameStateType stateToReturn = m_requestedState;
    m_requestedState = GameStateType::NoChange;
    return stateToReturn;
}


void MainMenu::draw()
{
    m_window.draw(m_backgroundSprite);

    for (auto* p : m_allButtons)
    {
        p->draw(m_window);
    }
}


void MainMenu::createUI()
{
    auto NewGameBounce = std::make_unique<UI::ScaleEffect>(m_NewGameBtn.getSprite(), 0.f, 0.15f, 1.15f);
    IEffect* NewGame_BouncePtr = NewGameBounce.get();
    m_NewGameBtn.setOnClick([this, NewGame_BouncePtr]()
    {
        NewGame_BouncePtr->trigger();
        onNewGameClick();
    });
    m_persistentEffects.push_back(std::move(NewGameBounce));


    auto SavedGameBounce = std::make_unique<UI::ScaleEffect>(m_SavedGameBtn.getSprite(), 0.f, 0.15f, 1.15f);
    IEffect* SavedGame_BouncePtr = SavedGameBounce.get();
    m_SavedGameBtn.setOnClick([this, SavedGame_BouncePtr]()
    {
        SavedGame_BouncePtr->trigger();
        onSavedGameClick();
    });
    m_persistentEffects.push_back(std::move(SavedGameBounce));


    auto ExitBounce = std::make_unique<UI::ScaleEffect>(m_ExitBtn.getSprite(), 0.f, 0.15f, 1.15f);
    IEffect* Exit_BouncePtr = ExitBounce.get();
    m_ExitBtn.setOnClick([this, Exit_BouncePtr]()
    {
        Exit_BouncePtr->trigger();
        onExitClick();
    });
    m_persistentEffects.push_back(std::move(ExitBounce));


    auto SettingBounce = std::make_unique<UI::ScaleEffect>(m_SettingBtn.getSprite(), 0.f, 0.15f, 1.15f);
    IEffect* Setting_BouncePtr = SettingBounce.get();
    m_SettingBtn.setOnClick([this, Setting_BouncePtr]()
    {
        Setting_BouncePtr->trigger();
        this->onSettingClick();
    });
    m_persistentEffects.push_back(std::move(SettingBounce));


    auto AboutBounce = std::make_unique<UI::ScaleEffect>(m_AboutBtn.getSprite(), 0.f, 0.15f, 1.15f);
    IEffect* About_BouncePtr = AboutBounce.get();
    m_AboutBtn.setOnClick([this, About_BouncePtr]()
    {
        About_BouncePtr->trigger();
        onAboutClick();
    });
    m_persistentEffects.push_back(std::move(AboutBounce));
}


void MainMenu::createEffects()
{
    auto NewGameGrow = std::make_unique<UI::ScaleEffect>(m_NewGameBtn.getSprite(), 0.3f, 0.f, 1.f);
    NewGameGrow -> pop_init();
    m_oneShotEffects.push_back(std::move(NewGameGrow));


    auto SavedGameGrow = std::make_unique<UI::ScaleEffect>(m_SavedGameBtn.getSprite(), 0.3f, 0.f, 1.f);
    SavedGameGrow -> pop_init();
    m_oneShotEffects.push_back(std::move(SavedGameGrow));


    auto ExitGrow = std::make_unique<UI::ScaleEffect>(m_ExitBtn.getSprite(), 0.3f, 0.f, 1.f);
    ExitGrow -> pop_init();
    m_oneShotEffects.push_back(std::move(ExitGrow));


    auto SettingGrow = std::make_unique<UI::ScaleEffect>(m_SettingBtn.getSprite(), 0.3f, 0.f, 1.f);
    SettingGrow -> pop_init();
    m_oneShotEffects.push_back(std::move(SettingGrow));


    auto AboutGrow = std::make_unique<UI::ScaleEffect>(m_AboutBtn.getSprite(), 0.3f, 0.f, 1.f);
    AboutGrow -> pop_init();
    m_oneShotEffects.push_back(std::move(AboutGrow));
}



void MainMenu::onNewGameClick()
{
    m_requestedState = GameStateType::NewGame;
}

void MainMenu::onSavedGameClick()
{
    m_requestedState = GameStateType::SavedGame;
}

void MainMenu::onExitClick()
{
    m_requestedState = GameStateType::Quit;
}

void MainMenu::onSettingClick()
{
    m_requestedState = GameStateType::Settings;
}

void MainMenu::onAboutClick()
{
    m_requestedState = GameStateType::About;
}
