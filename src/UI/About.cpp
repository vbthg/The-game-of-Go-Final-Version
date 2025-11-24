#include "About.h"
#include <iostream>

const float PANEL_WIDTH = 600.f;
const float PANEL_HEIGHT = 500.f;
const float SCROLLBAR_WIDTH = 20.f;

About::About(sf::RenderWindow& window) :
    m_window(window),
    m_requestedState(GameStateType::NoChange),
    m_scrollOffset(0.f),
    m_maxScroll(0.f),
    m_viewHeight(0.f),
    m_backBtn(ResourceManager::getInstance().getTexture("about_back_btn"), {62.f, 57.f}, true)
{
    createUI();
}

void About::createUI()
{
    sf::Vector2u windowSize = m_window.getSize();

    m_background.setColor(sf::Color(0, 0, 0, 150));

    m_panel.setSize(sf::Vector2f(PANEL_WIDTH, PANEL_HEIGHT));
    m_panel.setFillColor(sf::Color(50, 50, 50, 230));
    m_panel.setOutlineColor(sf::Color::White);
    m_panel.setOutlineThickness(2.f);
    m_panel.setOrigin(PANEL_WIDTH / 2, PANEL_HEIGHT / 2);
    m_panel.setPosition(windowSize.x / 2.f, windowSize.y / 2.f);

    sf::Font& font = ResourceManager::getInstance().getFont("main_font");


    m_titleText.setFont(font);
    m_titleText.setString("ABOUT PROJECT");
    m_titleText.setCharacterSize(32);
    m_titleText.setFillColor(sf::Color::White);

    sf::FloatRect titleBounds = m_titleText.getLocalBounds();
    m_titleText.setOrigin(titleBounds.width / 2, 0);
    m_titleText.setPosition(windowSize.x / 2.f, (windowSize.y / 2.f) - (PANEL_HEIGHT / 2) + 20.f);


    m_contentText.setFont(font);
    m_contentText.setCharacterSize(20);
    m_contentText.setFillColor(sf::Color(220, 220, 220));

    std::string content =
        "UNIVERSITY OF SCIENCE - VNUHCM\n"
        "DEPARTMENT OF SOFTWARE ENGINEERING\n\n"
        "SUBJECT: INTRODUCTION TO COMPUTER SCIENCE\n"
        "PROJECT: GO GAME (CO VAY)\n\n"
        "--- TEAM MEMBERS ---\n"
        "1. [Vo Ba Thong] - [25125036]\n\n"
        "--- FEATURES ---\n"
        "- 2 Player Mode\n"
        "- Player vs AI (Minimax Alpha-Beta)\n"
        "- Undo/Redo & Save/Load Game\n\n"
        "Copyright 2025. All rights reserved.\n"
        "\n\nThank you for playing!\n\n";

    m_contentText.setString(content);

    m_contentText.setPosition(0, 0);

    float headerHeight = 80.f;
    float footerHeight = 60.f;
    m_viewHeight = PANEL_HEIGHT - headerHeight - footerHeight;
    float contentWidth = PANEL_WIDTH - 40.f - SCROLLBAR_WIDTH;

    m_scrollView.setSize(contentWidth, m_viewHeight);
    m_scrollView.setCenter(contentWidth / 2.f, m_viewHeight / 2.f);

    float viewportX = (windowSize.x - contentWidth) / 2.f / windowSize.x;

    float panelTopY = (windowSize.y - PANEL_HEIGHT) / 2.f;
    float viewportY = (panelTopY + headerHeight) / windowSize.y;
    float viewportW = contentWidth / windowSize.x;
    float viewportH = m_viewHeight / windowSize.y;

    m_scrollView.setViewport(sf::FloatRect(viewportX, viewportY, viewportW, viewportH));

    float totalTextHeight = m_contentText.getGlobalBounds().height;
    if (totalTextHeight > m_viewHeight)
    {
        m_maxScroll = totalTextHeight - m_viewHeight;
    }
    else
    {
        m_maxScroll = 0;
    }

    float sliderX = (windowSize.x / 2.f) + (PANEL_WIDTH / 2.f) - SCROLLBAR_WIDTH - 10.f;
    float sliderY = panelTopY + headerHeight;

    sf::Texture& sliderTrackTex = ResourceManager::getInstance().getTexture("about_slider_track");

    m_scrollbar = std::make_unique<UI::Slider>
    (
        UI::Orientation::Vertical,
        sliderTrackTex,
        510.f, 450.f,
        m_viewHeight
    );

    m_scrollbar -> setOnValueChange([this](float percent)
    {
        this -> updateScroll(percent);
    });

    sf::FloatRect btnSize = m_backBtn.getSprite().getLocalBounds();
    m_backBtn.getSprite().setOrigin(btnSize.width / 2.f, btnSize.height / 2.f);

    float btnY = (windowSize.y / 2.f) + (PANEL_HEIGHT / 2.f) - 50.f;
    m_backBtn.setPosition(sf::Vector2f(62.f, 57.f));

    m_backBtn.setOnClick([this]()
    {
        this -> onBackClick();
    });
}

void About::onBackClick()
{
    m_requestedState = GameStateType::MainMenu;
}

void About::updateScroll(float percent)
{
    if (m_maxScroll <= 0) return;

    float currentScrollY = m_maxScroll * percent;

    float newCenterY = (m_viewHeight / 2.f) + currentScrollY;

    sf::Vector2f center = m_scrollView.getCenter();
    center.y = newCenterY;
    m_scrollView.setCenter(center);
}

void About::handleEvent(sf::Event& event)
{
    m_backBtn.handleEvent(event, m_window);

    if (m_scrollbar)
    {
        m_scrollbar -> handleEvent(event, m_window);
    }
}

GameStateType About::update(float deltaTime)
{
    m_backBtn.update(m_window);

    if (m_scrollbar)
    {
        m_scrollbar -> update(m_window);
    }

    return m_requestedState;
}

void About::draw()
{
    m_window.draw(m_background);

    m_window.draw(m_panel);
    m_window.draw(m_titleText);

    sf::View defaultView = m_window.getView();

    m_window.setView(m_scrollView);

    m_window.draw(m_contentText);

    m_window.setView(defaultView);

    if (m_scrollbar)
    {
        m_scrollbar -> draw(m_window);
    }
    m_backBtn.draw(m_window);
}
