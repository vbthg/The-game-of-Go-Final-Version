#pragma once

#include <cmath>

namespace UI
{

class BoardBreathEffect
{
public:

    BoardBreathEffect(float speed, float amplitude, float phaseOffset);

    void update(float deltaTime);

    float getScaleForStone(int x, int y, float baseScale) const;

private:
    float m_elapsedTime;
    float m_breathSpeed;
    float m_breathAmplitude;
    float m_phaseOffset;
};

}
