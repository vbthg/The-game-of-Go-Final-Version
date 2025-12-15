#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include "IBot.h"
#include "PachiBot.h"
#include "MiniMaxBot.h"
#include "GlobalSetting.h"

class BotManager
{
public:
    static BotManager& getInstance()
    {
        static BotManager instance;
        return instance;
    }

    void startBackgroundPachi(int boardSize)
    {
        if(m_backgroundPachi)
        {
            m_backgroundPachi->sendCommand("clear_board");
            m_backgroundPachi->setBoardSize(boardSize);
            m_backgroundPachi->sendCommand("boardsize " + std::to_string(boardSize));
            m_backgroundPachi->sendCommand("time_settings 0 1 1");
            return;
        }

        m_backgroundPachi = std::make_shared<PachiBot>(boardSize, 3);

        std::thread t([this]()
        {
            if(m_backgroundPachi)
            {
                m_backgroundPachi->init();
                m_backgroundPachi->sendCommand("time_settings 0 1 1");
            }
        });
        t.detach();
    }

    void startBot(AiDifficulty difficulty, int boardSize)
    {
        m_isReady = false;
        m_currentBot.reset();

        if(difficulty == AiDifficulty::Hard || difficulty == AiDifficulty::Medium)
        {
            if(!m_backgroundPachi)
            {
                startBackgroundPachi(boardSize);
            }

            m_currentBot = m_backgroundPachi;

            if (m_backgroundPachi)
            {
                if (difficulty == AiDifficulty::Hard)
                {
                    m_backgroundPachi->sendCommand("time_settings 0 8 1");
                }
                else
                {
                    m_backgroundPachi->sendCommand("time_settings 0 4 1");
                }
            }

            m_isReady = true;
        }
        else
        {
            int depth = 1;
            m_currentBot = std::make_shared<MiniMaxBot>(boardSize, depth);

            std::thread t([this]()
            {
                if(m_currentBot)
                {
                    m_currentBot->init();
                    m_isReady = true;
                }
            });
            t.detach();
        }
    }

    std::shared_ptr<IBot> getBot()
    {
        return m_currentBot;
    }

    std::shared_ptr<PachiBot> getBackgroundBot()
    {
        return m_backgroundPachi;
    }

    bool isReady() const
    {
        return m_isReady;
    }

private:
    BotManager() {}

    std::shared_ptr<IBot> m_currentBot;
    std::shared_ptr<PachiBot> m_backgroundPachi;
    std::atomic<bool> m_isReady{false};
};
