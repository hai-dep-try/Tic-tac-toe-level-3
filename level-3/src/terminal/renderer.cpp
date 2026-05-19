/**
 * Terminal Renderer cpp implementation
 *
 * Mo ta:
 *   Cai dat renderer cho terminal. Dung cout, ANSI escape codes.
 */

#include "renderer.h"

/* ---------- Importing ---------- */

#include <iostream>
#include <iomanip>
#include <sstream>

/* ---------- ANSI Colors ---------- */

namespace {

const std::string RESET   = "\033[0m";
const std::string RED     = "\033[31m";
const std::string GREEN   = "\033[32m";
const std::string YELLOW  = "\033[33m";
const std::string BLUE    = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN    = "\033[36m";
const std::string BOLD    = "\033[1m";
const std::string DIM     = "\033[2m";

std::string colorize(char cell) {
    if (cell == SYMBOL_X) return BOLD + BLUE + cell + RESET;
    if (cell == SYMBOL_O) return BOLD + RED + cell + RESET;
    return DIM + std::string(1, cell) + RESET;
}

}  // namespace

/* ---------- Definitions ---------- */

TerminalRenderer::TerminalRenderer() : I_Renderer() {}
TerminalRenderer::~TerminalRenderer() = default;

void TerminalRenderer::init(const RunConfig& /*config*/) {
    // Terminal renderer khong can init dac biet
}

void TerminalRenderer::clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif
    std::cout.flush();
}

void TerminalRenderer::showSelectMenu(SelectType selectType, int context) {
    switch (selectType) {
        case SelectType::TITLE_UI:
            std::cout << BOLD << CYAN;
            std::cout << "========================================\n";
            std::cout << "       TIC-TAC-TOE / GOMOKU\n";
            std::cout << "       Level 3 - Functional Programming\n";
            std::cout << "========================================\n";
            std::cout << RESET;
            std::cout << "Press ENTER to continue...\n";
            break;

        case SelectType::SIZE_UI:
            std::cout << BOLD << "\n--- SELECT BOARD SIZE ---\n" << RESET;
            std::cout << "Enter board size (" << BOARD_N_MIN << " - " << BOARD_N_MAX << "): ";
            break;

        case SelectType::GOAL_UI:
            std::cout << BOLD << "\n--- SELECT GOAL ---\n" << RESET;
            std::cout << "Enter goal (3 - " << std::min(context, GOAL_MAX) << "): ";
            break;

        case SelectType::GAME_MODE_UI:
            std::cout << BOLD << "\n--- SELECT GAME MODE ---\n" << RESET;
            std::cout << "1. PVP (Player vs Player)\n";
            std::cout << "2. PVE (Player vs Bot)\n";
            std::cout << "3. EVE (Bot vs Bot)\n";
            std::cout << "Enter choice (1-3): ";
            break;

        case SelectType::BOT_LEVEL_UI:
            std::cout << BOLD << "\n--- SELECT BOT LEVEL ---\n" << RESET;
            std::cout << "1. EASY\n";
            std::cout << "2. MEDIUM\n";
            std::cout << "3. HARD\n";
            std::cout << "Enter choice (1-3): ";
            break;

        case SelectType::PLAYER_UI:
            std::cout << BOLD << "\n--- CHOOSE SIDE ---\n" << RESET;
            std::cout << "1. Play as X (go first)\n";
            std::cout << "2. Play as O (go second)\n";
            std::cout << "Enter choice (1-2): ";
            break;

        case SelectType::MUL_BOT_LEVEL_UI:
            std::cout << BOLD << "\n--- SELECT BOT " << (context + 1) << " LEVEL ---\n" << RESET;
            std::cout << "1. EASY\n";
            std::cout << "2. MEDIUM\n";
            std::cout << "3. HARD\n";
            std::cout << "Enter choice (1-3): ";
            break;

        default:
            break;
    }
    std::cout.flush();
}

void TerminalRenderer::showInvalidSelect(SelectType selectType, int /*context*/) {
    std::cout << RED << "Invalid selection! Please try again.\n" << RESET;
    std::cout.flush();
}

void TerminalRenderer::showValidSelect(SelectType selectType, int context) {
    switch (selectType) {
        case SelectType::SIZE_UI:
            std::cout << GREEN << "Board size: " << context << "x" << context << RESET << "\n";
            break;
        case SelectType::GOAL_UI:
            std::cout << GREEN << "Goal: " << context << RESET << "\n";
            break;
        case SelectType::GAME_MODE_UI:
            std::cout << GREEN << "Mode: " << modeToString(context) << RESET << "\n";
            break;
        case SelectType::BOT_LEVEL_UI:
        case SelectType::MUL_BOT_LEVEL_UI:
            std::cout << GREEN << "Bot level: " << botToString(context) << RESET << "\n";
            break;
        default:
            break;
    }
    std::cout.flush();
}

void TerminalRenderer::displayBoard(const Board& board) {
    std::cout << "\n";

    // Header: column numbers
    std::cout << "    ";
    for (int c = 0; c < board.size; ++c) {
        std::cout << std::setw(3) << c;
    }
    std::cout << "\n";

    std::cout << "   +";
    for (int c = 0; c < board.size; ++c) {
        std::cout << "---";
    }
    std::cout << "+\n";

    // Rows
    for (int r = 0; r < board.size; ++r) {
        std::cout << std::setw(2) << r << " |";
        for (int c = 0; c < board.size; ++c) {
            std::cout << " " << colorize(board.at(r, c)) << " ";
        }
        std::cout << "|\n";
    }

    std::cout << "   +";
    for (int c = 0; c < board.size; ++c) {
        std::cout << "---";
    }
    std::cout << "+\n\n";
    std::cout.flush();
}

void TerminalRenderer::showMove(int row, int col) {
    std::cout << CYAN << "Move: (" << row << ", " << col << ")" << RESET << "\n";
    std::cout.flush();
}

void TerminalRenderer::showInvalidMove() {
    std::cout << RED << "Invalid move! Cell is occupied or out of range." << RESET << "\n";
    std::cout.flush();
}

void TerminalRenderer::showPlayer(int player, bool is_bot) {
    char symbol = (player == 0) ? SYMBOL_X : SYMBOL_O;
    std::string color = (player == 0) ? BLUE : RED;
    std::string type = is_bot ? " [BOT]" : "";

    std::cout << BOLD << color << "Player " << symbol << type << "'s turn" << RESET << "\n";
    std::cout.flush();
}

void TerminalRenderer::showResult(int winner, bool is_bot, const WinLine* winLine) {
    std::cout << "\n" << BOLD;

    if (winner == DRAW_RESULT) {
        std::cout << YELLOW << "=== DRAW! ===" << RESET << "\n";
    } else {
        char symbol = (winner == 0) ? SYMBOL_X : SYMBOL_O;
        std::string color = (winner == 0) ? BLUE : RED;
        std::string type = is_bot ? " (BOT)" : "";

        std::cout << color << "=== PLAYER " << symbol << type << " WINS! ===" << RESET << "\n";

        // Highlight win line
        if (winLine && !winLine->cells.empty()) {
            std::cout << GREEN << "Winning line: ";
            for (size_t i = 0; i < winLine->cells.size(); ++i) {
                if (i > 0) std::cout << " -> ";
                std::cout << "(" << winLine->cells[i].first << "," << winLine->cells[i].second << ")";
            }
            std::cout << RESET << "\n";
        }
    }

    std::cout.flush();
}

void TerminalRenderer::printResult(const GameResult& gameResult) {
    std::cout << "\n" << BOLD << "=== GAME OVER ===" << RESET << "\n";

    if (gameResult.winner == DRAW_RESULT) {
        std::cout << YELLOW << "Result: DRAW" << RESET << "\n";
    } else {
        char symbol = (gameResult.winner == 0) ? SYMBOL_X : SYMBOL_O;
        std::string color = (gameResult.winner == 0) ? BLUE : RED;
        std::string type = gameResult.isBot ? " (BOT)" : "";
        std::cout << color << "Winner: Player " << symbol << type << RESET << "\n";
    }

    std::cout << "Total turns: " << gameResult.turns << "\n";
    std::cout.flush();
}

void TerminalRenderer::close() {
    // no-op
}
