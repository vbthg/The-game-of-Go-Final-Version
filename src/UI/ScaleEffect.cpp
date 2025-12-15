#include "ScaleEffect.h"
#include <SFML/Graphics.hpp>

#include <functional>
#include <string>
#include <cmath>
#include <algorithm>
#include <iostream>

namespace UI
{

ScaleEffect::ScaleEffect(sf::Sprite& target, float growDuration, float popDuration, float popAmount):
    m_target(target),
    m_growDuration(growDuration),
    m_popDuration(popDuration),
    m_popAmount(popAmount),
    m_elapsedTime(0.f),
    m_currentDuration(0.f),
    m_startScale(0.f),
    m_state(State::IDLE),
    m_targetScale(1.f)
{
    m_target.setScale(1.f, 1.f);
}

void ScaleEffect::pop_init()
{
    m_state = State::GROWING;
    m_elapsedTime = 0.f;
    m_currentDuration = m_growDuration;
    m_startScale = 0.f;
    m_targetScale = 1.f;

    m_target.setScale(0.f, 0.f);
}

void ScaleEffect::bounce_init()
{
    if(m_state != State::IDLE) return;

    m_state = State::POPPING;
    m_elapsedTime = 0.f;
    m_currentDuration = m_popDuration;
    m_startScale = 1.f;
    m_targetScale = m_popAmount;
}

void ScaleEffect::update(float deltaTime)
{
    if(isFinished()) return;

    m_elapsedTime += deltaTime;

    if(m_elapsedTime > m_currentDuration)
    {
        m_elapsedTime = m_currentDuration;
    }

    float Ratio = 0.f;
    if(m_currentDuration > 0.f)
    {
        Ratio = m_elapsedTime / m_currentDuration;
    }
    else
    {
        Ratio = 1.f;
    }

    float easedRatio = sin(Ratio * (M_PI / 2.f));
    float m_currentScale = m_startScale + (m_targetScale - m_startScale) * easedRatio;

    m_target.setScale(m_currentScale, m_currentScale);

    if(Ratio >= 1.f)
    {
        switch(m_state)
        {
        case State::GROWING:
            m_state = State::POPPING;
            m_elapsedTime = 0.f;
            m_currentDuration = m_popDuration;
            m_startScale = 1.f;
            m_targetScale = m_popAmount;
            break;

        case State::POPPING:
            m_state = State::SETTLING;
            m_elapsedTime = 0.f;
            m_currentDuration = m_popDuration;
            m_startScale = m_popAmount;
            m_targetScale = 1.f;
            break;

        case State::SETTLING:
            m_state = State::IDLE;
            m_elapsedTime = 0.f;
            m_currentDuration = 0.f;
            m_startScale = 1.f;
            m_targetScale = 1.f;

            m_target.setScale(1.f, 1.f);
            break;

        case State::IDLE:
            break;
        }
    }
}

void ScaleEffect::trigger()
{
    bounce_init();
}

bool ScaleEffect::isFinished() const
{
    return m_state == State::IDLE;
}

void ScaleEffect::setPopAmount(float amount)
{
    m_popAmount = amount;
}

}
