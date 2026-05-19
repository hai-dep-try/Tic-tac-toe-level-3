/**
 * Core Logic header file (PURE)
 *
 * Mô tả:
 *   Toàn bộ luật chơi Tic-tac-toe / Gomoku, viết theo phong cách FP:
 *     - Mọi hàm đều PURE: không log, không IO, không random, không global.
 *     - Mọi hàm KHÔNG mutate Board -- đầu vào là `const Board&`,
 *       nếu cần "thay đổi" thì TRẢ VỀ Board mới.
 *     - Không sử dụng exception cho luồng bình thường (chỉ dùng
 *       `NotImplementedException` ở stub).
 *
 * Đối chiếu với level 2 (`game/logic.{h,cpp}`):
 *   level 2:   void Logic::makeMove(char board[][N], int r, int c, char s);
 *   level 3:   Board core::applyMove(const Board&, Move, char symbol);
 *              ^ trả về board MỚI, board cũ không bị sửa.
 *
 * Yêu cầu mức 1 -- task 1 (FP-hoá Logic::*):
 *   Sinh viên hoàn thiện các stub bên dưới, đảm bảo:
 *     (a) Hàm pure: cùng input → cùng output, không side-effect.
 *     (b) Trả về giá trị mới thay vì sửa tham số.
 *     (c) Ít nhất 1 hàm dùng `std::ranges` / pipeline filter→map→reduce.
 *     (d) Ít nhất 1 hàm dùng đệ quy (ưu tiên đệ quy đuôi) thay vì for-loop.
 */

#pragma once

/* ---------- Importing ---------- */

#include <optional>
#include <vector>

#include "types.h"

namespace core {

/* ---------- Construction ---------- */

/**
 * Mô tả: Tạo board rỗng kích thước `size × size`.
 *
 * Đầu vào: size -- kích thước (BOARD_N_MIN ≤ size ≤ BOARD_N_MAX).
 * Đầu ra : Board mới, mọi ô = EMPTY_CELL.
 */
Board initBoard(int size);

/* ---------- Queries (pure, không sửa board) ---------- */

/**
 * Mô tả: Một nước đi có hợp lệ trên board không?
 *   - Trong phạm vi
 *   - Ô đang trống
 *
 * Đầu ra: true nếu hợp lệ.
 */
bool isValidMove(const Board& board, Move move);

/**
 * Mô tả:
 *   Liệt kê tất cả ô (Move) trên board (không lọc).
 *   Hữu ích làm "nguồn" cho pipeline filter → map → reduce.
 *
 * Yêu cầu mức 1d: nên cài bằng cách FP (ranges hoặc đệ quy).
 */
std::vector<Move> enumerateCells(const Board& board);

/**
 * Mô tả:
 *   Liệt kê các nước đi hợp lệ (ô trống) trên board.
 *   Gợi ý: dùng pipeline `enumerateCells | filter(isValidMove(board, _))`.
 */
std::vector<Move> enumerateValidMoves(const Board& board);

/**
 * Mô tả:
 *   Kiểm tra một ô có phải "đầu mở" hay không (dùng cho Gomoku).
 *   Đặc tả: ô trống, kề với chuỗi `symbol` đã có.
 */
bool isEmptyHead(const Board& board, int x, int y, char symbol);

/* ---------- Transitions (pure, trả về Board/State mới) ---------- */

/**
 * Mô tả:
 *   Tạo Board MỚI sau khi thực hiện `move` với `symbol`.
 *   Yêu cầu: KHÔNG sửa `board` đầu vào.
 *
 * Tiền điều kiện: isValidMove(board, move) == true. Nếu vi phạm,
 *   được phép trả về board không đổi (hoặc throw -- tuỳ thiết kế).
 *
 * Đây là viên gạch nền của FP game-loop ở mức 2.
 */
Board applyMove(const Board& board, Move move, char symbol);

/**
 * Mô tả:
 *   Cập nhật GameState (mức 2): tạo state mới sau 1 nước đi.
 *   Bao gồm: applyMove, đổi player, tăng turn, kiểm tra win/draw.
 *
 *   Đây là hàm "fold step" -- game loop sẽ fold qua chuỗi moves
 *   để ra GameState cuối cùng.
 */
GameState applyTurn(const GameState& state, Move move, int goal);

/* ---------- End-of-game checks ---------- */

/**
 * Mô tả: Có người thắng với `symbol` chưa?
 *
 * Yêu cầu mức 1: pure, nhận `const Board&`.
 */
bool checkWin(const Board& board, char symbol, int goal,
              EndRule rule = EndRule::NONE);

/**
 * Mô tả: Đã hết ô trống và chưa có người thắng → hoà.
 */
bool checkDraw(const Board& board);

/**
 * Mô tả: Lấy đường thắng (cells) nếu tồn tại; nullopt nếu không.
 */
std::optional<WinLine> getWinLine(const Board& board, char symbol, int goal,
                                   EndRule rule = EndRule::NONE);

/* ---------- Helpers cho mức 2 / mức 3 ---------- */

/**
 * Mô tả:
 *   Map player index (0 hoặc 1) sang symbol ('X' hoặc 'O'). Pure.
 *   lambda p. if p == 0 then 'X' else 'O'
 *
 * Sử dụng trong applyTurn để biết lượt hiện tại đặt quân gì:
 *   char sym = symbolOf(state.currentPlayer);
 */
constexpr char symbolOf(int player) noexcept {
    return player == 0 ? SYMBOL_X : SYMBOL_O;
}

/**
 * Mô tả:
 *   Trả về symbol đối thủ. Pure, trivially total.
 *   lambda s. if s == 'X' then 'O' else 'X'
 */
constexpr char opponentOf(char symbol) noexcept {
    return symbol == SYMBOL_X ? SYMBOL_O : SYMBOL_X;
}


/**
 * Mô tả:
 *   Đếm số ô của `symbol` trên board.
 *   Đây là một ứng cử viên TỐT cho:
 *     - chứng minh bằng quy nạp ở mức 3 nhánh A
 *       (ví dụ: countSymbol(applyMove(b,m,s)) = countSymbol(b) + 1
 *        khi isValidMove(b,m)).
 *     - viết bằng đệ quy đuôi.
 */
int countSymbol(const Board& board, char symbol);

}  // namespace core
