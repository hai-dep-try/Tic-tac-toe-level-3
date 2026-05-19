/**
 * Core Logic implementation (PURE)
 *
 * Mô tả:
 *   Các stub TODO cho sinh viên mức 1 -- yêu cầu task 1 ("FP-hoá Logic::*").
 *
 * Quy tắc khi cài đặt:
 *   - KHÔNG include <iostream>, KHÔNG dùng Logger (đây là tầng pure).
 *   - KHÔNG dùng biến toàn cục, KHÔNG random ở đây.
 *   - Trả về giá trị mới thay vì sửa tham số.
 *   - Khuyến khích std::ranges (C++20) và đệ quy.
 *
 * NOTE:
 *   File này không include "../shell/logger.h" hay tương tự -- nếu cần log,
 *   phải log ở tầng `shell/` SAU KHI gọi pure function.
 */

#include "logic.h"
#include <algorithm>
#include <ranges>
#include <numeric>
#include "pipeline.h"

namespace core {

/* ---------- Construction ---------- */

/**
 * Mô tả: Tạo board rỗng.
 */
Board initBoard(int size) {
    Board b;
    b.size = size;
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            b.grid[r][c] = EMPTY_CELL;
        }
    }
    return b;
}

/* ---------- Queries ---------- */

/**
 * Mô tả: Nước đi có hợp lệ?
 */
bool isValidMove(const Board& board, Move move) {
    return board.inRange(move.row, move.col) && board.at(move.row, move.col) == EMPTY_CELL;
}

/**
 * Mô tả: Liệt kê tất cả các Move (r, c) trên board.
 * Sử dụng đệ quy đuôi (Tail recursion) để bộc lộ FP như yêu cầu.
 */
std::vector<Move> enumerateCells(const Board& board) {
    auto helper = [&](auto self, int r, int c, std::vector<Move> acc) -> std::vector<Move> {
        if (r == board.size) return acc;
        acc.push_back(Move{r, c});
        if (c + 1 < board.size) {
            return self(self, r, c + 1, std::move(acc));
        } else {
            return self(self, r + 1, 0, std::move(acc));
        }
    };
    return helper(helper, 0, 0, {});
}

/**
 * Mô tả: Liệt kê các nước đi hợp lệ (= các ô trống).
 * Dùng pipeline fp::pipe và fp::filter theo yêu cầu Mức 1.
 */
std::vector<Move> enumerateValidMoves(const Board& board) {
    return fp::pipe(
        enumerateCells,
        fp::filter([&](Move m) { return isValidMove(board, m); })
    )(board);
}

/**
 * Mô tả: Ô (x, y) có phải đầu mở của chuỗi `symbol` không?
 */
bool isEmptyHead(const Board& board, int x, int y, char symbol) {
    if (!board.inRange(x, y) || board.at(x, y) != EMPTY_CELL) return false;
    static constexpr int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    static constexpr int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    for (int i = 0; i < 8; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (board.inRange(nx, ny) && board.at(nx, ny) == symbol) {
            return true;
        }
    }
    return false;
}

/* ---------- Transitions ---------- */

/**
 * Mô tả: Tạo Board MỚI sau khi đặt symbol vào ô move.
 */
Board applyMove(const Board& board, Move move, char symbol) {
    Board nextBoard = board;
    if (isValidMove(board, move)) {
        nextBoard.grid[move.row][move.col] = symbol;
    }
    return nextBoard;
}

/**
 * Mô tả: Tính GameState MỚI sau 1 lượt đi.
 */
GameState applyTurn(const GameState& state, Move move, int goal) {
    char symbol = symbolOf(state.currentPlayer);
    Board nextBoard = applyMove(state.board, move, symbol);

    GameState nextState;
    nextState.board = nextBoard;
    nextState.currentPlayer = 1 - state.currentPlayer;
    nextState.turn = state.turn + 1;

    if (checkWin(nextBoard, symbol, goal)) {
        nextState.winner = state.currentPlayer;
        nextState.isFinished = true;
    } else if (checkDraw(nextBoard)) {
        nextState.winner = DRAW_RESULT;
        nextState.isFinished = true;
    } else {
        nextState.winner = -1;
        nextState.isFinished = false;
    }
    return nextState;
}

/* ---------- End-of-game ---------- */

/**
 * Mô tả: Có người thắng?
 */
bool checkWin(const Board& board, char symbol, int goal, EndRule rule) {
    return getWinLine(board, symbol, goal, rule).has_value();
}

/**
 * Mô tả: Hoà?
 */
bool checkDraw(const Board& board) {
    return countSymbol(board, EMPTY_CELL) == 0;
}

/**
 * Mô tả: Lấy đường thắng (nếu có).
 */
std::optional<WinLine> getWinLine(const Board& board, char symbol, int goal, EndRule rule) {
    static constexpr int dr[] = {0, 1, 1, 1};
    static constexpr int dc[] = {1, 0, 1, -1};

    for (int r = 0; r < board.size; ++r) {
        for (int c = 0; c < board.size; ++c) {
            for (int d = 0; d < 4; ++d) {
                int curr_dr = dr[d];
                int curr_dc = dc[d];

                bool match = true;
                std::vector<pII> cells;
                cells.reserve(goal);
                for (int i = 0; i < goal; ++i) {
                    int nr = r + i * curr_dr;
                    int nc = c + i * curr_dc;
                    if (!board.inRange(nr, nc) || board.at(nr, nc) != symbol) {
                        match = false;
                        break;
                    }
                    cells.push_back({nr, nc});
                }

                if (match) {
                    int openCount = 0;
                    int pr = r - curr_dr;
                    int pc = c - curr_dc;
                    if (board.inRange(pr, pc) && board.at(pr, pc) == EMPTY_CELL) {
                        openCount++;
                    }
                    int nr = r + goal * curr_dr;
                    int nc = c + goal * curr_dc;
                    if (board.inRange(nr, nc) && board.at(nr, nc) == EMPTY_CELL) {
                        openCount++;
                    }

                    if (rule == EndRule::NONE ||
                        (rule == EndRule::OPEN_ONE && openCount >= 1) ||
                        (rule == EndRule::OPEN_TWO && openCount == 2)) {
                        return WinLine{cells};
                    }
                }
            }
        }
    }
    return std::nullopt;
}

/* ---------- Helpers ---------- */

/**
 * Mô tả: Đếm số ô = symbol.
 * Tối ưu: dùng vòng lặp thay vì pipeline để tránh cấp phát vector trung gian.
 * Vẫn là pure function — không mutate, không side-effect.
 */
int countSymbol(const Board& board, char symbol) {
    int count = 0;
    for (int r = 0; r < board.size; ++r) {
        for (int c = 0; c < board.size; ++c) {
            count += (board.grid[r][c] == symbol);
        }
    }
    return count;
}

}  // namespace core
