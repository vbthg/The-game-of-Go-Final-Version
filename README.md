# The Game of Go (C++ & SFML)

![Language](https://img.shields.io/badge/Language-C++17-blue.svg)
![Library](https://img.shields.io/badge/Library-SFML%202.6.1-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)
![Course](https://img.shields.io/badge/Course-Intro%20to%20Programming-orange.svg)

> **⚠️ Important Note regarding the GitHub Version vs. Submitted Version:**
>
> The submitted version is **fully functional and feature-complete**. However, a minor visual regression was introduced during the final optimization phase on the deadline. Due to a missing default initialization in the `VirtualConnectedComponent` struct (specifically the `anchorDistance` variable), the *territory wave animation* (visual effect) might be invisible in the submitted build.
>
> **Rest assured, the core logic and scoring mechanics remain accurate.** The source code hosted here on GitHub fixes this visual issue. To restore the animation in the submitted version, you can do one of the following:
> * **Option 1 (Recommended):** Copy the `include/ScoringOverlay.h` file from this repository and replace the one in your project.
> * **Option 2 (Manual Fix):** Open `ScoringOverlay.h` and initialize `anchorDistance` to a large value (e.g., `1000000000` or `1e9`) directly inside the struct definition.

> **Final Project - Introduction to Computer Science** > **University of Science, VNU-HCM (HCMUS)** > **Faculty of Information Technology**

## 📖 Overview

This project is a comprehensive digital implementation of the ancient board game **Go (Weiqi/Baduk)**. Built from scratch using **C++** and the **SFML** multimedia library, the application focuses on providing a polished user experience, robust game logic, and challenging AI opponents.

The game supports standard Go rules including Capture, Ko, Suicide restrictions, and Territory Scoring (Chinese Rules).

## 🎮 Features

### 🕹️ Game Modes
* **PvP (Player vs Player):** Local multiplayer on the same device with automatic turn management.
* **PvAI (Player vs Computer):**
    * **Easy (Novice):** Custom Minimax algorithm for beginners.
    * **Hard (Master):** Integration with the **Pachi Engine** (via Windows Pipes) for a high-level challenge.

### ⚙️ Core Mechanics
* **Complete Ruleset:** Handles complex interactions like *Ko* loops, *Suicide* moves, and group captures.
* **End Game Scoring:** Automatic territory calculation using **BFS (Breadth-First Search)** algorithms.
* **Utilities:** Unlimited **Undo/Redo**, **Hint System**, and **Resign** functionality.

### 🎨 User Interface (UI/UX)
* **Dynamic Animations:** Stone placement effects, territory visualization waves, and smooth screen transitions.
* **Custom UI Framework:** Buttons, Sliders, Toggle Switches, and Scrollable Lists built without external GUI libraries.
* **Themes:** Multiple board and stone skins (Classic, Dark, Cartoon, Realistic).
* **Audio:** Interactive sound effects and background music with volume control.

### 💾 System
* **Save/Load:** Save game state with a visual thumbnail preview and metadata.
* **Settings:** Configurable time limits, Komi, and audiovisual preferences.

## 📂 Project Structure

The project follows a modular architecture separating Logic, AI, and UI.

```text
GoGame/
├── bin/                 # Executables and DLLs
│   ├── GoGame.exe
│   ├── pachi/           # External AI Engine
│   │   ├── pachi.exe
│   │   └── patterns.spat
│   └── *.dll            # SFML & MinGW DLLs
├── src/                 # Source files (.cpp)
├── include/             # Header files (.h)
├── resources/           # Static Assets
│   ├── fonts/
│   ├── images/
│   └── sounds/
├── assets/              # Runtime Data (Saves, Settings)
└── README.md
