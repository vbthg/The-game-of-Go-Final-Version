// src/UI/BoardBreathEffect.cpp
#include "BoardBreathEffect.h"
#include <cmath>

namespace UI
{

const float PI = std::acos(-1.f);

BoardBreathEffect::BoardBreathEffect(float speed, float amplitude, float phaseOffset) :
    m_elapsedTime(0.f),
    m_breathSpeed(speed),
    m_breathAmplitude(amplitude),
    m_phaseOffset(phaseOffset)
{
}

void BoardBreathEffect::update(float deltaTime)
{
    m_elapsedTime += deltaTime;
}

float BoardBreathEffect::getScaleForStone(int x, int y, float baseScale) const
{
    // 1. Dùng (x+y) để tạo độ lệch pha
    float timeOffset = (float)(x + y) * m_phaseOffset;

    // 2. Tính toán sóng sin

    float theta = (m_elapsedTime + timeOffset) * m_breathSpeed * PI;
    float sinWave = std::sin(theta);

    // 3. Tính hệ số scale
    // breathMultiplier sẽ dao động từ (1.0 - amplitude) đến (1.0 + amplitude)
    float breathMultiplier = 1.0f + (sinWave * m_breathAmplitude);

    // 4. Trả về kết quả
    return baseScale * breathMultiplier;
}

} // namespace UI
