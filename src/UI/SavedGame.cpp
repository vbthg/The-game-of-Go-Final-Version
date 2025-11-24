#include "SavedGame.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>

std::vector<std::string> splitStr(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) tokens.push_back(token);
    return tokens;
}


SavedGame::SavedGame(sf::RenderWindow& window) :
    m_window(window),
    m_requestedState(GameStateType::NoChange),
    m_background(ResourceManager::getInstance().getTexture("savedgame_background")),
    m_backBtn(ResourceManager::getInstance().getTexture("savedgame_back_btn"), {100.f, 50.f}, true),
    m_scrollOffset(0.f),
    m_maxScroll(0.f),
    m_slotHeight(180.f),
    m_showPopup(false),
    m_targetDeleteIndex(-1),
    m_popupYesBtn(ResourceManager::getInstance().getTexture("savedgame_popupyes_btn"), {700.f, 500.f}, true),
    m_popupNoBtn(ResourceManager::getInstance().getTexture("savedgame_popupno_btn"), {900.f, 500.f}, true)
{
    m_backBtn.setOnClick([this]()
                         {
                             onBackClick();
                         });

    m_scrollbar = std::make_unique<UI::Slider>
    (
        UI::Orientation::Vertical,
        ResourceManager::getInstance().getTexture("savedgame_slidertrack_btn"),
        1580.f, 450.f, 800.f
    );
    m_scrollbar -> setOnValueChange([this](float p){ updateScroll(p); });

    m_popupOverlay.setSize((sf::Vector2f)m_window.getSize());
    m_popupOverlay.setFillColor(sf::Color(0, 0, 0, 200));

    m_popupBox.setSize({600.f, 300.f});
    m_popupBox.setOrigin(300.f, 150.f);
    m_popupBox.setPosition(window.getSize().x/2.f, window.getSize().y/2.f);
    m_popupBox.setFillColor(sf::Color(50, 50, 50));
    m_popupBox.setOutlineColor(sf::Color::White);
    m_popupBox.setOutlineThickness(2.f);

    m_popupText.setFont(ResourceManager::getInstance().getFont("main_font"));
    m_popupText.setString("Are you sure you want to delete?");
    m_popupText.setCharacterSize(30);
    sf::FloatRect tr = m_popupText.getLocalBounds();
    m_popupText.setOrigin(tr.width/2.f, tr.height/2.f);
    m_popupText.setPosition(window.getSize().x/2.f, window.getSize().y/2.f - 50.f);

    m_popupYesBtn.setOnClick([this]()
    {
        if (m_targetDeleteIndex >= 0 && m_targetDeleteIndex < m_slots.size())
        {
            std::filesystem::remove(m_slots[m_targetDeleteIndex] -> info.filename);
            std::filesystem::remove(m_slots[m_targetDeleteIndex] -> info.screenshotPath);
            loadSaveFiles();
        }
        m_showPopup = false;
    });
    m_popupNoBtn.setOnClick([this](){ m_showPopup = false; });

    loadSaveFiles();
}

void SavedGame::loadSaveFiles()
{
    m_slots.clear();
    namespace fs = std::filesystem;
    if (!fs::exists("assets/saves")) fs::create_directories("assets/saves");

    float y = 120.f;
    int idx = 0;

    for (const auto& entry : fs::directory_iterator("assets/saves"))
    {
        if (entry.path().extension() == ".txt")
        {
            std::ifstream file(entry.path());
            std::string line;

            if (std::getline(file, line))
            {
                auto parts = splitStr(line, '|');
                if (parts.size() >= 5)
                {
                    SaveInfo info;
                    info.filename = entry.path().string();

                    std::filesystem::path tempPath = entry.path();
                    tempPath.replace_extension(".png");
                    info.screenshotPath = tempPath.string();

                    info.userTitle = parts[0];
                    info.timestamp = parts[1];
                    info.boardSize = std::stoi(parts[2]);
                    info.modeStr = parts[3];
                    info.status = parts[4];

                    createSlotUI(idx, info, y + idx * m_slotHeight);
                    idx++;
                }
            }
        }
    }

    float totalHeight = idx * m_slotHeight;
    float viewHeight = 800.f;
    m_maxScroll = (totalHeight > viewHeight) ? (totalHeight - viewHeight) : 0.f;
    if (m_scrollbar) m_scrollbar -> setValue(0.f);
    m_scrollOffset = 0.f;
}

void SavedGame::createSlotUI(int index, const SaveInfo& info, float yPos)
{
    auto slot = std::make_unique<SaveSlotUI>();
    slot -> info = info;

    slot -> panel.setSize({1000.f, 160.f});
    slot -> panel.setFillColor(sf::Color(30, 30, 30, 220));
    slot -> panel.setOutlineColor(sf::Color::White);
    slot -> panel.setOutlineThickness(1.f);
    slot -> panel.setPosition(300.f, yPos);

    if (slot -> textureHolder.loadFromFile(info.screenshotPath))
    {
        slot -> textureHolder.setSmooth(true);
        slot -> thumbnail.setTexture(slot->textureHolder);
        sf::Vector2u size = slot->textureHolder.getSize();
        float sX = 140.f / size.x;
        float sY = 140.f / size.y;
        slot -> thumbnail.setScale(sX, sY);
        slot -> thumbnail.setPosition(310.f, std::round(yPos + 10.f));
    }

    const auto& font = ResourceManager::getInstance().getFont("main_font");

    slot -> textTitle.setFont(font);
    slot -> textTitle.setString(info.userTitle);
    slot -> textTitle.setCharacterSize(24);
    slot -> textTitle.setFillColor(sf::Color::White);
    slot -> textTitle.setPosition(470.f, std::round(yPos + 15.f));

    slot -> textDetail.setFont(font);
    std::string details = info.timestamp + "  |  " +
                          std::to_string(info.boardSize) + "x" + std::to_string(info.boardSize) + "  |  " +
                          info.modeStr;
    slot -> textDetail.setString(details);
    slot -> textDetail.setCharacterSize(18);
    slot -> textDetail.setFillColor(sf::Color(200, 200, 200));
    slot -> textDetail.setPosition(470.f, std::round(yPos + 55.f));

    slot -> textStatus.setFont(font);
    slot -> textStatus.setString("Status: " + info.status);
    slot -> textStatus.setCharacterSize(18);
    slot -> textStatus.setFillColor(sf::Color(255, 215, 0));
    slot -> textStatus.setPosition(470.f, std::round(yPos + 85.f));

    slot -> btnLoad = std::make_unique<UI::Button>
    (
        ResourceManager::getInstance().getTexture("savedgame_load_btn"),
        sf::Vector2f{1150.f, std::round(yPos + 80.f)}, true
    );
    slot -> btnLoad -> setOnClick([this, index](){ onLoadClick(index); });

    slot -> btnDelete = std::make_unique<UI::Button>
    (
        ResourceManager::getInstance().getTexture("savedgame_delete_btn"),
        sf::Vector2f{1250.f, std::round(yPos + 80.f)}, true
    );
    slot -> btnDelete -> setOnClick([this, index](){ onDeleteClick(index); });

    m_slots.push_back(std::move(slot));
}

void SavedGame::updateScroll(float percent)
{
    m_scrollOffset = percent * m_maxScroll;
}

void SavedGame::handleEvent(sf::Event& event)
{
    if (m_showPopup)
    {
        m_popupYesBtn.handleEvent(event, m_window);
        m_popupNoBtn.handleEvent(event, m_window);
        return;
    }

    m_backBtn.handleEvent(event, m_window);
    m_scrollbar -> handleEvent(event, m_window);

    if (event.type == sf::Event::MouseWheelScrolled)
    {
        float currentVal = m_scrollbar->getValue();
        float delta = -event.mouseWheelScroll.delta * 0.1f;
        m_scrollbar -> setValue(currentVal + delta);
    }

    for (auto& slot : m_slots)
    {
        float basePosY = slot->panel.getPosition().y;

        slot -> btnLoad -> setPosition({1150.f, std::round(basePosY + 80.f - m_scrollOffset)});
        slot -> btnDelete -> setPosition({1250.f, std::round(basePosY + 80.f - m_scrollOffset)});

        slot -> btnLoad -> handleEvent(event, m_window);
        slot -> btnDelete -> handleEvent(event, m_window);
    }
}

GameStateType SavedGame::update(float deltaTime)
{
    if (m_showPopup)
    {
        m_popupYesBtn.update(m_window);
        m_popupNoBtn.update(m_window);
        return GameStateType::NoChange;
    }

    m_backBtn.update(m_window);
    m_scrollbar -> update(m_window);

    for (auto& slot : m_slots)
    {
        slot->btnLoad->update(m_window);
        slot->btnDelete->update(m_window);
    }

    GameStateType state = m_requestedState;
    m_requestedState = GameStateType::NoChange;
    return state;
}

void SavedGame::draw()
{
    m_window.draw(m_background);

    for (auto& slot : m_slots)
    {
        float yDisplay = slot->panel.getPosition().y - m_scrollOffset;

        if (yDisplay > -200 && yDisplay < m_window.getSize().y)
        {
            slot -> panel.setPosition(300.f, yDisplay);
            m_window.draw(slot -> panel);

            sf::Vector2f thumbPos = slot -> thumbnail.getPosition();
            slot -> thumbnail.setPosition(310.f, std::round(yDisplay + 10.f));
            m_window.draw(slot -> thumbnail);
            slot -> thumbnail.setPosition(thumbPos);

            slot -> textTitle.setPosition(470.f, std::round(yDisplay + 15.f));
            m_window.draw(slot -> textTitle);

            slot -> textDetail.setPosition(470.f, std::round(yDisplay + 55.f));
            m_window.draw(slot -> textDetail);

            slot -> textStatus.setPosition(470.f, std::round(yDisplay + 85.f));
            m_window.draw(slot -> textStatus);

            slot -> btnLoad -> setPosition({1150.f, std::round(yDisplay + 80.f)});
            slot -> btnDelete -> setPosition({1250.f, std::round(yDisplay + 80.f)});

            slot -> btnLoad -> draw(m_window);
            slot -> btnDelete -> draw(m_window);

            slot -> panel.setPosition(300.f, std::round(yDisplay + m_scrollOffset));
        }
    }

    m_scrollbar -> draw(m_window);
    m_backBtn.draw(m_window);

    if (m_showPopup)
    {
        m_window.draw(m_popupOverlay);
        m_window.draw(m_popupBox);
        m_window.draw(m_popupText);
        m_popupYesBtn.draw(m_window);
        m_popupNoBtn.draw(m_window);
    }
}

void SavedGame::onLoadClick(int index)
{
    m_selectedFileToLoad = m_slots[index]->info.filename;
    m_requestedState = GameStateType::SavedGame;
}

void SavedGame::onDeleteClick(int index)
{
    m_targetDeleteIndex = index;
    m_showPopup = true;
}

void SavedGame::onBackClick()
{
    m_requestedState = GameStateType::MainMenu;
}

std::string SavedGame::getFileToLoad() const
{
    return m_selectedFileToLoad;
}
