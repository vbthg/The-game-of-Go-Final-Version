#include "BoardBreathEffect.h"
#include <cmath>

namespace UI
{

const float PI = std::acos(-1.f);

BoardBreathEffect::BoardBreathEffect(float speed, float amplitude, float phaseOffset) :
    m_elapsedTime(0.f),
    m_breathSpeed(speed),
    m_breathAmplitude(amplitude),
    m_phaseOffset(phaseOffset) { }

void BoardBreathEffect::update(float deltaTime)
{
    m_elapsedTime += deltaTime;
}

float BoardBreathEffect::getScaleForStone(int x, int y, float baseScale) const
{
    float timeOffset = (float)(x + y) * m_phaseOffset;

    float theta = (m_elapsedTime + timeOffset) * m_breathSpeed * PI;
    float sinWave = std::sin(theta);

    float breathMultiplier = 1.0f + (sinWave * m_breathAmplitude);

    return baseScale * breathMultiplier;
}

}
