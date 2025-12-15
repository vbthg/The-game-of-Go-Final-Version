#pragma once

#include <SFML/Graphics.hpp>
#include "IEffect.h"

namespace UI
{
class SlideEffect: public IEffect
{
public:
    SlideEffect(sf::Sprite& target,
                sf::Vector2f startPos,
                const float& slideDuration);

    void update(float deltaTime) override;

    bool isFinished() const override;

private:
    sf::Sprite& m_target;

    sf::Vector2f m_startPos;
    sf::Vector2f m_endPos;

    float m_slideDuration;
    float m_elapsedTime;
    bool m_isFinished;
};


}
