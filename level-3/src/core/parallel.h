/**
 * Parallel evaluation header (Mức 3 -- nhánh B)
 *
 * Mô tả:
 *   File này chỉ cần thiết nếu sinh viên chọn LÀM nhánh B (song song hoá).
 *   Nếu không chọn nhánh B, có thể bỏ qua hoặc xoá file này.
 *
 *   Ý tưởng: tận dụng tính chất "pure → no shared state → song song an toàn"
 *   để chạy đánh giá nhiều nước đi đồng thời, rồi reduce ra max.
 *
 * Tại sao FP làm việc này dễ hơn OOP?
 *   - Pure function không có race condition (không chia sẻ trạng thái).
 *   - Bất biến: input không bị sửa giữa chừng.
 *   - Hàm bậc cao (`std::for_each(par, ..., λ)`) là phong cách FP tự nhiên.
 *
 * Yêu cầu nhánh B:
 *   1. Cài đặt 2 phiên bản: `evaluateAllMoves_seq` và `evaluateAllMoves_par`.
 *   2. Chứng minh OUTPUT bằng nhau (test ≥100 board ngẫu nhiên).
 *   3. Benchmark: ≥30 lần × 3 cấu hình board (5×5, 8×8, 12×12).
 *   4. Báo cáo speedup + giải thích.
 */

#pragma once

/* ---------- Importing ---------- */

#include <utility>
#include <vector>

#include "types.h"

namespace core::par {

/**
 * Mô tả:
 *   Đánh giá điểm của một nước đi (pure).
 *   Sinh viên định nghĩa heuristic -- có thể tái dùng từ Bot Hard.
 *
 * TODO (nhánh B):
 *   - Pure: KHÔNG mutate state.
 *   - Có thể dùng minimax 1 tầng, alpha-beta, hoặc heuristic đơn giản.
 *   - Đảm bảo deterministic: cùng (state, move, symbol, goal) → cùng score.
 */
int evaluateMove(const GameState& state, Move move, char symbol, int goal);

/**
 * Mô tả:
 *   Đánh giá toàn bộ nước đi hợp lệ -- TUẦN TỰ.
 *
 * TODO (nhánh B):
 *   moves = enumerateValidMoves(board);
 *   return std::transform(seq, moves, evaluateMove);
 */
std::vector<std::pair<Move, int>>
evaluateAllMoves_seq(const GameState& state, char symbol, int goal);

/**
 * Mô tả:
 *   Đánh giá toàn bộ nước đi hợp lệ -- SONG SONG.
 *
 * TODO (nhánh B):
 *   - Cách 1: std::for_each(std::execution::par, ...) (cần <execution>).
 *   - Cách 2: std::async + std::future (mỗi nước 1 task).
 *   - Cách 3: thread pool tự xây.
 *
 *   Yêu cầu: KẾT QUẢ phải bằng `_seq` cho mọi input (test bằng `==`).
 */
std::vector<std::pair<Move, int>>
evaluateAllMoves_par(const GameState& state, char symbol, int goal,
                      int num_threads = 0);

}  // namespace core::par
