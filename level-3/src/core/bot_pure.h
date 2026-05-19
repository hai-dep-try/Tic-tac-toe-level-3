/**
 * Bot (FP-style) header file
 *
 * Mô tả:
 *   Trong level 2, Bot là một CLASS hierarchy:
 *       Bot (abstract) ← BotLevel1 ← BotLevel2 ← BotLevel3
 *   với polymorphism qua virtual `getMove(...)`.
 *
 *   Trong level 3, Bot được mô hình hoá theo phong cách FP:
 *       BotFn = (GameState, Rng&) → Move
 *   tức "Bot là một HÀM". Polymorphism được thay bằng higher-order: ta
 *   chuyển bot giữa các module bằng cách truyền `BotFn` (giống truyền
 *   `comparator` cho `std::sort`).
 *
 *   Ưu điểm so với class hierarchy:
 *     - Không cần inheritance, không có vtable.
 *     - Dễ tổ hợp: `pipe(filter, map, reduce)` ra một bot mới.
 *     - Dễ test: bot là pure function (trừ phần dùng RNG).
 *
 * Ánh xạ tới level 2:
 *     BotLevel1::getMove(...)        ↔  bot::easy(state, rng)
 *     BotLevel2::getMove(...)        ↔  bot::medium(state, rng)
 *     BotLevel3::getMove(...)        ↔  bot::hard(state, rng)
 *     BotFactory::createBot(level)   ↔  bot::makeBot(level, goal)
 */

#pragma once

/* ---------- Importing ---------- */

#include <functional>

#include "../shell/rng.h"
#include "types.h"

namespace bot {

/* ---------- Type Definitions ---------- */

/**
 * Mô tả:
 *   Một bot là một hàm: (GameState, Rng&) → Move.
 *   - GameState chứa board, player hiện tại, ...
 *   - Rng&     là dependency được INJECT (không global).
 *
 * NOTE:
 *   Đây là điểm mấu chốt của mức 1 task 2: thay vì truy cập
 *   `generator` global, hàm bot nhận `Rng&` từ ngoài.
 */
using BotFn = std::function<Move(const GameState&, Rng&)>;

/**
 * Mô tả:
 *   Bot symbol-aware: nhiều bot cần biết mình đang chơi 'X' hay 'O'.
 *   Ta dùng partial application (curry) để cố định symbol & goal:
 *       BotFn easyForX = withSymbol(easyRaw, 'X', goal);
 *   Điều này thể hiện "currying" rất tự nhiên trong code.
 */
using SymbolicBot = std::function<Move(const GameState&, char symbol, int goal, Rng&)>;

/* ---------- Bot implementations ---------- */

/**
 * Mô tả:
 *   Bot Easy -- chọn ngẫu nhiên một ô trống.
 *
 * Yêu cầu mức 1 -- task 2 (Higher-order function cho Bot LV1):
 *   Cài đặt theo pipeline FP:
 *       enumerateCells(board)
 *           | filter(isValidMove(board, _))    // lọc ô trống
 *           | reduce(pickRandom)               // hoặc sample
 *
 *   Có thể dùng `fp::pipe(...)` từ pipeline.h, hoặc `std::ranges`,
 *   hoặc đệ quy. Quan trọng: KHÔNG dùng for-loop tích luỹ kiểu mệnh lệnh.
 *
 * Đầu vào:
 *   - state: trạng thái game.
 *   - rng  : nguồn random (KHÔNG dùng global).
 *
 * Đầu ra: Move; trả về INVALID_MOVE nếu không còn ô trống.
 */
Move easy(const GameState& state, Rng& rng);

/**
 * Mô tả:
 *   Bot Medium -- heuristic đơn giản:
 *     1. nếu thắng được trong 1 nước → đánh.
 *     2. nếu đối thủ thắng trong 1 nước → chặn.
 *     3. còn lại → easy().
 *
 * Yêu cầu mức 2 (Bot -- sinh viên tự chọn áp dụng FP vào bot nào):
 *   Khuyến khích cài bằng `pipe(...)` để pipeline rõ ràng:
 *       moves = enumerateValidMoves(board);
 *       winning = moves | filter(λm. isWinningMove(state, m));
 *       blocking = moves | filter(λm. isBlockingMove(state, m));
 *       choose = pipe(winning, ifEmpty(blocking), ifEmpty(easy));
 *
 *   Có thể dùng currying: `auto isWinningFor = curry(isWinningMove)(state);`.
 */
Move medium(const GameState& state, int goal, Rng& rng);

/**
 * Mô tả:
 *   Bot Hard -- minimax / alpha-beta / heuristic mạnh.
 *
 * Yêu cầu mức 2 (nâng cao) hoặc mức 3 nhánh B:
 *   - Mức 2: cài bằng FP với composition/currying.
 *   - Mức 3 nhánh B: tách phần `evaluateAllMoves` thành pure function
 *     để có thể chạy song song (xem core/parallel.h).
 *
 *   Yêu cầu KHÔNG mutate board ở bất kỳ bước nào.
 *
 * NOTE: nếu chưa hoàn thiện, sinh viên có thể fallback `medium`.
 */
Move hard(const GameState& state, int goal, Rng& rng);

/**
 * Mô tả:
 *   Bot Hard -- phiên bản song song (Mức 3 nhánh B).
 *   Dùng core::par::evaluateAllMoves_par để đánh giá nước đi đa luồng.
 *
 *   Yêu cầu:
 *     - num_threads <= 0 → tự động dùng hardware_concurrency.
 *     - Kết quả phải deterministic (cùng state → cùng bestMove).
 */
Move hardParallel(const GameState& state, int goal, Rng& rng, int num_threads = 0);

/* ---------- Factory ---------- */

/**
 * Mô tả:
 *   Trả về một BotFn theo level. Đây là phiên bản FP của `BotFactory`
 *   ở level 2 -- không trả về con trỏ heap, mà trả về một hàm
 *   (đóng gói symbol, goal, ...).
 *
 * Ví dụ dùng:
 *   BotFn botX = bot::makeBot(BotLevel::HARD, goal);
 *   Move m = botX(state, rng);
 *
 * TODO (mức 1 -- task 2):
 *   - Switch theo level → return một lambda gọi easy/medium/hard.
 *   - Nếu level invalid → return một lambda trả về INVALID_MOVE
 *     (KHÔNG throw -- pure function).
 */
BotFn makeBot(BotLevel level, int goal);

/**
 * Mô tả:
 *   Overload của makeBot hỗ trợ parallel mode (Mức 3 nhánh B).
 *   Khi parallel=true và level=HARD, trả về bot dùng evaluateAllMoves_par.
 *
 * Ví dụ dùng:
 *   BotFn botX = bot::makeBot(BotLevel::HARD, goal, true, 4);
 */
BotFn makeBot(BotLevel level, int goal, bool parallel, int num_threads = 0);

}  // namespace bot
