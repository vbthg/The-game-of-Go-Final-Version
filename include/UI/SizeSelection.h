#pragma once

#include "GameState.h"
#include "Button.h"
#include "ResourceManager.h"
#include "IEffect.h"
#include "SlideEffect.h"
#include "ScaleEffect.h"
#include <vector>
#include <memory>

class SizeSelection : public GameState
{
public:
    SizeSelection(sf::RenderWindow& window, GameMode mode, sf::Vector2f mousePos);
    virtual ~SizeSelection() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;

    int getSelectedSize() const;
    AiDifficulty getSelectedDifficulty() const;

private:
    void createUI();
    void createEffects();


    void updateButtonVisuals(float deltaTime);
    float lerp(float start, float end, float speed, float dt);


    void on9x9Click();
    void on13x13Click();
    void on19x19Click();
    void onEasyClick();
    void onMedClick();
    void onHardClick();
    void onPlayClick();
    void onBackClick();

    sf::RenderWindow& m_window;
    GameStateType m_requestedState;
    sf::Sprite m_background;

    GameMode m_currentMode;
    int m_selectedSize;
    AiDifficulty m_selectedDiff;


    sf::Text m_labelSize;
    UI::Button m_9x9Btn;
    UI::Button m_13x13Btn;
    UI::Button m_19x19Btn;
    sf::Sprite m_sizeTextBg;


    sf::Text m_labelDiff;
    UI::Button m_easyBtn;
    UI::Button m_medBtn;
    UI::Button m_hardBtn;
    sf::Sprite m_diffTextBg;


    UI::Button m_playBtn;
    UI::Button m_backBtn;


    std::vector<UI::Button*> m_groupSizeBtns;
    std::vector<UI::Button*> m_groupDiffBtns;
    std::vector<UI::Button*> m_allInteractiveBtns;

    std::vector<std::unique_ptr<IEffect>> m_effects;

    sf::Vector2f m_startClickPos;

    std::vector<std::unique_ptr<IEffect>> m_introEffects; // Hiệu ứng lúc mới vào

};
