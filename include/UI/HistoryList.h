// include/HistoryList.h
#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include "Slider.h"

namespace UI
{

class HistoryList
{
private:
    struct HistoryEntry
    {
        bool isBlack;
        sf::Sprite stoneIcon;
        sf::Text moveText;

        sf::Text indexText;
    };

public:
    HistoryList(sf::Vector2f position, sf::Vector2f size,
                const sf::Texture& bgTex,
                const sf::Texture& sliderTrack,
                const sf::Texture& blackIconTex,
                const sf::Texture& whiteIconTex,
                const sf::Font& font);
    ~HistoryList() = default;

    void handleEvent(sf::Event& event, const sf::RenderWindow& window);
    void update(const sf::RenderWindow& window);
    void draw(sf::RenderTarget& target) const;

    void addMove(bool isBlack, const std::string& moveCoords);
    void removeLastMove(); // Xóa dòng cuối
    void clear();

    void updateTheme(const sf::Texture* blackTex, const sf::Texture* whiteTex);

    std::string getLastMoveNotation() const;

private:
    void onScroll(float percent);
    void updateScrollbarState();

    sf::Vector2f m_position;
    sf::Vector2f m_size;
    sf::Sprite m_background;
    sf::View m_view;
    std::unique_ptr<UI::Slider> m_scrollbar;

    std::vector<HistoryEntry> m_entries;
    float m_totalContentHeight;
    bool m_isScrollbarVisible;

    const sf::Texture* m_blackStoneTex;
    const sf::Texture* m_whiteStoneTex;
    const sf::Font& m_font;
};

}
