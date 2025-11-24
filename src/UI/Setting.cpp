#include "Setting.h"
#include "ResourceManager.h"
#include "GlobalSetting.h"
#include "ScaleEffect.h"
#include <iostream>
#include <vector>
#include <string>

Setting::Setting(sf::RenderWindow& window, GameStateType backState) :
    m_window(window),
    m_requestedState(GameStateType::NoChange),
    m_backState(backState),
    m_backgroundSprite(ResourceManager::getInstance().getTexture("setting_background")),
    m_applyBtn(ResourceManager::getInstance().getTexture("setting_apply_btn"), {942.f, 596.f}, true),
    m_backBtn(ResourceManager::getInstance().getTexture("setting_back_btn"), {1057.f, 596.f}, true)
{
    createUI();
    loadSetting();

    m_renderTexture.create(m_window.getSize().x, m_window.getSize().y);
}

void Setting::handleEvent(sf::Event& event)
{
    m_applyBtn.handleEvent(event, m_window);
    m_backBtn.handleEvent(event, m_window);

    if(m_musicSlider) m_musicSlider -> handleEvent(event, m_window);
    if(m_sfxSlider) m_sfxSlider -> handleEvent(event, m_window);

    if(m_stoneTheme) m_stoneTheme -> handleEvent(event, m_window);
    if(m_musicTheme) m_musicTheme -> handleEvent(event, m_window);
    if(m_boardTheme) m_boardTheme -> handleEvent(event, m_window);

    if(m_timeLimit) m_timeLimit -> handleEvent(event, m_window);
    if(m_komiPoints) m_komiPoints -> handleEvent(event, m_window);
}

GameStateType Setting::update(float deltaTime)
{
    m_applyBtn.update(m_window);
    m_backBtn.update(m_window);

    if(m_musicSlider) m_musicSlider -> update(m_window);
    if(m_sfxSlider) m_sfxSlider -> update(m_window);

    if(m_stoneTheme) m_stoneTheme -> update(m_window);
    if(m_musicTheme) m_musicTheme -> update(m_window);
    if(m_boardTheme) m_boardTheme -> update(m_window);

    if(m_timeLimit) m_timeLimit -> update(m_window);
    if(m_komiPoints) m_komiPoints -> update(m_window);

    for(auto &effect : m_persistentEffects) effect->update(deltaTime);

    GameStateType stateToReturn = m_requestedState;
    m_requestedState = GameStateType::NoChange;
    return stateToReturn;
}

void Setting::draw()
{
    m_renderTexture.clear(sf::Color::Transparent);

    m_renderTexture.draw(m_backgroundSprite);

    if(m_stoneTheme) m_stoneTheme->draw(m_renderTexture);
    if(m_musicTheme) m_musicTheme->draw(m_renderTexture);
    if(m_boardTheme) m_boardTheme->draw(m_renderTexture);

    if(m_musicSlider) m_musicSlider->draw(m_renderTexture);
    if(m_sfxSlider) m_sfxSlider->draw(m_renderTexture);

    if(m_timeLimit) m_timeLimit->draw(m_renderTexture);
    if(m_komiPoints) m_komiPoints->draw(m_renderTexture);

    m_applyBtn.draw(m_renderTexture);
    m_backBtn.draw(m_renderTexture);

    m_renderTexture.display();
    m_renderSprite.setTexture(m_renderTexture.getTexture());
    m_window.draw(m_renderSprite);
}

void Setting::createUI()
{
    m_stoneTheme = std::make_unique<UI::RadioGroup>
    (
        3,
        sf::Vector2f{486.f, 247.f},
        78.5f
    );
    m_stoneTheme -> setOnChange([this](int i){ onStoneThemeChange(i); });

    m_musicTheme = std::make_unique<UI::RadioGroup>(3, sf::Vector2f{486.f, 451.f}, 78.5f);
    m_musicTheme -> setOnChange([this](int i){ onMusicThemeChange(i); });

    m_boardTheme = std::make_unique<UI::RadioGroup>(3, sf::Vector2f{486.f, 654.f}, 78.5f);
    m_boardTheme -> setOnChange([this](int i){ onBoardThemeChange(i); });


    auto& barTex = ResourceManager::getInstance().getTexture("setting_slidertrack_btn");

    m_musicSlider = std::make_unique<UI::Slider>
    (
        UI::Orientation::Horizontal,
        barTex,
        1028.f, 365.f, 250.f
    );
    m_musicSlider -> setOnValueChange([this](float v){ onMusicVolumeChange(v); });

    m_sfxSlider = std::make_unique<UI::Slider>
    (
        UI::Orientation::Horizontal,
        barTex,
        1028.f, 435.f, 250.f
    );
    m_sfxSlider -> setOnValueChange([this](float v){ onSfxVolumeChange(v); });


    std::vector<std::string> timeOptions =
    {
        "5 min", "10 min", "20 min", "30 min",
        "45 min", "60 min", "no limit"
    };

    m_timeLimit = std::make_unique<UI::Stepper>
    (
        sf::Vector2f{980.f, 157.f},
        153.f,
        timeOptions,
        1
    );
    m_timeLimit->setOnChange([this](int idx){ onTimeLimitChange(idx); });


    std::vector<std::string> komiOptions = {
        "0.5", "6.5", "7.5"
    };

    m_komiPoints = std::make_unique<UI::Stepper>
    (
        sf::Vector2f{1022.f, 227.f},
        111.f,
        komiOptions,
        1
    );
    m_komiPoints->setOnChange([this](int idx){ onKomiChange(idx); });



    auto applyBounce = std::make_unique<UI::ScaleEffect>(m_applyBtn.getSprite(), 0.f, 0.15f, 1.15f);
    m_applyBtn.setOnClick([this, ptr = applyBounce.get()]()
    {
        ptr -> trigger();
        onApplyClick();
    });
    m_persistentEffects.push_back(std::move(applyBounce));

    auto backBounce = std::make_unique<UI::ScaleEffect>(m_backBtn.getSprite(), 0.f, 0.15f, 1.15f);
    m_backBtn.setOnClick([this, ptr = backBounce.get()]()
    {
        ptr -> trigger();
        onBackClick();
    });
    m_persistentEffects.push_back(std::move(backBounce));
}

void Setting::loadSetting()
{
    auto& g = GlobalSetting::getInstance();


    if(m_musicSlider) m_musicSlider->setValue(g.musicVolume / 100.f);
    if(m_sfxSlider)   m_sfxSlider->setValue(g.sfxVolume / 100.f);

    if(m_stoneTheme) m_stoneTheme->setSelectedIndex(g.stoneThemeIndex);
    if(m_musicTheme) m_musicTheme->setSelectedIndex(g.musicThemeIndex);
    if(m_boardTheme) m_boardTheme->setSelectedIndex(g.boardThemeIndex);

    if(m_timeLimit)  m_timeLimit->setSelectedIndex(g.timeLimitIndex);
    if(m_komiPoints) m_komiPoints->setSelectedIndex(g.komiIndex);
}

void Setting::saveSetting()
{
    GlobalSetting::getInstance().saveToFile();
}

void Setting::onApplyClick()
{
    saveSetting();
}

void Setting::onBackClick()
{
    m_requestedState = m_backState;
}

void Setting::onMusicVolumeChange(float value)
{
    float vol = value * 100.f;

    GlobalSetting::getInstance().musicVolume = vol;

    ResourceManager::getInstance().setMusicVolume(vol);
}

void Setting::onSfxVolumeChange(float value)
{
    GlobalSetting::getInstance().sfxVolume = value * 100.f;
}

void Setting::onStoneThemeChange(int index)
{
    GlobalSetting::getInstance().stoneThemeIndex = index;
}

void Setting::onBoardThemeChange(int index)
{
    GlobalSetting::getInstance().boardThemeIndex = index;
}

void Setting::onMusicThemeChange(int index)
{
    GlobalSetting::getInstance().musicThemeIndex = index;

    ResourceManager::getInstance().playMusic(index);

    ResourceManager::getInstance().setMusicVolume(GlobalSetting::getInstance().musicVolume);
}

void Setting::onTimeLimitChange(int index)
{
    GlobalSetting::getInstance().timeLimitIndex = index;
}

void Setting::onKomiChange(int index)
{
    GlobalSetting::getInstance().komiIndex = index;
}
