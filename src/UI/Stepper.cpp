#include "Stepper.h"
#include "ResourceManager.h"
#include <algorithm>

namespace UI
{

Stepper::Stepper(sf::Vector2f minusBtnPos,
                 float distance,
                 const std::vector<std::string>& options,
                 int defaultIndex) :
    m_options(options),
    m_currentIndex(defaultIndex),
    onChange(nullptr),

    m_prevBtn(ResourceManager::getInstance().getTexture("stepper_minus_btn"), {0,0}, true),
    m_nextBtn(ResourceManager::getInstance().getTexture("stepper_plus_btn"), {0,0}, true)
{

    m_font = ResourceManager::getInstance().getFont("main_font");


    m_valueLabel.setFont(m_font);
    m_valueLabel.setCharacterSize(24);
    m_valueLabel.setFillColor(sf::Color::Black);


    m_prevBtn.setPosition(minusBtnPos);

    m_nextBtn.setPosition({minusBtnPos.x + distance, minusBtnPos.y});

    m_centerPosition = {minusBtnPos.x + distance / 2.f, minusBtnPos.y};

    m_valueLabel.setPosition(m_centerPosition);


    if(m_currentIndex < 0) m_currentIndex = 0;
    if(m_currentIndex >= (int)m_options.size()) m_currentIndex = (int)m_options.size() - 1;

    updateLabel();

    m_prevBtn.setOnClick([this](){ this->prevOption(); });
    m_nextBtn.setOnClick([this](){ this->nextOption(); });
}

void Stepper::handleEvent(sf::Event& event, const sf::RenderWindow& window)
{
    m_prevBtn.handleEvent(event, window);
    m_nextBtn.handleEvent(event, window);
}

void Stepper::update(const sf::RenderWindow& window)
{
    m_prevBtn.update(window);
    m_nextBtn.update(window);
}

void Stepper::draw(sf::RenderTarget& target) const
{
    target.draw(m_valueLabel);
    m_prevBtn.draw(target);
    m_nextBtn.draw(target);
}

void Stepper::setOnChange(std::function<void(int)> cb) { onChange = cb; }
int Stepper::getSelectedIndex() const { return m_currentIndex; }
std::string Stepper::getSelectedString() const
{
    if(m_options.empty()) return "";
    return m_options[m_currentIndex];
}

void Stepper::setSelectedIndex(int index)
{
    if(index >= 0 && index < (int)m_options.size())
    {
        m_currentIndex = index;
        updateLabel();
        if(onChange) onChange(m_currentIndex);
    }
}

void Stepper::prevOption()
{
    if(m_options.empty()) return;
    m_currentIndex = (m_currentIndex - 1 + m_options.size()) % m_options.size();
    updateLabel();
    if(onChange) onChange(m_currentIndex);
}

void Stepper::nextOption()
{
    if(m_options.empty()) return;
    m_currentIndex = (m_currentIndex + 1) % m_options.size();
    updateLabel();
    if(onChange) onChange(m_currentIndex);
}

void Stepper::updateLabel()
{
    if(m_options.empty()) return;
    m_valueLabel.setString(m_options[m_currentIndex]);

    sf::FloatRect bounds = m_valueLabel.getLocalBounds();


    m_valueLabel.setOrigin(
        bounds.left + bounds.width / 2.f,
        bounds.top + bounds.height / 2.f
    );

    m_valueLabel.setPosition(m_centerPosition);
}

}
