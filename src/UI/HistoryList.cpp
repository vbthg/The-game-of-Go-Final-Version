#include "HistoryList.h"
#include <iostream>
#include <cmath>

namespace UI
{

const float ITEM_HEIGHT = 50.f;
const float SCROLLBAR_WIDTH = 13.f;
const float PADDING_X = 30.f;
const float PADDING_Y = 35.f;
const float ICON_OFFSET_X = 60.f;
const float TEXT_OFFSET_X = 140.f;

HistoryList::HistoryList(sf::Vector2f position, sf::Vector2f size,
                         const sf::Texture& bgTex,
                         const sf::Texture& sliderTrack,
                         const sf::Texture& blackIconTex,
                         const sf::Texture& whiteIconTex,
                         const sf::Font& font) :
    m_position(position),
    m_size(size),
    m_totalContentHeight(0.f),
    m_isScrollbarVisible(false),
    m_blackStoneTex(blackIconTex),
    m_whiteStoneTex(whiteIconTex),
    m_font(font)
{
    m_background.setTexture(bgTex);
    m_background.setOrigin(m_size.x / 2.f, m_size.y / 2.f);
    m_background.setPosition(m_position);

    float leftX = m_position.x - (m_size.x / 2.f);
    float topY  = m_position.y - (m_size.y / 2.f);

    float sliderX = leftX + 10.f;

    float sliderHeight = std::round(m_size.y - (PADDING_Y * 1.5f));
    float sliderY = std::round(topY + PADDING_Y / 1.2f);

    m_scrollbar = std::make_unique<UI::Slider>
    (
        UI::Orientation::Vertical,
        sliderTrack,
        sliderX,
        sliderY + 255,
        sliderHeight
    );

    m_scrollbar->setOnValueChange([this](float percent)
    {
        this -> onScroll(percent);
    });
    m_scrollbar -> setValue(1.0f);

    float viewW = m_size.x - SCROLLBAR_WIDTH - (PADDING_X * 2.5f);
    float viewH = m_size.y - (PADDING_Y * 2.f);

    m_view.setSize(viewW, viewH);
    m_view.setCenter(viewW / 2.f, viewH / 2.f);
}

void HistoryList::handleEvent(sf::Event& event, const sf::RenderWindow& window)
{
    if (m_scrollbar)
    {
        m_scrollbar -> handleEvent(event, window);
    }
}

void HistoryList::update(const sf::RenderWindow& window)
{
    if (m_scrollbar)
    {
        m_scrollbar -> update(window);
    }
}

void HistoryList::draw(sf::RenderTarget& target) const
{
    target.draw(m_background);

    if (m_scrollbar)
    {
        m_scrollbar->draw(target);
    }

    sf::View oldView = target.getView();

    float leftX = m_position.x - (m_size.x / 2.f);
    float topY  = m_position.y - (m_size.y / 2.f);

    float viewX = leftX + SCROLLBAR_WIDTH + 15.f;
    float viewY = topY + PADDING_Y;

    float viewW = m_size.x - SCROLLBAR_WIDTH - (PADDING_X * 2.5f);
    float viewH = m_size.y - (PADDING_Y * 2.f);

    sf::Vector2f windowSize(target.getSize());

    sf::FloatRect viewport
    (
        viewX / windowSize.x,
        viewY / windowSize.y,
        viewW / windowSize.x,
        viewH / windowSize.y
    );

    sf::View scrollingView = m_view;
    scrollingView.setViewport(viewport);
    target.setView(scrollingView);

    for (const auto& entry : m_entries)
    {
        target.draw(entry.stoneIcon);
        target.draw(entry.moveText);
    }

    target.setView(oldView);
}

void HistoryList::addMove(bool isBlack, const std::string& moveCoords)
{
    bool shouldAutoScroll = true;
    if (m_scrollbar && m_scrollbar -> getValue() < 0.95f && m_totalContentHeight > m_view.getSize().y)
    {
        shouldAutoScroll = false;
    }

    HistoryEntry newEntry;
    newEntry.stoneIcon.setTexture(isBlack ? m_blackStoneTex : m_whiteStoneTex);

    float iconScale = (ITEM_HEIGHT - 8.f) / newEntry.stoneIcon.getLocalBounds().height;

    newEntry.moveText.setFont(m_font);
    newEntry.moveText.setString(moveCoords);
    newEntry.moveText.setCharacterSize(18);
    newEntry.moveText.setFillColor(sf::Color::Black);

    float newY = m_entries.size() * ITEM_HEIGHT + (ITEM_HEIGHT / 2.f);

    newEntry.stoneIcon.setOrigin(newEntry.stoneIcon.getLocalBounds().width / 2.f, newEntry.stoneIcon.getLocalBounds().height / 2.f);
    newEntry.stoneIcon.setPosition(ICON_OFFSET_X, newY);

    sf::FloatRect textBounds = newEntry.moveText.getLocalBounds();
    newEntry.moveText.setOrigin(0, textBounds.height / 2.f);

    newEntry.moveText.setPosition(TEXT_OFFSET_X, newY - 4.f);

    m_entries.push_back(newEntry);
    m_totalContentHeight += ITEM_HEIGHT;

    updateScrollbarState();

    if (shouldAutoScroll && m_scrollbar)
    {
        m_scrollbar -> setValue(1.0f);
        onScroll(1.0f);
    }
}


void HistoryList::clear()
{
    m_entries.clear();
    m_totalContentHeight = 0.f;
    updateScrollbarState();
    if (m_scrollbar)
    {
        m_scrollbar -> setValue(0.f);
        onScroll(0.f);
    }
}

void HistoryList::onScroll(float percent)
{
    float viewHeight = m_view.getSize().y;
    if (m_totalContentHeight <= viewHeight)
    {
        m_view.setCenter(m_view.getSize().x / 2.f, viewHeight / 2.f);
        return;
    }

    float scrollableRange = m_totalContentHeight - viewHeight;
    float newTopY = scrollableRange * percent;
    m_view.setCenter(m_view.getSize().x / 2.f, newTopY + (viewHeight / 2.f));
}

void HistoryList::updateScrollbarState()
{
    float viewHeight = m_view.getSize().y;
    if (m_totalContentHeight > viewHeight)
        m_isScrollbarVisible = true;
    else
        m_isScrollbarVisible = false;
}

void HistoryList::removeLastMove()
{
    if (m_entries.empty()) return;
    m_entries.pop_back();
    m_totalContentHeight -= ITEM_HEIGHT;
    if (m_totalContentHeight < 0) m_totalContentHeight = 0;
    updateScrollbarState();
    if (m_scrollbar) onScroll(m_scrollbar->getValue());
}

std::string HistoryList::getLastMoveNotation() const
{
    if (m_entries.empty()) return "";
    return m_entries.back().moveText.getString().toAnsiString();
}

}
