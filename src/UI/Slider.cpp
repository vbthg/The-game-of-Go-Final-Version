#include "Slider.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace UI
{

Slider::Slider(Orientation orientation, const sf::Texture& trackTexture, float x, float y, float length) :
    m_orientation(orientation),
    m_isDragging(false),
    m_value(0.f),
    m_length(length),
    onValueChange(nullptr),
    m_track(trackTexture, {x, y}, false),
    m_thumb(m_generatedThumbTex, {x, y}, true)
{
    generateThumbTexture();
    m_thumb.getSprite().setTexture(m_generatedThumbTex, true);

    sf::FloatRect thumbBounds = m_thumb.getLocalBounds();
    m_thumb.getSprite().setOrigin(thumbBounds.width / 2.f, thumbBounds.height / 2.f);

    if (m_orientation == Orientation::Vertical)
        m_thumbSize = thumbBounds.height;
    else
        m_thumbSize = thumbBounds.width;

    sf::Sprite& trackSprite = m_track.getSprite();
    if (m_orientation == Orientation::Vertical)
    {
        float originalHeight = trackSprite.getLocalBounds().height;
        if (originalHeight > 0)
            trackSprite.setScale(trackSprite.getScale().x, m_length / originalHeight);
    }
    else
    {
        float originalWidth = trackSprite.getLocalBounds().width;
        if (originalWidth > 0)
            trackSprite.setScale(m_length / originalWidth, trackSprite.getScale().y);
    }

    m_track.setPosition({std::round(x), std::round(y)});

    updateThumbPosition();
}

void Slider::generateThumbTexture()
{
    float radius = 10.f;
    float outline = 2.f;
    unsigned int dim = (unsigned int)((radius + outline) * 2);

    sf::RenderTexture rt;
    rt.create(dim, dim);
    rt.clear(sf::Color::Transparent);

    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(dim / 2.f, dim / 2.f);

    circle.setFillColor(sf::Color::White);
    circle.setOutlineThickness(outline);
    circle.setOutlineColor(sf::Color::Black);

    rt.draw(circle);
    rt.display();

    m_generatedThumbTex = rt.getTexture();
    m_generatedThumbTex.setSmooth(true);
}

void Slider::handleEvent(sf::Event& event, const sf::RenderWindow& window)
{
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        if (m_thumb.getSprite().getGlobalBounds().contains(mousePos))
        {
            m_isDragging = true;
        }
        else if (m_track.getSprite().getGlobalBounds().contains(mousePos))
        {
            float newValue = 0.f;
            float usableLength = m_length - m_thumbSize;
            if (usableLength <= 0) return;

            if (m_orientation == Orientation::Vertical)
            {
                float trackTop = m_track.getPosition().y - m_length / 2.f;
                float relativeY = mousePos.y - trackTop - (m_thumbSize / 2.f);
                newValue = relativeY / usableLength;
            }
            else
            {
                float trackLeft = m_track.getPosition().x - m_length / 2.f;
                float relativeX = mousePos.x - trackLeft - (m_thumbSize / 2.f);
                newValue = relativeX / usableLength;
            }
            setValue(newValue);
            m_isDragging = true;
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
    {
        m_isDragging = false;
    }
    else if (m_orientation == Orientation::Vertical && event.type == sf::Event::MouseWheelScrolled && m_track.getSprite().getGlobalBounds().contains(mousePos))
    {
        setValue(m_value - event.mouseWheelScroll.delta * 0.05f);
    }
}

void Slider::update(const sf::RenderWindow& window)
{
    if (m_isDragging)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        float newValue = 0.f;
        float usableLength = m_length - m_thumbSize;
        if (usableLength <= 0) usableLength = 1.f;

        if (m_orientation == Orientation::Vertical)
        {
            float trackTop = m_track.getPosition().y - m_length / 2.f;
            float clampedMouseY = std::clamp(mousePos.y, trackTop + m_thumbSize/2.f, trackTop + m_length - m_thumbSize/2.f);
            newValue = (clampedMouseY - (trackTop + m_thumbSize/2.f)) / usableLength;
        }
        else
        {
            float trackLeft = m_track.getPosition().x - m_length / 2.f;
            float clampedMouseX = std::clamp(mousePos.x, trackLeft + m_thumbSize/2.f, trackLeft + m_length - m_thumbSize/2.f);
            newValue = (clampedMouseX - (trackLeft + m_thumbSize/2.f)) / usableLength;
        }
        setValue(newValue);
    }

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    bool isHovered = m_thumb.getSprite().getGlobalBounds().contains(mousePos);
    m_thumb.setVisualState(isHovered, m_isDragging);
}

void Slider::draw(sf::RenderTarget& target) const
{
    m_track.draw(target);
    m_thumb.draw(target);
}

void Slider::setOnValueChange(std::function<void(float)> cb)
{
    onValueChange = cb;
}
float Slider::getValue() const
{
    return m_value;
}

void Slider::setValue(float value)
{
    float oldValue = m_value;
    m_value = std::clamp(value, 0.f, 1.f);
    if (m_value != oldValue)
    {
        updateThumbPosition();
        if (onValueChange) onValueChange(m_value);
    }
}

void Slider::updateThumbPosition()
{
    sf::Vector2f trackPos = m_track.getPosition();
    float usableLength = m_length - m_thumbSize;
    float startOffset = -m_length / 2.f + m_thumbSize / 2.f;

    if (m_orientation == Orientation::Vertical)
    {
        float thumbY = trackPos.y + startOffset + (usableLength * m_value);
        m_thumb.setPosition({trackPos.x, thumbY});
    }
    else
    {
        float thumbX = trackPos.x + startOffset + (usableLength * m_value);
        m_thumb.setPosition({thumbX, trackPos.y});
    }
}

}
