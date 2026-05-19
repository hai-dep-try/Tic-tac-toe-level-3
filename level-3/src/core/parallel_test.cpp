#include "parallel_test.h"
#include "parallel.h"
#include "logic.h"
#include <chrono>
#include <numeric>
#include <random>
#include <sstream>

namespace core::test {

/**
 * Sinh board ngẫu nhiên với mật độ fill khoảng 30-50%
 */
static Board generateRandomBoard(int size, Rng& rng) {
    Board board = initBoard(size);
    std::uniform_int_distribution<int> cellDist(0, 9);
    std::uniform_int_distribution<int> symDist(0, 1);

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            if (cellDist(rng) < 4) {  // ~40% filled
                board.grid[r][c] = (symDist(rng) == 0) ? SYMBOL_X : SYMBOL_O;
            }
        }
    }
    return board;
}

/**
 * Kiểm tra equality: seq == par cho cùng input
 */
bool runParallelEqualityTest(Logger* logger, Rng& rng, int numTests) {
    if (logger) {
        std::ostringstream ss;
        ss << "[Parallel Test] Starting equality test: seq vs par ("
           << numTests << " random boards)...";
        logger->log(ss.str(), Logger::Level::INFO);
    }

    std::uniform_int_distribution<int> sizeDist(BOARD_N_MIN, BOARD_N_MAX);
    std::uniform_int_distribution<int> playerDist(0, 1);
    std::uniform_int_distribution<int> goalDist(3, 5);

    for (int i = 0; i < numTests; ++i) {
        int size = sizeDist(rng);
        Board board = generateRandomBoard(size, rng);
        int currentPlayer = playerDist(rng);
        int goal = std::min(goalDist(rng), size);

        GameState state;
        state.board = board;
        state.currentPlayer = currentPlayer;
        state.turn = 0;
        state.winner = -1;
        state.isFinished = false;

        char symbol = core::symbolOf(currentPlayer);

        auto seqResult = par::evaluateAllMoves_seq(state, symbol, goal);
        auto parResult = par::evaluateAllMoves_par(state, symbol, goal);

        // So sánh: cùng số phần tử, cùng thứ tự, cùng score
        if (seqResult.size() != parResult.size()) {
            if (logger) {
                std::ostringstream err;
                err << "[Parallel Test] FAILURE at test " << i
                    << " (size=" << size << "): seq has " << seqResult.size()
                    << " moves, par has " << parResult.size();
                logger->log(err.str(), Logger::Level::ERROR);
            }
            return false;
        }

        for (size_t j = 0; j < seqResult.size(); ++j) {
            if (seqResult[j].first != parResult[j].first ||
                seqResult[j].second != parResult[j].second) {
                if (logger) {
                    std::ostringstream err;
                    err << "[Parallel Test] FAILURE at test " << i
                        << " (size=" << size << ", move " << j
                        << "): seq={" << seqResult[j].first.row << ","
                        << seqResult[j].first.col << "," << seqResult[j].second
                        << "}, par={" << parResult[j].first.row << ","
                        << parResult[j].first.col << "," << parResult[j].second
                        << "}";
                    logger->log(err.str(), Logger::Level::ERROR);
                }
                return false;
            }
        }
    }

    if (logger) {
        std::ostringstream ss;
        ss << "[Parallel Test] Equality: Passed " << numTests << "/" << numTests
           << " boards. seq == par confirmed!";
        logger->log(ss.str(), Logger::Level::INFO);
    }
    return true;
}

/**
 * Benchmark: đo thời gian seq vs par trên 3 cấu hình
 */
std::vector<ParallelBenchmarkResult> runParallelBenchmark(
    Logger* logger, Rng& rng, int runsPerConfig, int numThreads) {

    std::vector<ParallelBenchmarkResult> results;
    std::vector<int> sizes = {5, 8, 12};

    for (int size : sizes) {
        if (logger) {
            std::ostringstream ss;
            ss << "[Parallel Benchmark] Testing " << size << "x" << size
               << " board (" << runsPerConfig << " runs)...";
            logger->log(ss.str(), Logger::Level::INFO);
        }

        // Tạo board mẫu (đủ dense để có ý nghĩa benchmark)
        Board board = generateRandomBoard(size, rng);
        int goal = std::min(3 + (size / 4), size);

        GameState state;
        state.board = board;
        state.currentPlayer = 0;
        state.turn = 0;
        state.winner = -1;
        state.isFinished = false;

        char symbol = core::symbolOf(0);

        std::vector<double> seqTimes;
        std::vector<double> parTimes;
        seqTimes.reserve(runsPerConfig);
        parTimes.reserve(runsPerConfig);

        for (int run = 0; run < runsPerConfig; ++run) {
            // Sequential
            auto t0 = std::chrono::high_resolution_clock::now();
            auto seqResult = par::evaluateAllMoves_seq(state, symbol, goal);
            auto t1 = std::chrono::high_resolution_clock::now();
            double seqMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
            seqTimes.push_back(seqMs);

            // Parallel
            auto t2 = std::chrono::high_resolution_clock::now();
            auto parResult = par::evaluateAllMoves_par(state, symbol, goal, numThreads);
            auto t3 = std::chrono::high_resolution_clock::now();
            double parMs = std::chrono::duration<double, std::milli>(t3 - t2).count();
            parTimes.push_back(parMs);

            // Verify equality mỗi lần
            if (seqResult != parResult) {
                if (logger) {
                    std::ostringstream err;
                    err << "[Parallel Benchmark] MISMATCH at " << size << "x" << size
                        << " run " << run;
                    logger->log(err.str(), Logger::Level::ERROR);
                }
            }
        }

        // Tính trung bình
        double avgSeq = std::accumulate(seqTimes.begin(), seqTimes.end(), 0.0) / runsPerConfig;
        double avgPar = std::accumulate(parTimes.begin(), parTimes.end(), 0.0) / runsPerConfig;
        double speedup = (avgPar > 0.001) ? (avgSeq / avgPar) : 0.0;

        ParallelBenchmarkResult result;
        result.boardSize = size;
        result.seqMs = avgSeq;
        result.parMs = avgPar;
        result.speedup = speedup;
        results.push_back(result);

        if (logger) {
            std::ostringstream ss;
            ss << "  " << size << "x" << size << " | seq: " << std::fixed
               << std::setprecision(3) << avgSeq << "ms | par: " << avgPar
               << "ms | speedup: " << speedup << "x";
            logger->log(ss.str(), Logger::Level::INFO);
        }
    }

    return results;
}

} // namespace core::test
