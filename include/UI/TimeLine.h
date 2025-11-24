#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <deque>
#include <vector>

namespace UI
{

class Timeline
{
private:
    struct TimelineSegment
    {
        float actualTime;
        bool isBlack;
        std::string moveCoords;
        sf::RectangleShape shape;

        enum class State
        {
            STATIC,
            SLIDING_IN,
            SHRINKING_OUT
        };
        State state = State::STATIC;

        float animTimer = 0.f;
        float animDuration = 0.3f;
        float startWidth = 0.f;
        float targetWidth = 0.f;
    };

public:
    Timeline(sf::Vector2f position, sf::Vector2f size,
             const sf::Texture& bgTex, const sf::Texture& tooltipBgTex, const sf::Font& font);

    void handleEvent(sf::Event& event, const sf::RenderWindow& window);
    void update(float deltaTime, const sf::RenderWindow& window);
    void draw(sf::RenderTarget& target) const;

    void addMove(float time, bool isBlack, const std::string& coords, bool animate = true);

    void removeLastSegment();
    void clear();

    float getLastMoveTime() const; // Lấy thời gian để Redo

private:
    void recalculateProportions();

    sf::Vector2f m_position;
    sf::Vector2f m_size;
    sf::Sprite m_background;
    sf::Color m_blackColor = sf::Color(40, 40, 40);
    sf::Color m_whiteColor = sf::Color(220, 220, 220);

    const int m_maxMoves = 20;
    const float m_slideInWidth = 40.f;

    std::deque<TimelineSegment> m_segments;
    std::vector<TimelineSegment> m_overflowSegments;

    sf::Sprite m_tooltipBackground;
    sf::Text m_tooltipText;
    const sf::Font& m_font;
    bool m_isTooltipVisible;
};

}
