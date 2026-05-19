/**
 * Logger header file (Level 3 -- instance-based, không global)
 *
 * Mô tả:
 *   Khác với level 2, Logger ở level 3 KHÔNG còn là global state.
 *   Nó là một class/đối tượng được TẠO ra ở `main.cpp` rồi INJECT
 *   xuống Engine, Bot, helper… qua constructor hoặc tham số.
 *
 *   Đây là điều kiện cần để các hàm trong tầng `core/` thực sự "pure":
 *   pure function không được phụ thuộc biến toàn cục, không được log.
 *   Mọi side-effect log đều xảy ra ở tầng `shell/`.
 *
 * Yêu cầu mức 1, task 3:
 *   Sinh viên cần loại bỏ mọi `Logger::log(...)` global trong code,
 *   thay bằng `logger.log(...)` với `logger` là tham chiếu được inject.
 */

#pragma once

/* ---------- Importing ---------- */

#include <fstream>
#include <string>

/* ---------- Declarations ---------- */

class Logger {
   public:
    /**
     * Mô tả: Mức độ log.
     */
    enum class Level {
        DEBUG,    // log chi tiết
        INFO,     // thông tin chung
        WARNING,  // cảnh báo
        ERROR,    // lỗi nghiêm trọng
        MSG,      // message thuần (không prefix)
    };

    /**
     * Mô tả: Constructor -- không-op, gọi `init()` để cấu hình thật.
     */
    Logger();

    /**
     * Mô tả: Destructor -- đóng file log nếu còn mở.
     */
    ~Logger();

    // Logger không được copy (chứa file stream + cấu hình).
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * Mô tả: Khởi tạo logger.
     *
     * Đầu vào:
     *   - judge_mode: chế độ chấm (im lặng, chỉ in result)
     *   - to_file: có ghi log ra file hay không
     *   - path: đường dẫn file log
     *   - verbose_flag: bật DEBUG level
     */
    void init(bool judge_mode, bool to_file = true,
              const std::string& path = "log.txt", bool verbose_flag = false);

    /**
     * Mô tả: Ghi một dòng log.
     */
    void log(const std::string& msg, Level level = Level::INFO);

    /**
     * Mô tả: Đóng file log (idempotent).
     */
    void close();

    /**
     * Mô tả: Chuyển Level sang string.
     */
    static std::string levelToString(Level level);

   private:
    Level min_level_ = Level::INFO;  // mức log tối thiểu hiển thị
    bool write_to_file_ = false;     // có ghi ra file không
    bool is_judge_mode_ = false;     // chế độ chấm
    std::ofstream log_file_;         // file stream
};
