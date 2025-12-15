#pragma once
#include "IBot.h"
#include "Bot.h"
#include <string>
#include <sstream>

class PachiBot : public IBot
{
private:
    Bot pachiEngine;
    int boardSize;
    int difficultyLevel; // 1=Easy, 2=Medium, 3=Hard

public:
    PachiBot(int size, int difficulty) : boardSize(size), difficultyLevel(difficulty)
    { }

    std::string sendCommand(std::string cmd)
    {
        return pachiEngine.sendCommand(cmd);
    }

    std::string toGTP(int x, int y)
    {
        return pachiEngine.toGTP(x, y);
    }

    void setBoardSize(int size) override
    {
        this->boardSize = size;
        pachiEngine.setBoardSize(size);
    }

    void init() override
    {
        std::string pachiPath = "assets/pachi/pachi.exe";
        if (!pachiEngine.start(pachiPath, boardSize))
        {
            std::cerr << "[PachiBot] ERROR: Failed to start exe.\n";
            return;
        }

        std::cout << "[PachiBot] Engine started. Waiting for startup logs...\n";

        char buffer[4096];
        DWORD read, avail;
        int quietCount = 0;

        for (int i = 0; i < 200; ++i)
        {
            if (!pachiEngine.peekPipe(&avail)) break;

            if (avail > 0)
            {
                if (pachiEngine.readPipe(buffer, sizeof(buffer) - 1, &read))
                {
                    buffer[read] = '\0';
                    std::string log(buffer);
                    std::cout << "[Engine Output] " << log;
                    quietCount = 0;
                }
            }
            else
            {
                quietCount++;
                Sleep(100);
            }

            if (quietCount >= 15)
            {
                std::cout << "[PachiBot] Engine is quiet (Ready).\n";
                break;
            }
        }

        std::cout << "[PachiBot] Init Finished Successfully.\n";

        std::string test = pachiEngine.sendCommand("name");
        std::cout << "BOT NAME = " << test << "\n";
    }

    void syncMove(std::string color, int x, int y)
    {
        std::string coord = pachiEngine.toGTP(x, y);
        std::string cmd = "play " + color + " " + coord;

        std::string response = sendCommand(cmd);

//        std::cout << "[PachiBot] Sync: " << cmd << " -> Done." << std::endl;
    }

    BotMove generateMove(bool isBlackTurn) override
    {
        std::string turnColor = isBlackTurn ? "black" : "white";
        std::string response = pachiEngine.sendCommand("genmove " + turnColor);

        BotMove move;
        if(response == "pass")
        {
            move.isPass = true;
        }
        else if(response == "resign")
        {
            move.isResign = true;
        }
        else
        {
            sf::Vector2i p = pachiEngine.fromGTP(response);
            move.x = p.x;
            move.y = p.y;
        }
        return move;
    }


    std::vector<sf::Vector2i> getDeadStones() override
    {
        std::vector<sf::Vector2i> deadPoints;

        std::string response = pachiEngine.sendCommand("final_status_list dead");

        if (response.empty()) return deadPoints;

        std::stringstream ss(response);
        std::string segment;

        while (std::getline(ss, segment, ' '))
        {
            segment.erase(std::remove_if(segment.begin(), segment.end(),
                [](unsigned char c) { return std::isspace(c) && c != ' '; }),
                segment.end());

            if (segment.empty()) continue;

            sf::Vector2i p = pachiEngine.fromGTP(segment);

            if (p.x != -1 && p.y != -1)
            {
                deadPoints.push_back(p);
            }
        }

        return deadPoints;
    }
};
