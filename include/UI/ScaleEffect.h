#pragma once
#include <SFML/Graphics.hpp>
#include "IEffect.h"
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

    /**
     * Kích hoạt chuỗi hiệu ứng xuất hiện (0.0 -> x -> 1.0).
     */
    void pop_init();

    /**
     * Kích hoạt chuỗi hiệu ứng nhấn (1.0 -> x -> 1.0).
     */
    void bounce_init();

    /**
     * Cập nhật trạng thái hiệu ứng (PHẢI GỌI MỖI KHUNG HÌNH).
     */
    void update(float deltaTime) override;

    /**
     * Kích hoạt hiệu ứng nhấn (bounce).
     */
    void trigger() override;

    /**
     * Kiểm tra xem hiệu ứng đã hoàn thành hay chưa.
     */
    bool isFinished() const override;

    /**
     * Cho phép thay đổi độ nảy ('x') một cách linh hoạt.
     */
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
