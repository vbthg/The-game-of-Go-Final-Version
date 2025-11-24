#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include "Button.h"

namespace UI
{

class Stepper
{
public:

    Stepper(sf::Vector2f minusBtnPos,
            float distance,
            const std::vector<std::string>& options,
            int defaultIndex = 0);

    void handleEvent(sf::Event& event, const sf::RenderWindow& window);
    void update(const sf::RenderWindow& window);
    void draw(sf::RenderTarget& target) const;

    void setOnChange(std::function<void(int)> cb);
    int getSelectedIndex() const;
    std::string getSelectedString() const;
    void setSelectedIndex(int index);

private:
    void nextOption();
    void prevOption();
    void updateLabel();

    sf::Text m_valueLabel;
    sf::Font m_font;

    Button m_nextBtn;
    Button m_prevBtn;

    std::vector<std::string> m_options;
    int m_currentIndex;

    std::function<void(int)> onChange;

    sf::Vector2f m_centerPosition;
};

}
