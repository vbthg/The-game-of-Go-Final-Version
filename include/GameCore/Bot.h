#pragma once
#include <windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <SFML/System/Vector2.hpp>
#include "IBot.h"

class Bot
{
private:
    HANDLE hChildStd_IN_Rd = NULL;
    HANDLE hChildStd_IN_Wr = NULL;
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;

    int boardSize = 19;
    const std::string BOARD_COLS = "ABCDEFGHJKLMNOPQRST";

    std::mutex m_mutex;

    std::string cleanResponse(std::string response)
    {
        size_t equalSign = response.find("=");
        if(equalSign != std::string::npos)
        {
            std::string clean = response.substr(equalSign + 1);
            while(!clean.empty() && (clean.back() == '\n' || clean.back() == '\r' || clean.back() == ' ')) clean.pop_back();
            while(!clean.empty() && (clean.front() == ' ' || clean.front() == '\r' || clean.front() == '\n')) clean.erase(0, 1);
            return clean;
        }
        return "";
    }

public:
    Bot() { }
    ~Bot() { close(); }

    bool peekPipe(DWORD* bytesAvail)
    {
        if(hChildStd_OUT_Rd == NULL) return false;
        return PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, bytesAvail, NULL);
    }

    bool readPipe(char* buffer, int size, DWORD* bytesRead)
    {
        if(hChildStd_OUT_Rd == NULL) return false;
        return ReadFile(hChildStd_OUT_Rd, buffer, size, bytesRead, NULL);
    }

    void flushPipe()
    {
        char chBuf[4096];
        DWORD dwRead, dwAvail;
        int safety = 50;
        while(safety-- > 0)
        {
            if(!peekPipe(&dwAvail)) break;
            if(dwAvail > 0)
            {
                ReadFile(hChildStd_OUT_Rd, chBuf, sizeof(chBuf), &dwRead, NULL);
            }
            else break;
        }
    }

    void setBoardSize(int size)
    {
        this->boardSize = size;
    }

    bool start(std::string exePath, int size = 19)
    {
        this->boardSize = size;
        SECURITY_ATTRIBUTES saAttr;
        saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
        saAttr.bInheritHandle = TRUE;
        saAttr.lpSecurityDescriptor = NULL;

        if(!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) return false;
        if(!SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) return false;

        if(!CreatePipe(&hChildStd_IN_Rd, &hChildStd_IN_Wr, &saAttr, 0)) return false;
        if(!SetHandleInformation(hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) return false;

        HANDLE hNullFile = CreateFileA("NUL", GENERIC_WRITE, FILE_SHARE_WRITE,
                                       &saAttr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);

        si.hStdInput = hChildStd_IN_Rd;
        si.hStdOutput = hChildStd_OUT_Wr;
        si.hStdError = hNullFile;

        si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        ZeroMemory(&pi, sizeof(pi));

        std::string cmdLine = "\"" + exePath + "\"";

        if(!CreateProcessA(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
        {
            CloseHandle(hNullFile);
            return false;
        }

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_IN_Rd);
        CloseHandle(hNullFile);

        return true;
    }

    std::string sendCommand(std::string cmd)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        DWORD dwWritten;
        std::string fullCmd = cmd + "\n";

//        std::cout << "[GTP >>] " << cmd << " (Sending...)" << "\n";

        if(!WriteFile(hChildStd_IN_Wr, fullCmd.c_str(), (DWORD)fullCmd.length(), &dwWritten, NULL))
        {
            std::cerr << "[Bot Error] WriteFile failed. Pipe broken?\n";
            return "";
        }

        std::string response = "";
        char chBuf[4096];
        DWORD dwRead, dwAvail;

        int maxRetries = 2000;
        while(maxRetries > 0)
        {
            if(hChildStd_OUT_Rd == NULL) break;

            if(!PeekNamedPipe(hChildStd_OUT_Rd, NULL, 0, NULL, &dwAvail, NULL))
            {
                break;
            }

            if(dwAvail > 0)
            {
                if(ReadFile(hChildStd_OUT_Rd, chBuf, sizeof(chBuf) - 1, &dwRead, NULL) && dwRead > 0)
                {
                    chBuf[dwRead] = '\0';
                    response += chBuf;

                    size_t eqPos = response.find('=');
                    if(eqPos != std::string::npos)
                    {
                        size_t nlAfterEq = response.find('\n', eqPos);
                        if(nlAfterEq != std::string::npos)
                        {
                            break;
                        }
                    }
                    else
                    {
                    }
                }
            }
            else
            {
                Sleep(10);
                maxRetries--;
            }
        }

        if(maxRetries == 0)
        {
//            std::cout << "[GTP <<] TIMEOUT! (No response in ~20s)\n";
            if(!response.empty())
            {
//                std::cout << "[GTP <<] Partial response (raw):\n" << response << "\n";
            }
        }
        else
        {
//            std::cout << "[GTP <<] Raw response:\n" << response << "\n";
        }

        return cleanResponse(response);
    }

    void close()
    {
        if(hChildStd_IN_Wr)
        {
            DWORD dwWritten;
            std::string s = "quit\n";
            WriteFile(hChildStd_IN_Wr, s.c_str(), (DWORD)s.length(), &dwWritten, NULL);
            CloseHandle(hChildStd_IN_Wr);
            CloseHandle(hChildStd_OUT_Rd);
            hChildStd_IN_Wr = NULL;
        }
    }

    std::string toGTP(int x, int y)
    {
        if(x < 0 || x >= boardSize || y < 0 || y >= boardSize) return "pass";
        char colChar = BOARD_COLS[x];
        int gtpRow = boardSize - y;
        return colChar + std::to_string(gtpRow);
    }

    sf::Vector2i fromGTP(std::string gtp)
    {
        if(gtp.length() < 2) return {-1, -1};
        std::string lower = gtp;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if(lower.find("pass") != std::string::npos) return {-1, -1};
        if(lower.find("resign") != std::string::npos) return {-2, -2};

        char c = std::toupper(gtp[0]);
        int x = -1;
        size_t pos = BOARD_COLS.find(c);
        if(pos != std::string::npos) x = (int)pos;

        int rowNum = 0;
        try
        {
            std::string numPart = gtp.substr(1);
            numPart.erase(std::remove_if(numPart.begin(), numPart.end(), [](unsigned char c)
            {
                return !std::isdigit(c);
            }), numPart.end());
            if(!numPart.empty()) rowNum = std::stoi(numPart);
        }
        catch(...)
        {
            return {-1, -1};
        }

        int y = boardSize - rowNum;
        if(x < 0 || x >= boardSize || y < 0 || y >= boardSize) return {-1, -1};
        return {x, y};
    }
};
