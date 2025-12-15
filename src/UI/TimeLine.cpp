#include "Timeline.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace UI
{
const float PI = std::acos(-1.f);

Timeline::Timeline(sf::Vector2f position, sf::Vector2f size,
                   const sf::Texture& bgTex, const sf::Texture& tooltipBgTex, const sf::Font& font)
    : m_position(position)
    , m_size(size)
    , m_font(font)
    , m_isTooltipVisible(false)
{
    m_background.setTexture(bgTex);
    sf::FloatRect bounds = m_background.getLocalBounds();
    m_background.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    m_background.setPosition(m_position);

    m_tooltipBackground.setTexture(tooltipBgTex);
    m_tooltipText.setFont(m_font);
    m_tooltipText.setCharacterSize(14);
    m_tooltipText.setFillColor(sf::Color::White);
}

void Timeline::handleEvent(sf::Event& event, const sf::RenderWindow& window)
{
    (void)event;
    (void)window;
}

void Timeline::draw(sf::RenderTarget& target) const
{
    target.draw(m_background);

    for(const auto& seg : m_segments)
    {
        target.draw(seg.shape);
    }

    if(m_isTooltipVisible)
    {
        target.draw(m_tooltipBackground);
        target.draw(m_tooltipText);
    }
}

void Timeline::update(float deltaTime, const sf::RenderWindow& window)
{
    float currentX = m_position.x - m_size.x / 2.f;
    bool needsRecalculate = false;
    bool foundHover = false;

    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

    for(auto it = m_segments.begin(); it != m_segments.end(); )
    {
        auto& seg = *it;

        if(seg.state != TimelineSegment::State::STATIC)
        {
            seg.animTimer = std::min(seg.animTimer + deltaTime, seg.animDuration);
            float ratio = (seg.animDuration > 0) ? seg.animTimer / seg.animDuration : 1.f;

            float easedRatio = std::sin(ratio * (PI / 2.f));

            float currentWidth = seg.startWidth + (seg.targetWidth - seg.startWidth) * easedRatio;
            seg.shape.setSize({currentWidth, m_size.y});

            if(ratio >= 1.f)
            {
                if(seg.state == TimelineSegment::State::SHRINKING_OUT)
                {
                    TimelineSegment storedSeg = seg;
                    storedSeg.state = TimelineSegment::State::STATIC;
                    m_overflowSegments.push_back(storedSeg);

                    it = m_segments.erase(it);
                    needsRecalculate = true;
                    continue;
                }

                needsRecalculate = true;
                seg.state = TimelineSegment::State::STATIC;
                seg.shape.setSize({seg.targetWidth, m_size.y});
            }
        }

        float currentY = m_position.y - m_size.y / 2.f;
        seg.shape.setPosition(currentX, currentY);

        if(!foundHover && seg.shape.getGlobalBounds().contains(mousePos))
        {
             foundHover = true;
             m_isTooltipVisible = true;

             std::stringstream ss;
             ss << (seg.isBlack ? "Black" : "White") << " (" << seg.moveCoords << ")\n"
                << std::fixed << std::setprecision(1) << seg.actualTime << "s";
             m_tooltipText.setString(ss.str());

             sf::FloatRect textBounds = m_tooltipText.getLocalBounds();
             sf::Vector2u tipBgSize = m_tooltipBackground.getTexture()->getSize();

             float padding = 10.f;
             float tipWidth = textBounds.width + padding * 2;
             float tipHeight = textBounds.height + padding * 2;

             if(tipBgSize.x > 0)
             {
                 m_tooltipBackground.setScale(tipWidth / tipBgSize.x, tipHeight / tipBgSize.y);
             }

             m_tooltipBackground.setPosition(mousePos.x + 10.f, mousePos.y - tipHeight - 10.f);

             sf::Vector2f bgPos = m_tooltipBackground.getPosition();
             m_tooltipText.setPosition(bgPos.x + padding, bgPos.y + padding);
        }

        currentX += seg.shape.getSize().x;
        ++it;
    }

    if(!foundHover) m_isTooltipVisible = false;
    if(needsRecalculate) recalculateProportions();
}

void Timeline::addMove(float time, bool isBlack, const std::string& coords, bool animate)
{
    if(time <= 0.f) time = 0.1f;

    if(m_segments.size() >= static_cast<size_t>(m_maxMoves))
    {
        auto& oldSegment = m_segments.front();

        if(animate)
        {
            if(oldSegment.state == TimelineSegment::State::STATIC)
            {
                oldSegment.state = TimelineSegment::State::SHRINKING_OUT;
                oldSegment.animTimer = 0.f;
                oldSegment.startWidth = oldSegment.shape.getSize().x;
                oldSegment.targetWidth = 0.f;
            }
        }
        else
        {
            m_overflowSegments.push_back(oldSegment);
            m_segments.pop_front();
        }
    }

    TimelineSegment newSegment;
    newSegment.actualTime = time;
    newSegment.isBlack = isBlack;
    newSegment.moveCoords = coords;

    newSegment.shape.setFillColor(isBlack ? m_blackColor : m_whiteColor);
    newSegment.shape.setOutlineThickness(-1.f);
    newSegment.shape.setOutlineColor(sf::Color(100, 100, 100));

    if(animate)
    {
        newSegment.state = TimelineSegment::State::SLIDING_IN;
        newSegment.animTimer = 0.f;
        newSegment.startWidth = 0.f;
        newSegment.targetWidth = m_slideInWidth;
        newSegment.shape.setSize({0.f, m_size.y});
    }
    else
    {
        newSegment.state = TimelineSegment::State::STATIC;
    }

    m_segments.push_back(newSegment);
    recalculateProportions();
}

void Timeline::removeLastSegment()
{
    if(m_segments.empty()) return;

    m_segments.pop_back();

    if(!m_overflowSegments.empty())
    {
        TimelineSegment recoveredSeg = m_overflowSegments.back();
        m_overflowSegments.pop_back();

        recoveredSeg.state = TimelineSegment::State::STATIC;

        m_segments.push_front(recoveredSeg);
    }

    recalculateProportions();
}

float Timeline::getLastMoveTime() const
{
    if(m_segments.empty()) return 0.f;
    return m_segments.back().actualTime;
}

void Timeline::clear()
{
    m_segments.clear();
    m_overflowSegments.clear();
}

void Timeline::recalculateProportions()
{
    float totalStaticTime = 0.f;
    float totalAnimatedWidth = 0.f;

    for(const auto& seg : m_segments)
    {
        if(seg.state == TimelineSegment::State::STATIC)
        {
            totalStaticTime += seg.actualTime;
        }
        else
        {
            totalAnimatedWidth += seg.targetWidth;
        }
    }

    float availableStaticWidth = m_size.x - totalAnimatedWidth;
    if(availableStaticWidth < 0) availableStaticWidth = 0;

    if(totalStaticTime <= 0.f) return;

    for(auto& seg : m_segments)
    {
        if(seg.state == TimelineSegment::State::STATIC)
        {
            float ratio = seg.actualTime / totalStaticTime;
            float newWidth = ratio * availableStaticWidth;

            seg.shape.setSize({newWidth, m_size.y});

            seg.startWidth = newWidth;
            seg.targetWidth = newWidth;
        }
    }
}

}
