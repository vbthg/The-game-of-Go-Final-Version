#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

namespace UI
{

class Button
{
public:

    Button() : normalColor(sf::Color::White), hovered(false), pressed(false), m_isInteractive(true) { }

    Button(const sf::Texture& textureRef,
           const sf::Vector2f& position,
           bool isInteractive = true);


    virtual ~Button() = default;

    void setPosition(const sf::Vector2f& pos);

    void setScale(float scale);

    void setOnClick(std::function<void()> cb);

    sf::FloatRect getLocalBounds() const;

    sf::Vector2f getPosition() const;

    sf::Sprite& getSprite();

    bool handleEvent(const sf::Event& ev, const sf::RenderWindow& window);

    void update(const sf::RenderWindow& window);

    void draw(sf::RenderTarget& target) const;

    void setVisualState(bool isHovered, bool isPressed);

    bool isHoveredAndInteractive() const;

private:

    void applyVisualState();


    sf::Sprite m_sprite;     // Sprite của nút

    // Các màu sắc cho các trạng thái
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color pressedColor;

    // Trạng thái logic
    bool m_isInteractive; // Nút có hoạt động không
    bool hovered;         // Chuột đang trỏ vào hay ko
    bool pressed;         // Đang bị nhấn giữ hay ko

    // Hàm callback
    std::function<void()> onClick; // Hàm được gọi khi click
};

}
