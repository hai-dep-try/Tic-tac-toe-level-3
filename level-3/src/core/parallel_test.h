#pragma once

#include "types.h"
#include "../shell/logger.h"
#include "../shell/rng.h"
#include <string>
#include <vector>

namespace core::test {

struct ParallelBenchmarkResult {
    int boardSize;
    double seqMs;
    double parMs;
    double speedup;
};

/**
 * Mô tả: Kiểm tra evaluateAllMoves_seq == evaluateAllMoves_par
 * trên >=100 board ngẫu nhiên. Trả về true nếu tất cả pass.
 */
bool runParallelEqualityTest(Logger* logger, Rng& rng, int numTests = 100);

/**
 * Mô tả: Benchmark seq vs par trên 3 cấu hình (5x5, 8x8, 12x12),
 * mỗi cấu hình chạy >=30 lần. Trả về danh sách kết quả.
 */
std::vector<ParallelBenchmarkResult> runParallelBenchmark(
    Logger* logger, Rng& rng, int runsPerConfig = 30, int numThreads = 0);

} // namespace core::test
