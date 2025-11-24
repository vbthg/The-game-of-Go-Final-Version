#include "SlideEffect.h"
#include <SFML/Graphics.hpp>
#include <cmath>

namespace UI
{

SlideEffect::SlideEffect(sf::Sprite& target,
            sf::Vector2f startPos,
            const float& slideDuration):

                m_target(target),
                m_startPos(startPos),
                m_endPos(target.getPosition()),
                m_slideDuration(slideDuration),
                m_elapsedTime(0.f),
                m_isFinished(false)
                {
                    m_target.setPosition(m_startPos);
                }


void SlideEffect::update(float deltaTime)
{
    if(SlideEffect::isFinished()) return;

    m_elapsedTime += deltaTime;
    if(m_elapsedTime > m_slideDuration)
    {
        m_elapsedTime = m_slideDuration;
        m_isFinished = true;
    }

    float Ratio = m_elapsedTime / m_slideDuration;
    float easedRatio = sin(Ratio * (acos(-1) / 2.f));

    float x = m_startPos.x + easedRatio * (m_endPos.x - m_startPos.x);
    float y = m_startPos.y + easedRatio * (m_endPos.y - m_startPos.y);

    m_target.setPosition(sf::Vector2f(x, y));

    if (m_isFinished)
    {
        m_target.setPosition(m_endPos);
    }
}

bool SlideEffect::isFinished() const
{
    return m_isFinished;
}

}
