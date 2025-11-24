#include "RadioButton.h"

namespace UI
{

RadioGroup::RadioGroup(int numOptions,
                       sf::Vector2f position,
                       float spacing) :
    m_activeIndex(0),
    onChange(nullptr)
{
    generateButtonTexture();

    float startX = position.x;
    float startY = position.y;

    unsigned int texWidth = m_generatedBtnTexture.getSize().x;

    for(int i = 0; i < numOptions; ++i)
    {
        float buttonX = startX + (i * (texWidth + spacing)) + (texWidth / 2.f);

        auto button = std::make_unique<Button>
        (
            m_generatedBtnTexture,
            sf::Vector2f{buttonX, startY},
            true
        );

        m_buttons.push_back(std::move(button));
    }
}

void RadioGroup::generateButtonTexture()
{
    float radius = 12.f;
    float outline = 2.f;
    unsigned int dim = (unsigned int)((radius + outline) * 2);

    sf::RenderTexture rt;
    rt.create(dim, dim);
    rt.clear(sf::Color::Transparent);

    sf::CircleShape circle(radius);
    circle.setOrigin(radius, radius);
    circle.setPosition(dim / 2.f, dim / 2.f);

    circle.setFillColor(sf::Color::White);
    circle.setOutlineThickness(outline);
    circle.setOutlineColor(sf::Color(50, 50, 50));

    rt.draw(circle);
    rt.display();

    m_generatedBtnTexture = rt.getTexture();
    m_generatedBtnTexture.setSmooth(true);
}

void RadioGroup::handleEvent(sf::Event& event, const sf::RenderWindow& window)
{
    if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        for(size_t i = 0; i < m_buttons.size(); ++i)
        {
            if (m_buttons[i] -> getSprite().getGlobalBounds().contains(mousePos))
            {
                onOptionClicked(static_cast<int>(i));
                break;
            }
        }
    }
}

void RadioGroup::update(const sf::RenderWindow& window)
{
    sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
    for(size_t i = 0; i < m_buttons.size(); ++i)
    {
        bool isHovered = m_buttons[i]->getSprite().getGlobalBounds().contains(mousePos);
        bool isActive = (static_cast<int>(i) == m_activeIndex);
        m_buttons[i] -> setVisualState(isHovered, isActive);
    }
}

void RadioGroup::draw(sf::RenderTarget& target) const
{
    for(const auto& button : m_buttons)
    {
        button -> draw(target);
    }
}

void RadioGroup::onOptionClicked(int index)
{
    if(m_activeIndex == index) return;
    m_activeIndex = index;
    if(onChange) onChange(m_activeIndex);
}

void RadioGroup::setOnChange(std::function<void(int)> cb) { onChange = cb; }
int RadioGroup::getSelectedIndex() const { return m_activeIndex; }
void RadioGroup::setSelectedIndex(int index)
{
    if (index >= 0 && index < static_cast<int>(m_buttons.size()))
        m_activeIndex = index;
}

}
