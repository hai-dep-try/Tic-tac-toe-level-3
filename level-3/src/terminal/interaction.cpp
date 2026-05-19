/**
 * Terminal Interaction cpp implementation
 *
 * Mo ta:
 *   Cai dat interaction cho terminal. Dung cin de doc input.
 */

#include "interaction.h"

/* ---------- Importing ---------- */

#include <iostream>
#include <string>
#include <sstream>
#include <limits>

/* ---------- Definitions ---------- */

TerminalInteraction::TerminalInteraction() : I_Interaction() {}
TerminalInteraction::~TerminalInteraction() = default;

void TerminalInteraction::init(const RunConfig& /*config*/) {
    // Terminal interaction khong can init dac biet
}

void TerminalInteraction::pause(int /*timeout*/) {
    std::cout.flush();
    std::string dummy;
    if (!std::getline(std::cin, dummy)) {
        throw QuitException();
    }
}

bool TerminalInteraction::selectSize(int* size) {
    std::string input;
    if (!std::getline(std::cin, input)) {
        throw QuitException();
    }

    try {
        int val = std::stoi(input);
        if (val >= BOARD_N_MIN && val <= BOARD_N_MAX) {
            *size = val;
            return true;
        }
    } catch (...) {}
    return false;
}

bool TerminalInteraction::selectGoal(int* goal, int size) {
    std::string input;
    if (!std::getline(std::cin, input)) {
        throw QuitException();
    }

    try {
        int val = std::stoi(input);
        int maxGoal = std::min(size, GOAL_MAX);
        if (val >= 3 && val <= maxGoal) {
            *goal = val;
            return true;
        }
    } catch (...) {}
    return false;
}

bool TerminalInteraction::selectGameMode(GameMode* mode) {
    std::string input;
    if (!std::getline(std::cin, input)) {
        throw QuitException();
    }

    try {
        int val = std::stoi(input);
        switch (val) {
            case 1: *mode = GameMode::PVP; return true;
            case 2: *mode = GameMode::PVE; return true;
            case 3: *mode = GameMode::EVE; return true;
            default: break;
        }
    } catch (...) {}
    return false;
}

bool TerminalInteraction::selectBotLevel(BotLevel* levels, int index) {
    std::string input;
    if (!std::getline(std::cin, input)) {
        throw QuitException();
    }

    try {
        int val = std::stoi(input);
        switch (val) {
            case 1: levels[index] = BotLevel::EASY;   return true;
            case 2: levels[index] = BotLevel::MEDIUM;  return true;
            case 3: levels[index] = BotLevel::HARD;    return true;
            default: break;
        }
    } catch (...) {}
    return false;
}

bool TerminalInteraction::getPlayerMove(int* row, int* col) {
    std::cout << "Enter row and col (e.g. 1 2) or 'q' to quit: ";
    std::cout.flush();

    std::string input;
    if (!std::getline(std::cin, input)) {
        throw QuitException();
    }

    // Check for quit
    if (input == "q" || input == "Q") {
        throw QuitException();
    }

    // Parse "row col"
    std::istringstream iss(input);
    int r, c;
    if (iss >> r >> c) {
        *row = r;
        *col = c;
        return true;
    }

    return false;
}

void TerminalInteraction::close() {
    // no-op
}
