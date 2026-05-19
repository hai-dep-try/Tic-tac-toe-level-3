/**
 * I Renderer header file
 *
 * Mô tả:
 *   Interface cho renderer (UI layer). Phần này KHÔNG đổi nhiều so với
 *   level 2 -- chỉ thay tham số `char board[][BOARD_N_MAX]` thành `const Board&`
 *   để thống nhất với kiểu immutable ở core.
 */

#pragma once

/* ---------- Importing ---------- */

#include "../core/types.h"
#include "../utils/config.h"

/* ---------- Declarations ---------- */

class I_Renderer {
   public:
    I_Renderer();
    virtual ~I_Renderer();

    virtual void init(const RunConfig& config) = 0;

    virtual void clearScreen() = 0;

    virtual void showSelectMenu(SelectType selectType, int context = NO_CONTEXT) = 0;
    virtual void showInvalidSelect(SelectType selectType, int context = NO_CONTEXT) = 0;
    virtual void showValidSelect(SelectType selectType, int context = NO_CONTEXT) = 0;

    /**
     * Mô tả: Hiển thị bàn cờ.
     * KHÁC LV2: nhận `const Board&` (immutable) thay vì `char[][]` (mutable).
     */
    virtual void displayBoard(const Board& board) = 0;

    virtual void showMove(int row, int col) = 0;
    virtual void showInvalidMove() = 0;
    virtual void showPlayer(int player, bool is_bot) = 0;

    virtual void showResult(int winner, bool is_bot, const WinLine* winLine = nullptr) = 0;
    virtual void printResult(const GameResult& gameResult) = 0;

    virtual void close() = 0;
};
