/**
 * Logger cpp implementation
 */

#include "logger.h"

/* ---------- Importing ---------- */

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

/* ---------- Constants ---------- */

namespace {

const std::string RESET  = "\033[0m";
const std::string RED    = "\033[31m";
const std::string GREEN  = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE   = "\033[34m";

std::string getColor(Logger::Level level) {
    switch (level) {
        case Logger::Level::DEBUG:   return BLUE;
        case Logger::Level::INFO:    return GREEN;
        case Logger::Level::WARNING: return YELLOW;
        case Logger::Level::ERROR:   return RED;
        default:                     return RESET;
    }
}

std::string nowString() {
    auto t  = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(t);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%H:%M:%S");
    return os.str();
}

}  // namespace

/* ---------- Definitions ---------- */

Logger::Logger() = default;

Logger::~Logger() {
    close();
}

void Logger::init(bool judge_mode, bool to_file, const std::string& path, bool verbose_flag) {
    is_judge_mode_ = judge_mode;
    write_to_file_ = to_file && !judge_mode;
    min_level_     = verbose_flag ? Level::DEBUG : Level::INFO;

    if (write_to_file_) {
        log_file_.open(path, std::ios::out | std::ios::trunc);
    }
}

void Logger::log(const std::string& msg, Level level) {
    // Judge mode: tuyệt đối im lặng (output chỉ phục vụ grader).
    if (is_judge_mode_) return;

    // Lọc theo min_level.
    if (static_cast<int>(level) < static_cast<int>(min_level_)) return;

    const std::string label = levelToString(level);
    const std::string time  = nowString();

    // In ra console (có màu).
    if (level == Level::MSG) {
        std::cout << msg << "\n";
    } else {
        std::cout << "[" << time << "] "
                  << getColor(level) << "[" << label << "]" << RESET
                  << " " << msg << "\n";
    }

    // Ghi ra file (không có màu).
    if (write_to_file_ && log_file_.is_open()) {
        if (level == Level::MSG) {
            log_file_ << msg << "\n";
        } else {
            log_file_ << "[" << time << "] [" << label << "] " << msg << "\n";
        }
        log_file_.flush();
    }
}

void Logger::close() {
    if (log_file_.is_open()) {
        log_file_.close();
    }
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARN";
        case Level::ERROR:   return "ERROR";
        case Level::MSG:     return "";
        default:             return "UNKNOWN";
    }
}
