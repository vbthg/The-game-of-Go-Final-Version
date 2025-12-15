#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <cmath> // Dùng cho std::abs để kiểm tra vị trí easing
#include "Button.h"
#include <iostream>

namespace UI
{

Button::Button(const std::string& text,
               const sf::Texture& textureRef,
               const sf::Vector2f& position,
               const sf::Font& fontRef,
               const unsigned int& charSize):

               hovered(false),
               pressed(false),
               onClick(nullptr)
               {
                   m_sprite.setTexture(textureRef);

                   normalColor = sf::Color::White;
                   hoverColor = sf::Color(200, 200, 200);
                   pressedColor = sf::Color(150, 150, 150);

                   /*
                   đặt điểm gốc của sprite về chính giữa
                   */
                   sf::FloatRect spriteSize = m_sprite.getLocalBounds();
                   float centerX = (spriteSize.left + spriteSize.width) / 2.f;
                   float centerY = (spriteSize.top + spriteSize.height) / 2.f;
                   m_sprite.setOrigin(sf::Vector2f(centerX, centerY));

                   setLabel(text, charSize);
                   label.setFont(fontRef);
                   setPosition(position);
               }


void Button::setLabel(const std::string& text, const unsigned int &charSize)
{
//    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(charSize);


    sf::FloatRect labelSize = label.getLocalBounds();
    float center_x = (labelSize.left + labelSize.width) / 2.f,
         center_y = (labelSize.top + labelSize.height) / 2.f;
    label.setOrigin(std::round(center_x), std::round(center_y));

}

void Button::setPosition(const sf::Vector2f& pos)
{
    m_sprite.setPosition(pos);
    label.setPosition(pos);
}

void Button::setScale(float scale)
{
    m_sprite.setScale(scale, scale);
    label.setScale(scale, scale);
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
    if(ev.type == sf::Event::MouseButtonReleased
    && ev.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(ev.mouseButton.x, ev.mouseButton.y));
        if(m_sprite.getGlobalBounds().contains(mousePos))
        {
            if(onClick != nullptr)
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
    hovered = pressed = false;

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    if(m_sprite.getGlobalBounds().contains(mousePos))
    {
        hovered = true;

        if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
            pressed = true;
    }

    applyVisualState();
}

void Button::draw(sf::RenderTarget& target) const
{
    target.draw(m_sprite);
    target.draw(label);
}

void Button::setOnClick(std::function<void()> cb)
{
    onClick = cb;
}

sf::Sprite& Button::getSprite()
{
    return m_sprite;
}

void Button::applyShadow()
{
    sf::Vector2f labelPos = label.getPosition();
    ShadowLabel.setPosition(labelPos.x + 2.0f, labelPos.y + 2.0f);
}

void Button::applyVisualState()
{
    if(pressed) m_sprite.setColor(pressedColor);
    else if(hovered) m_sprite.setColor(hoverColor);
    else m_sprite.setColor(normalColor);
}



} // namespace UI
