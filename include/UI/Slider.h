#pragma once

#include "Button.h"
#include <SFML/Graphics.hpp>
#include <functional>

namespace UI
{

enum class Orientation {
    Horizontal,
    Vertical
};

class Slider
{
public:

    Slider(Orientation orientation, const sf::Texture& trackTexture, float x, float y, float length);

    void handleEvent(sf::Event& event, const sf::RenderWindow& window);
    void update(const sf::RenderWindow& window);
    void draw(sf::RenderTarget& target) const;

    void setOnValueChange(std::function<void(float)> cb);
    float getValue() const;
    void setValue(float value);

private:
    void updateThumbPosition();
    void generateThumbTexture(); // Chỉ sinh ra texture cho Thumb

    Orientation m_orientation;
    bool m_isDragging;
    float m_value;
    float m_length;
    std::function<void(float)> onValueChange;

    // Texture tự sinh cho Thumb
    sf::Texture m_generatedThumbTex;

    Button m_track; // Dùng texture bên ngoài
    Button m_thumb; // Dùng texture tự sinh

    float m_thumbSize;
};

}
