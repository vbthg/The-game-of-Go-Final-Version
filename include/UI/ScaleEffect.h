#pragma once
#include <SFML/Graphics.hpp> // <-- ĐÃ THÊM
#include "IEffect.h"
// #include "Button.h" // <-- ĐÃ XÓA
#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846f
#endif


namespace UI
{

class ScaleEffect: public IEffect
{
public:
    ScaleEffect(sf::Sprite& target, float growDuration, float popDuration, float popAmount);

    void pop_init();

    void bounce_init();

    void update(float deltaTime) override;

    void trigger() override;

    bool isFinished() const override;

    void setPopAmount(float amount);

private:

    enum class State
    {
        IDLE,
        GROWING,
        POPPING,
        SETTLING
    };


    State m_state;

    sf::Sprite& m_target;

    float m_growDuration;
    float m_popDuration;
    float m_popAmount;

    float m_elapsedTime;
    float m_currentDuration;
    float m_startScale;
    float m_targetScale;

    const float m_finalScale = 1.0f;
};

}
