/**
 * Parallel evaluation implementation (Mức 3 -- nhánh B)
 *
 * Mô tả:
 *   File này CHỈ phải hoàn thành nếu sinh viên chọn nhánh B.
 *   Sinh viên không chọn nhánh B có thể giữ stub này -- Engine không gọi
 *   tới nó trừ khi flag `--parallel` được bật ở command-line.
 */

#include "parallel.h"

/* ---------- Importing ---------- */

#include <algorithm>
#include <future>
#include <numeric>
#include <thread>

#include "logic.h"

namespace core::par {

/* ---------- Heuristic helpers (pure, deterministic) ---------- */

/**
 * Đánh giá điểm của 1 hướng (dr, dc) từ ô (r, c) cho symbol.
 * Tái dùng logic từ bot_pure.cpp::evaluateCellDirection.
 */
static int evaluateCellDirection(const Board& board, int r, int c,
                                  int dr, int dc, char symbol, int goal) {
    int count = 0;
    int empty = 0;
    int opp  = 0;

    for (int i = 0; i < goal; ++i) {
        int nr = r + i * dr;
        int nc = c + i * dc;
        if (board.inRange(nr, nc)) {
            char cell = board.at(nr, nc);
            if (cell == symbol)      count++;
            else if (cell == EMPTY_CELL) empty++;
            else                     opp++;
        } else {
            opp++;
        }
    }

    if (opp > 0) return 0;

    if (count == goal)                return 10000;
    if (count == goal - 1 && empty == 1) return 1000;
    if (count == goal - 2 && empty == 2) return 100;
    if (count == goal - 3 && empty == 3) return 10;
    return count;
}

/**
 * Pure heuristic: đánh giá điểm của 1 nước đi.
 * - Thắng ngay → +1000000
 * - Đối thủ thắng nếu ta không chặn → +500000
 * - Bình thường → tổng điểm tấn công + phòng thủ trên mọi cửa sổ
 */
int evaluateMove(const GameState& state, Move move, char symbol, int goal) {
    char oppSymbol = core::opponentOf(symbol);

    // Thử đặt quân của mình xem có thắng luôn không
    Board myBoard = core::applyMove(state.board, move, symbol);
    if (core::checkWin(myBoard, symbol, goal)) {
        return 1000000;
    }

    // Thử đặt quân của đối thủ xem đối thủ có thắng luôn không
    Board oppBoard = core::applyMove(state.board, move, oppSymbol);
    if (core::checkWin(oppBoard, oppSymbol, goal)) {
        return 500000;
    }

    // Heuristic bình thường
    static constexpr int dr[] = {0, 1, 1, 1};
    static constexpr int dc[] = {1, 0, 1, -1};

    int score = 0;
    for (int r = 0; r < myBoard.size; ++r) {
        for (int c = 0; c < myBoard.size; ++c) {
            for (int d = 0; d < 4; ++d) {
                score += evaluateCellDirection(myBoard, r, c, dr[d], dc[d], symbol, goal);
                score += evaluateCellDirection(oppBoard, r, c, dr[d], dc[d], oppSymbol, goal) / 2;
            }
        }
    }

    return score;
}

/* ---------- Sequential baseline ---------- */

std::vector<std::pair<Move, int>>
evaluateAllMoves_seq(const GameState& state, char symbol, int goal) {
    auto moves = core::enumerateValidMoves(state.board);
    std::vector<std::pair<Move, int>> result;
    result.reserve(moves.size());
    for (auto m : moves) {
        result.emplace_back(m, evaluateMove(state, m, symbol, goal));
    }
    return result;
}

/* ---------- Parallel evaluation (std::async) ---------- */

std::vector<std::pair<Move, int>>
evaluateAllMoves_par(const GameState& state, char symbol, int goal,
                       int num_threads) {
    auto moves = core::enumerateValidMoves(state.board);
    if (moves.empty()) return {};

    int n = static_cast<int>(moves.size());
    int threads = (num_threads <= 0)
                      ? static_cast<int>(std::thread::hardware_concurrency())
                      : num_threads;
    if (threads <= 0) threads = 1;
    if (threads > n)  threads = n;

    // Chia moves thành chunks, mỗi chunk 1 task async
    int chunkSize = (n + threads - 1) / threads;

    std::vector<std::future<std::vector<std::pair<Move, int>>>> futures;
    futures.reserve(threads);

    for (int t = 0; t < threads; ++t) {
        int start = t * chunkSize;
        int end   = std::min(start + chunkSize, n);
        if (start >= n) break;

        futures.push_back(std::async(std::launch::async,
            [&, start, end]() {
                std::vector<std::pair<Move, int>> chunk;
                chunk.reserve(end - start);
                for (int i = start; i < end; ++i) {
                    chunk.emplace_back(moves[i],
                        evaluateMove(state, moves[i], symbol, goal));
                }
                return chunk;
            }));
    }

    // Gather results theo đúng thứ tự
    std::vector<std::pair<Move, int>> result;
    result.reserve(n);
    for (auto& f : futures) {
        auto chunk = f.get();
        result.insert(result.end(),
                       std::make_move_iterator(chunk.begin()),
                       std::make_move_iterator(chunk.end()));
    }

    return result;
}

}  // namespace core::par
