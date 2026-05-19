/**
 * Config header file
 *
 * Mô tả:
 *   Giữ tinh thần như level 2 -- RunConfig là container dữ liệu cấu hình
 *   parse từ command-line. Khác biệt: thêm flag `--seed` và `--threads`
 *   để hỗ trợ FP/parallel ở mức 3.
 */

#pragma once

/* ---------- Importing ---------- */

#include <cstdint>
#include <string>

/* ---------- Global Variables ---------- */

// version của chương trình
// TODO: sửa <STUDENT_ID> bằng mã số sinh viên
inline const std::string VERSION = "0.5.<STUDENT_ID>";

/* ---------- Type Definitions ---------- */

struct RunConfig {
    // ---------- Core ----------
    bool interactive = true;   // bật/tắt UI tương tác
    bool judge_mode  = false;  // chế độ chấm
    std::string input_file;    // file input cho judge mode

    bool to_file = true;
    std::string log_file = "log.txt";

    bool verbose_flag = false;  // bật DEBUG log
    bool gui_flag     = false;  // bật SDL (nếu có; level 3 không bắt buộc)

    bool is_help = false;

    // ---------- Random ----------
    std::uint32_t seed = 8702;  // seed cho RNG (truyền vào pure functions)

    // ---------- Parallel (Mức 3 nhánh B) ----------
    bool parallel_flag = false;  // bật chạy bot Hard song song
    int  num_threads   = 0;       // 0 = mặc định (tuỳ thư viện chuẩn)

    // ---------- SDL Layout ----------
    int screenWidth  = 800;
    int screenHeight = 800;
    int boardPadding = 100;

    // ---------- Helper ----------
    friend std::ostream& operator<<(std::ostream& os, const RunConfig& config);
    std::string toString() const;
};

/* ---------- Declarations ---------- */

/**
 * Mô tả: Parse argv → RunConfig.
 */
RunConfig parseArgs(int argc, char* argv[]);

/**
 * Mô tả: Trả về chuỗi help.
 */
std::string configHelpStr();
