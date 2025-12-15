#include "SizeSelection.h"
#include "IEffect.h"
#include "ScaleEffect.h"
#include "SlideEffect.h"
#include "GlobalSetting.h"
#include <iostream>
#include <cmath>

const float SCALE_SIZE_BTN = 1.08f;
const float SCALE_DIFF_BTN = 1.08f;
const float ANIM_SPEED     = 15.0f;

const float BOUNCE_SCALE = 1.1f;
const float BOUNCE_DURATION = 0.1f;

const float SLIDE_DURATION = 0.3f;
const float GROW_DURATION  = 0.3f;

const sf::Color COLOR_SELECTED    = sf::Color(255, 255, 255);
const sf::Color COLOR_UNSELECTED = sf::Color(120, 120, 120);

SizeSelection::SizeSelection(sf::RenderWindow& window, GameMode mode, sf::Vector2f mousePos) :
    m_window(window),
    m_requestedState(GameStateType::NoChange),
    m_background(ResourceManager::getInstance().getTexture("sizeselection_background")),
    m_currentMode(mode),
    m_selectedSize(19),
    m_selectedDiff(AiDifficulty::Easy),

    m_startClickPos(mousePos),

    m_9x9Btn(ResourceManager::getInstance().getTexture("sizeselection_9x9_btn"),     {396.f, 244.f}, true),
    m_13x13Btn(ResourceManager::getInstance().getTexture("sizeselection_13x13_btn"), {800.f, 244.f}, true),
    m_19x19Btn(ResourceManager::getInstance().getTexture("sizeselection_19x19_btn"), {1190.f, 244.f}, true),

    m_easyBtn(ResourceManager::getInstance().getTexture("sizeselection_novice_btn"),   {491.f, 545.f}, true),
    m_medBtn(ResourceManager::getInstance().getTexture("sizeselection_adept_btn"),     {800.f, 545.f}, true),
    m_hardBtn(ResourceManager::getInstance().getTexture("sizeselection_master_btn"),   {1109.f, 545.f}, true),

    m_playBtn(ResourceManager::getInstance().getTexture("sizeselection_apply_btn"),  {800.f, 810.f}, true),
    m_backBtn(ResourceManager::getInstance().getTexture("sizeselection_back_btn"),    {62.f, 62.f}, true),

    m_sizeTextBg(ResourceManager::getInstance().getTexture("sizeselection_text_background")),
    m_diffTextBg(ResourceManager::getInstance().getTexture("sizeselection_text_background"))
{
    sf::FloatRect bounds = m_sizeTextBg.getLocalBounds();
    m_sizeTextBg.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    bounds = m_diffTextBg.getLocalBounds();
    m_diffTextBg.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    createUI();
    createEffects();

    auto& gs = GlobalSetting::getInstance();
    auto& rm = ResourceManager::getInstance();

    std::string filepath = "resources/sounds/drum background.ogg";
    rm.playAmbient(filepath);
    rm.setAmbientVolume(std::min(gs.musicVolume + 30.f, 100.f));
}

void SizeSelection::createUI()
{
    auto& font = ResourceManager::getInstance().getFont("title_font");

    m_labelSize.setFont(font);
    m_labelSize.setString("Board Size");

    sf::FloatRect bSize = m_labelSize.getLocalBounds();
    m_labelSize.setOrigin(bSize.left + bSize.width / 2.f, bSize.top + bSize.height / 2.f);

    m_labelSize.setPosition(151.f, 244.f);

    m_sizeTextBg.setPosition(151.f, 244.f);

    m_9x9Btn.setOnClick([this]()
    {
        on9x9Click();
    });
    m_13x13Btn.setOnClick([this]()
    {
        on13x13Click();
    });
    m_19x19Btn.setOnClick([this]()
    {
        on19x19Click();
    });

    m_groupSizeBtns = { &m_9x9Btn, &m_13x13Btn, &m_19x19Btn };

    if(m_currentMode == GameMode::PlayerVsAI)
    {
        m_labelDiff.setFont(font);
        m_labelDiff.setString("Difficulty");

        bSize = m_labelDiff.getLocalBounds();
        m_labelDiff.setOrigin(bSize.left + bSize.width / 2.f, bSize.top + bSize.height / 2.f);

        m_labelDiff.setPosition(151.f, 545.f);

        m_diffTextBg.setPosition(151.f, 545.f);

        m_easyBtn.setOnClick([this]()
        {
            onEasyClick();
        });
        m_medBtn.setOnClick([this]()
        {
            onMedClick();
        });
        m_hardBtn.setOnClick([this]()
        {
            onHardClick();
        });

        m_groupDiffBtns = { &m_easyBtn, &m_medBtn, &m_hardBtn };
    }

    auto playBounce = std::make_unique<UI::ScaleEffect>(m_playBtn.getSprite(), 0.f, BOUNCE_DURATION, BOUNCE_SCALE);
    m_playBtn.setOnClick([this, e = playBounce.get()]()
    {
        e->trigger();
        onPlayClick();
    });
    m_effects.push_back(std::move(playBounce));

    m_backBtn.setOnClick([this]()
    {
        onBackClick();
    });

    m_allInteractiveBtns = m_groupSizeBtns;
    if(m_currentMode == GameMode::PlayerVsAI)
    {
        m_allInteractiveBtns.insert(m_allInteractiveBtns.end(), m_groupDiffBtns.begin(), m_groupDiffBtns.end());
    }
    m_allInteractiveBtns.push_back(&m_playBtn);
    m_allInteractiveBtns.push_back(&m_backBtn);
}

void SizeSelection::createEffects()
{
    for(auto* btn : m_allInteractiveBtns)
    {
        sf::Sprite& sprite = btn->getSprite();

        auto slide = std::make_unique<UI::SlideEffect>(sprite, m_startClickPos, SLIDE_DURATION);
        m_introEffects.push_back(std::move(slide));

        auto scale = std::make_unique<UI::ScaleEffect>(sprite, GROW_DURATION, 0.f, 1.f);
        scale->pop_init();
        m_introEffects.push_back(std::move(scale));
    }
}

void SizeSelection::handleEvent(sf::Event& event)
{
    for(auto* btn : m_allInteractiveBtns)
    {
        btn->handleEvent(event, m_window);
    }
}

GameStateType SizeSelection::update(float deltaTime)
{
    if(!m_introEffects.empty())
    {
        for(auto& effect : m_introEffects)
        {
            effect->update(deltaTime);
        }

        m_introEffects.erase(std::remove_if(m_introEffects.begin(), m_introEffects.end(),
                                            [](const auto& effect)
        {
            return effect->isFinished();
        }), m_introEffects.end());

        if(!m_introEffects.empty())
        {
            return m_requestedState;
        }
    }

    for(auto* btn : m_allInteractiveBtns)
    {
        btn->update(m_window);
    }

    updateButtonVisuals(deltaTime);

    bool anyHover = false;
    for(auto* btn : m_allInteractiveBtns)
    {
        if(btn->isHoveredAndInteractive())
        {
            anyHover = true;
            break;
        }
    }
    if(anyHover) m_window.setMouseCursor(ResourceManager::getInstance().getCursor("cursor_hand"));
    else m_window.setMouseCursor(ResourceManager::getInstance().getCursor("cursor_arrow"));

    for(auto& effect : m_effects) effect->update(deltaTime);

    GameStateType state = m_requestedState;
    m_requestedState = GameStateType::NoChange;
    return state;
}

void SizeSelection::draw()
{
    m_window.draw(m_background);
    m_window.draw(m_sizeTextBg);
    m_window.draw(m_labelSize);

    for(auto* btn : m_groupSizeBtns) btn->draw(m_window);

    if(m_currentMode == GameMode::PlayerVsAI)
    {
        m_window.draw(m_diffTextBg);
        m_window.draw(m_labelDiff);
        for(auto* btn : m_groupDiffBtns) btn->draw(m_window);
    }

    m_playBtn.draw(m_window);
    m_backBtn.draw(m_window);
}

float SizeSelection::lerp(float start, float end, float speed, float dt)
{
    return start + (end - start) * speed * dt;
}

void SizeSelection::updateButtonVisuals(float deltaTime)
{
    int sizes[] = {9, 13, 19};

    for(size_t i = 0; i < m_groupSizeBtns.size(); ++i)
    {
        UI::Button* btn = m_groupSizeBtns[i];
        sf::Sprite& sprite = btn->getSprite();
        bool isSelected = (m_selectedSize == sizes[i]);

        float targetScale = isSelected ? SCALE_SIZE_BTN : 1.0f;

        float currentScale = sprite.getScale().x;
        float newScale = lerp(currentScale, targetScale, ANIM_SPEED, deltaTime);
        sprite.setScale(newScale, newScale);

        if(isSelected)
        {
            sprite.setColor(COLOR_SELECTED);
        }
        else
        {
            sprite.setColor(COLOR_UNSELECTED);
        }
    }

    if(m_currentMode == GameMode::PlayerVsAI)
    {
        AiDifficulty diffs[] = {AiDifficulty::Easy, AiDifficulty::Medium, AiDifficulty::Hard};

        for(size_t i = 0; i < m_groupDiffBtns.size(); ++i)
        {
            UI::Button* btn = m_groupDiffBtns[i];
            sf::Sprite& sprite = btn->getSprite();
            bool isSelected = (m_selectedDiff == diffs[i]);

            float targetScale = isSelected ? SCALE_DIFF_BTN : 1.0f;

            float currentScale = sprite.getScale().x;
            float newScale = lerp(currentScale, targetScale, ANIM_SPEED, deltaTime);
            sprite.setScale(newScale, newScale);

            if(isSelected) sprite.setColor(COLOR_SELECTED);
            else sprite.setColor(COLOR_UNSELECTED);
        }
    }
}

void SizeSelection::on9x9Click()
{
    m_selectedSize = 9;
}
void SizeSelection::on13x13Click()
{
    m_selectedSize = 13;
}
void SizeSelection::on19x19Click()
{
    m_selectedSize = 19;
}

void SizeSelection::onEasyClick()
{
    m_selectedDiff = AiDifficulty::Easy;
}
void SizeSelection::onMedClick()
{
    m_selectedDiff = AiDifficulty::Medium;
}
void SizeSelection::onHardClick()
{
    m_selectedDiff = AiDifficulty::Hard;
}

void SizeSelection::onPlayClick()
{
    ResourceManager::getInstance().stopAmbient();
    auto& rm = ResourceManager::getInstance();
    rm.stopAmbient();

    float vol = GlobalSetting::getInstance().sfxVolume;
    rm.playSound("drum_drop", 100.f);

    m_requestedState = GameStateType::GamePlay;
}
void SizeSelection::onBackClick()
{
    ResourceManager::getInstance().stopAmbient();
    m_requestedState = GameStateType::NewGame;
}

int SizeSelection::getSelectedSize() const
{
    return m_selectedSize;
}
AiDifficulty SizeSelection::getSelectedDifficulty() const
{
    return m_selectedDiff;
}
