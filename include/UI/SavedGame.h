
#pragma once

#include "GameState.h"
#include "Button.h"
#include "Slider.h"
#include "ResourceManager.h"
#include "SaveDefinition.h"
#include <vector>
#include <memory>

struct SaveSlotUI
{
    SaveInfo info;
    sf::RectangleShape panel;
    sf::Sprite thumbnail;
    sf::Texture textureHolder;
    sf::Text textTitle;
    sf::Text textDetail;
    sf::Text textStatus;

    std::unique_ptr<UI::Button> btnLoad;
    std::unique_ptr<UI::Button> btnDelete;
};


class SavedGame : public GameState
{
public:
    SavedGame(sf::RenderWindow& window);
    virtual ~SavedGame() = default;

    virtual void handleEvent(sf::Event& event) override;
    virtual GameStateType update(float deltaTime) override;
    virtual void draw() override;

    std::string getFileToLoad() const;

private:
    void loadSaveFiles();
    void createSlotUI(int index, const SaveInfo& info, float yPos);
    void updateScroll(float percent);

    void onLoadClick(int index);
    void onDeleteClick(int index);
    void onBackClick();

    sf::RenderWindow& m_window;
    GameStateType m_requestedState;
    sf::Sprite m_background;
    UI::Button m_backBtn;

    std::vector<std::unique_ptr<SaveSlotUI>> m_slots;
    std::unique_ptr<UI::Slider> m_scrollbar;

    float m_scrollOffset;
    float m_maxScroll;
    float m_slotHeight;

    bool m_showPopup;
    int m_targetDeleteIndex;
    sf::RectangleShape m_popupOverlay;
    sf::RectangleShape m_popupBox;
    sf::Text m_popupText;
    UI::Button m_popupYesBtn;
    UI::Button m_popupNoBtn;

    std::string m_selectedFileToLoad;
};
