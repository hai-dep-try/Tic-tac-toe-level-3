/**
 * RNG header file (Level 3 -- instance-based)
 *
 * Mô tả:
 *   Khác với level 2 (`inline std::mt19937 generator(SEED)` global),
 *   level 3 quan niệm RNG là một dependency được TRUYỀN VÀO mỗi
 *   pure function cần ngẫu nhiên (ví dụ Bot Easy random pick).
 *
 *   Tại sao? Vì truy cập biến toàn cục là một dạng "side-effect ngầm":
 *   cùng một input của hàm có thể cho output khác nhau ở 2 lần gọi
 *   (do RNG thay đổi state). FP yêu cầu: cùng input → cùng output.
 *
 * Cách dùng (gợi ý):
 *   - Trong hàm pure: nhận `Rng& rng` làm tham số cuối cùng.
 *   - Khi cần "thuần" hoàn toàn: tách thành `pickRandom(seed, list)`
 *     trả về `pair<value, newSeed>` (state monad đơn giản).
 *
 * NOTE:
 *   Trong khuôn khổ bài tập, dùng `Rng&` là đủ -- đây là một
 *   "controlled side-effect" rõ ràng (sinh viên thấy được nó từ signature).
 */

#pragma once

/* ---------- Importing ---------- */

#include <cstdint>
#include <random>

/* ---------- Type Definitions ---------- */

using Rng = std::mt19937;

inline constexpr std::uint32_t DEFAULT_SEED = 8702;

/**
 * Mô tả: Tạo một RNG với seed cho trước. Đây là "factory" -- pure.
 */
inline Rng makeRng(std::uint32_t seed = DEFAULT_SEED) {
    return Rng(seed);
}
