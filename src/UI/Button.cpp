#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <cmath>
#include "Button.h"
#include <iostream>

namespace UI
{

Button::Button(const sf::Texture& textureRef,
               const sf::Vector2f& position,
               bool isInteractive):

               hovered(false),
               pressed(false),
               m_isInteractive(isInteractive),
               onClick(nullptr)
               {
                   m_sprite.setTexture(textureRef);

                   normalColor = sf::Color::White;
                   hoverColor = sf::Color(200, 200, 200);
                   pressedColor = sf::Color(150, 150, 150);

                   sf::FloatRect spriteSize = m_sprite.getLocalBounds();
                   float centerX = (spriteSize.left + spriteSize.width) / 2.f;
                   float centerY = (spriteSize.top + spriteSize.height) / 2.f;
                   m_sprite.setOrigin(sf::Vector2f(centerX, centerY));

                   setPosition(position);
               }

void Button::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
}

void Button::setScale(float scale)
{
    m_sprite.setScale(scale, scale);
}

sf::FloatRect Button::getLocalBounds() const
{
    return m_sprite.getLocalBounds();
}

sf::Vector2f Button::getPosition() const
{
    return m_sprite.getPosition();
}

bool Button::handleEvent(const sf::Event& ev, const sf::RenderWindow& window)
{
    if(!m_isInteractive) return false;

    if(ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(ev.mouseButton.x, ev.mouseButton.y));

        if(m_sprite.getGlobalBounds().contains(mousePos))
        {
            pressed = true;
            applyVisualState();
        }
    }

    if(ev.type == sf::Event::MouseButtonReleased && ev.mouseButton.button == sf::Mouse::Left)
    {
        if(pressed)
        {
            pressed = false;

            applyVisualState();

            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(ev.mouseButton.x, ev.mouseButton.y));

            if(m_sprite.getGlobalBounds().contains(mousePos) && onClick != nullptr)
            {
                onClick();
                return true;
            }
        }
    }

    return false;
}

void Button::update(const sf::RenderWindow& window)
{
    if(!m_isInteractive) return;

    hovered = false;

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    if(m_sprite.getGlobalBounds().contains(mousePos))
    {
        hovered = true;
    }

    applyVisualState();
}

void Button::draw(sf::RenderTarget& target) const
{
    target.draw(m_sprite);
}

void Button::setOnClick(std::function<void()> cb)
{
    onClick = cb;
}

sf::Sprite& Button::getSprite()
{
    return m_sprite;
}

void Button::setVisualState(bool isHovered, bool isPressed)
{
    if(!m_isInteractive) return;

    hovered = isHovered;
    pressed = isPressed;

    applyVisualState();
}

bool Button::isHoveredAndInteractive() const
{
    return (hovered && m_isInteractive);
}

void Button::applyVisualState()
{
    if(!m_isInteractive) return;

    if(pressed)
    {
        m_sprite.setColor(pressedColor);
    }
    else if(hovered)
    {
        m_sprite.setColor(hoverColor);
    }
    else
    {
        m_sprite.setColor(normalColor);
    }
}

}
