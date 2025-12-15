#pragma once
#include <string>
#include <vector>
#include <SFML/System/Vector2.hpp>

struct BotMove
{
    int x;
    int y;
    bool isPass = false;
    bool isResign = false;
};

class IBot
{
public:
    virtual ~IBot() {}

    virtual void init() = 0;

    virtual void syncMove(std::string color, int x, int y) = 0;

    virtual BotMove generateMove(bool isBlackTurn) = 0;

    virtual std::vector<sf::Vector2i> getDeadStones() = 0;

    virtual void setBoardSize(int size) = 0;
};
