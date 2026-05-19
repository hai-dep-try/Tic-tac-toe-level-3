#include "property_test.h"
#include "logic.h"
#include <random>
#include <sstream>

namespace core::test {

/**
 * Sinh một bàn cờ ngẫu nhiên
 */
static Board generateRandomBoard(int size, Rng& rng) {
    Board board = initBoard(size);
    std::uniform_int_distribution<int> cellDist(0, 2); // 0: EMPTY, 1: X, 2: O
    
    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            int val = cellDist(rng);
            if (val == 1) {
                board.grid[r][c] = SYMBOL_X;
            } else if (val == 2) {
                board.grid[r][c] = SYMBOL_O;
            }
        }
    }
    return board;
}

/**
 * Sinh nước đi ngẫu nhiên (có thể hợp lệ hoặc không hợp lệ)
 */
static Move generateRandomMove(int size, Rng& rng) {
    // Cho phép sinh nước đi ngoài phạm vi để kiểm thử độ bền vững (robustness)
    std::uniform_int_distribution<int> posDist(-2, size + 2);
    return Move{posDist(rng), posDist(rng)};
}

bool runPropertyTests(Logger* logger, Rng& rng) {
    if (logger) logger->log("[Property Test] Starting property-based testing for pure functions...");

    int testCases = 1000;
    int successCount = 0;

    std::uniform_int_distribution<int> sizeDist(BOARD_N_MIN, BOARD_N_MAX);
    std::uniform_int_distribution<int> symDist(0, 1);

    for (int i = 0; i < testCases; ++i) {
        int size = sizeDist(rng);
        Board board = generateRandomBoard(size, rng);
        Move move = generateRandomMove(size, rng);
        char symbol = (symDist(rng) == 0) ? SYMBOL_X : SYMBOL_O;

        try {
            // 1. Kiểm tra Đặc tính 1: isValidMove == false => applyMove không đổi board
            bool valid = isValidMove(board, move);
            if (!valid) {
                Board nextBoard = applyMove(board, move, symbol);
                if (!(nextBoard == board)) {
                    std::ostringstream err;
                    err << "Property 1 violated: applyMove mutated board for invalid move ("
                        << move.row << "," << move.col << ")";
                    throw PropertyTestFailure(i, size, move, symbol, err.str());
                }
            }

            // 2. Kiểm tra Đặc tính 2: isValidMove == true => countSymbol tăng thêm 1
            if (valid) {
                int countBefore = countSymbol(board, symbol);
                Board nextBoard = applyMove(board, move, symbol);
                int countAfter = countSymbol(nextBoard, symbol);
                if (countAfter != countBefore + 1) {
                    std::ostringstream err;
                    err << "Property 2 violated: countSymbol changed by " << (countAfter - countBefore)
                        << " (expected +1) after applying valid move ("
                        << move.row << "," << move.col << ")";
                    throw PropertyTestFailure(i, size, move, symbol, err.str());
                }
            }

            successCount++;
        } catch (const PropertyTestFailure& e) {
            if (logger) {
                std::ostringstream errLog;
                errLog << "[Property Test] FAILURE at test case " << e.testCase
                       << " (size=" << e.boardSize << ", move=(" << e.move.row << "," << e.move.col
                       << "), symbol=" << e.symbol << "): " << e.detail;
                logger->log(errLog.str(), Logger::Level::ERROR);
            }
            return false;
        }
    }

    if (logger) {
        std::ostringstream ss;
        ss << "[Property Test] Success: Passed " << successCount << "/" << testCases 
           << " randomized test cases. All mathematical invariants hold!";
        logger->log(ss.str(), Logger::Level::INFO);
        logger->log("[Grader/Property tests pass!]", Logger::Level::INFO);
    }
    return true;
}

} // namespace core::test
