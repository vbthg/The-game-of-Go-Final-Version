#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

#include "GameState.h"
#include "Button.h"
#include "IEffect.h"
#include "ResourceManager.h"
#include "Slider.h"
#include "Stepper.h"
#include "RadioButton.h"

class Setting : public GameState
{
public:

    Setting(sf::RenderWindow& window, GameStateType backState = GameStateType::GoBack);
    virtual ~Setting() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;



private:
    void createUI();
    void loadSetting();
    void saveSetting();

    void onApplyClick();
    void onBackClick();

    void onMusicVolumeChange(float value);
    void onSfxVolumeChange(float value);

    void onStoneThemeChange(int index);
    void onMusicThemeChange(int index);
    void onBoardThemeChange(int index);

    void onTimeLimitChange(int index);
    void onKomiChange(int index);

    sf::RenderWindow& m_window;
    GameStateType m_requestedState;
    GameStateType m_backState;

    sf::Sprite m_backgroundSprite;

    sf::RenderTexture m_renderTexture;
    sf::Sprite m_renderSprite;

    UI::Button m_applyBtn;
    UI::Button m_backBtn;

    std::unique_ptr<UI::Slider> m_musicSlider;
    std::unique_ptr<UI::Slider> m_sfxSlider;

    std::unique_ptr<UI::RadioGroup> m_stoneTheme;
    std::unique_ptr<UI::RadioGroup> m_musicTheme;
    std::unique_ptr<UI::RadioGroup> m_boardTheme;

    std::unique_ptr<UI::Stepper> m_timeLimit;
    std::unique_ptr<UI::Stepper> m_komiPoints;

    // Hiệu ứng nút bấm
    std::vector<std::unique_ptr<IEffect>> m_persistentEffects;
};
