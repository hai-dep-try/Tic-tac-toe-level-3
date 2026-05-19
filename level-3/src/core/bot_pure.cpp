/**
 * Bot (FP-style) implementation
 */

#include "bot_pure.h"
#include "logic.h"
#include "parallel.h"
#include "pipeline.h"
#include <algorithm>
#include <numeric>
#include <random>

namespace bot {

/* ---------- Easy ---------- */

/**
 * Mô tả: Bot Easy -- random pick.
 * Lấy các nước đi hợp lệ, chọn ngẫu nhiên 1 ô.
 */
Move easy(const GameState& state, Rng& rng) {
    auto moves = core::enumerateValidMoves(state.board);
    if (moves.empty()) return INVALID_MOVE;
    std::uniform_int_distribution<int> dist(0, (int)moves.size() - 1);
    return moves[dist(rng)];
}

/* ---------- Medium ---------- */

/**
 * Mô tả: Bot Medium -- heuristic 3 bước:
 *   1. Thắng ngay nếu có thể.
 *   2. Chặn đối thủ nếu đối thủ sắp thắng.
 *   3. Chọn ngẫu nhiên (easy).
 */
Move medium(const GameState& state, int goal, Rng& rng) {
    auto moves = core::enumerateValidMoves(state.board);
    if (moves.empty()) return INVALID_MOVE;

    char mySymbol = core::symbolOf(state.currentPlayer);
    char oppSymbol = core::opponentOf(mySymbol);

    // FP-style: Currying & closures để định nghĩa các bộ lọc nước đi
    auto isWinningMove = [&](Move m, char symbol) {
        Board nextBoard = core::applyMove(state.board, m, symbol);
        return core::checkWin(nextBoard, symbol, goal);
    };

    // Bước 1: Lọc nước đi giúp mình thắng
    auto winningMoves = fp::pipe(
        fp::constant(moves),
        fp::filter([&](Move m) { return isWinningMove(m, mySymbol); })
    )(moves);

    if (!winningMoves.empty()) return winningMoves[0];

    // Bước 2: Lọc nước đi giúp chặn đối thủ thắng
    auto blockingMoves = fp::pipe(
        fp::constant(moves),
        fp::filter([&](Move m) { return isWinningMove(m, oppSymbol); })
    )(moves);

    if (!blockingMoves.empty()) return blockingMoves[0];

    // Bước 3: Ngẫu nhiên
    return easy(state, rng);
}

/* ---------- Hard ---------- */

/**
 * Heuristic đánh giá điểm của 1 ô cụ thể cho 1 symbol (tấn công hoặc phòng thủ)
 */
static int evaluateCellDirection(const Board& board, int r, int c, int dr, int dc, char symbol, int goal) {
    int count = 0;
    int empty = 0;
    int opp = 0;

    // Xem xét cửa sổ kích thước goal quanh ô (r, c)
    // Để đơn giản và nhanh, ta quét goal ô tiếp diễn từ ô này
    for (int i = 0; i < goal; ++i) {
        int nr = r + i * dr;
        int nc = c + i * dc;
        if (board.inRange(nr, nc)) {
            char cell = board.at(nr, nc);
            if (cell == symbol) {
                count++;
            } else if (cell == EMPTY_CELL) {
                empty++;
            } else {
                opp++;
            }
        } else {
            opp++;
        }
    }

    // Nếu có quân đối thủ trong cửa sổ này thì cửa sổ không thể tạo thành hàng thắng
    if (opp > 0) return 0;

    // Chấm điểm dựa trên số quân của mình đã có
    if (count == goal) return 10000;
    if (count == goal - 1 && empty == 1) return 1000;
    if (count == goal - 2 && empty == 2) return 100;
    if (count == goal - 3 && empty == 3) return 10;
    return count;
}

/**
 * Đánh giá tổng điểm của 1 nước đi (pure)
 */
static int evaluateMoveScore(const GameState& state, Move move, char symbol, int goal) {
    char oppSymbol = core::opponentOf(symbol);

    // Thử đặt quân của mình xem có thắng luôn không
    Board myBoard = core::applyMove(state.board, move, symbol);
    if (core::checkWin(myBoard, symbol, goal)) {
        return 1000000; // Thắng ngay lập tức
    }

    // Thử đặt quân của đối thủ xem đối thủ có thắng luôn không (cần chặn gấp)
    Board oppBoard = core::applyMove(state.board, move, oppSymbol);
    if (core::checkWin(oppBoard, oppSymbol, goal)) {
        return 500000; // Chặn đối thủ thắng
    }

    // Tính điểm heuristic bình thường trên bàn cờ sau khi đi nước này
    int score = 0;
    static constexpr int dr[] = {0, 1, 1, 1};
    static constexpr int dc[] = {1, 0, 1, -1};

    // Duyệt qua mọi cửa sổ chứa nước đi này
    for (int r = 0; r < myBoard.size; ++r) {
        for (int c = 0; c < myBoard.size; ++c) {
            for (int d = 0; d < 4; ++d) {
                // Tấn công
                score += evaluateCellDirection(myBoard, r, c, dr[d], dc[d], symbol, goal);
                // Phòng thủ (chặn tiềm năng)
                score += evaluateCellDirection(oppBoard, r, c, dr[d], dc[d], oppSymbol, goal) / 2;
            }
        }
    }

    return score;
}

/**
 * Mô tả: Bot Hard -- heuristic mạnh dựa trên điểm tích lũy các cửa sổ thắng.
 * Dùng std::accumulate (fold) thay vì mutation bestScore/bestMove.
 */
Move hard(const GameState& state, int goal, Rng& rng) {
    auto moves = core::enumerateValidMoves(state.board);
    if (moves.empty()) return INVALID_MOVE;

    char mySymbol = core::symbolOf(state.currentPlayer);

    // Accumulator: {bestMove, bestScore}
    auto result = std::accumulate(
        moves.begin(),
        moves.end(),
        std::pair<Move, int>{INVALID_MOVE, -1},
        [&](std::pair<Move, int> acc, Move m) {
            int score = evaluateMoveScore(state, m, mySymbol, goal);
            if (score > acc.second) {
                return std::pair<Move, int>{m, score};
            } else if (score == acc.second) {
                std::uniform_int_distribution<int> dist(0, 1);
                if (dist(rng) == 1 || acc.first == INVALID_MOVE) {
                    return std::pair<Move, int>{m, score};
                }
            }
            return acc;
        }
    );

    return result.first != INVALID_MOVE ? result.first : moves[0];
}

/**
 * Mô tả: Bot Hard -- phiên bản song song (Mức 3 nhánh B).
 * Dùng core::par::evaluateAllMoves_par để đánh giá đa luồng,
 * sau đó reduce ra bestMove (deterministic, không random tie-break).
 */
Move hardParallel(const GameState& state, int goal, Rng& rng, int num_threads) {
    auto moves = core::enumerateValidMoves(state.board);
    if (moves.empty()) return INVALID_MOVE;

    char mySymbol = core::symbolOf(state.currentPlayer);

    // Đánh giá song song
    auto evaluated = core::par::evaluateAllMoves_par(state, mySymbol, goal, num_threads);

    // Reduce: tìm bestMove (deterministic: tie-break chọn nước đầu tiên)
    auto result = std::accumulate(
        evaluated.begin(),
        evaluated.end(),
        std::pair<Move, int>{INVALID_MOVE, -1},
        [&](std::pair<Move, int> acc, const std::pair<Move, int>& entry) {
            if (entry.second > acc.second) {
                return entry;
            }
            return acc;
        }
    );

    (void)rng;  // parallel mode không dùng RNG (deterministic)
    return result.first != INVALID_MOVE ? result.first : moves[0];
}

/* ---------- Factory ---------- */

/**
 * Mô tả: Trả về BotFn ứng với level.
 */
BotFn makeBot(BotLevel level, int goal) {
    switch (level) {
        case BotLevel::EASY:
            return [](const GameState& s, Rng& r) { return easy(s, r); };
        case BotLevel::MEDIUM:
            return [goal](const GameState& s, Rng& r) { return medium(s, goal, r); };
        case BotLevel::HARD:
            return [goal](const GameState& s, Rng& r) { return hard(s, goal, r); };
        default:
            return [](const GameState&, Rng&) { return INVALID_MOVE; };
    }
}

/**
 * Mô tả: Overload của makeBot hỗ trợ parallel mode (Mức 3 nhánh B).
 */
BotFn makeBot(BotLevel level, int goal, bool parallel, int num_threads) {
    if (parallel && level == BotLevel::HARD) {
        return [goal, num_threads](const GameState& s, Rng& r) {
            return hardParallel(s, goal, r, num_threads);
        };
    }
    // Fallback: dùng phiên bản thường
    return makeBot(level, goal);
}

}  // namespace bot
