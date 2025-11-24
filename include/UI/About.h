#pragma once

#include "GameState.h"
#include "Button.h"
#include "Slider.h"
#include "ResourceManager.h"
#include <memory>

class About : public GameState
{
public:
    About(sf::RenderWindow& window);
    virtual ~About() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;

private:
    void createUI();
    void updateScroll(float percent);
    void onBackClick();

    sf::RenderWindow& m_window;
    GameStateType m_requestedState;


    sf::Sprite m_background; // Ảnh nền mờ
    sf::RectangleShape m_panel; // Cái cửa sổ nhỏ ở giữa


    sf::Text m_titleText;
    sf::Text m_contentText; // Nội dung dài


    std::unique_ptr<UI::Slider> m_scrollbar;
    sf::View m_scrollView;  // Camera ảo để cắt chữ

    float m_scrollOffset;
    float m_maxScroll;
    float m_viewHeight;     // Chiều cao vùng nhìn thấy được


    UI::Button m_backBtn;
};
