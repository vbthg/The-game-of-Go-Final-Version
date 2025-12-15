#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include <functional>
#include "Button.h"

namespace UI
{

class RadioGroup
{
public:
    RadioGroup(int numOptions,
               sf::Vector2f position,
               float spacing);

    void handleEvent(sf::Event& event, const sf::RenderWindow& window);
    void update(const sf::RenderWindow& window);
    void draw(sf::RenderTarget& target) const;

    void setOnChange(std::function<void(int)> cb);
    int getSelectedIndex() const;
    void setSelectedIndex(int index);

private:
    void onOptionClicked(int index);
    void generateButtonTexture();

    int m_activeIndex;
    std::vector<std::unique_ptr<Button>> m_buttons;
    std::function<void(int)> onChange;

    sf::Texture m_generatedBtnTexture;

};

}
