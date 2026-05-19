/**
 * Terminal Renderer header file
 *
 * Mô tả:
 *   Cài đặt I_Renderer cho terminal. Đây là tầng "imperative shell"
 *   được phép side-effect (cout, clear screen, …).
 *
 *   So với level 2: chữ ký `displayBoard` đổi sang nhận `const Board&`.
 */

#pragma once

/* ---------- Importing ---------- */

#include "../interface/i_renderer.h"

/* ---------- Declarations ---------- */

class TerminalRenderer : public I_Renderer {
   public:
    TerminalRenderer();
    ~TerminalRenderer() override;

    void init(const RunConfig& config) override;
    void clearScreen() override;

    void showSelectMenu(SelectType selectType, int context = NO_CONTEXT) override;
    void showInvalidSelect(SelectType selectType, int context = NO_CONTEXT) override;
    void showValidSelect(SelectType selectType, int context = NO_CONTEXT) override;

    void displayBoard(const Board& board) override;

    void showMove(int row, int col) override;
    void showInvalidMove() override;
    void showPlayer(int player, bool is_bot) override;

    void showResult(int winner, bool is_bot, const WinLine* winLine = nullptr) override;
    void printResult(const GameResult& gameResult) override;

    void close() override;
};
