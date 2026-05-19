#pragma once

#include "types.h"
#include "../shell/logger.h"
#include "../shell/rng.h"
#include <string>
#include <stdexcept>

namespace core::test {

struct PropertyTestFailure : public std::runtime_error {
    int testCase;
    int boardSize;
    Move move;
    char symbol;
    std::string detail;

    PropertyTestFailure(int tc, int sz, Move m, char sym, const std::string& d)
        : std::runtime_error("[Property Test Failed] " + d),
          testCase(tc), boardSize(sz), move(m), symbol(sym), detail(d) {}
};

/**
 * Mô tả: Thực thi Property-based tests cho các pure functions.
 * Sinh ngẫu nhiên 1000 cấu hình bàn cờ để chứng thực Đặc tính 1 và 2.
 * Trả về true nếu tất cả pass, false nếu có lỗi.
 */
bool runPropertyTests(Logger* logger, Rng& rng);

} // namespace core::test
