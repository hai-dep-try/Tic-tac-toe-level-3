/**
 * Config cpp implementation
 */

#include "config.h"

/* ---------- Importing ---------- */

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

/* ---------- Definitions ---------- */

/**
 * Mô tả: Parse argv → RunConfig.
 *   Hỗ trợ thêm so với level 2:
 *     --seed <n>     : đặt seed cho RNG (mặc định 8702)
 *     --parallel, -p : bật chế độ song song cho bot Hard (mức 3 nhánh B)
 *     --threads <n>  : số luồng (mặc định 0 = std::thread::hardware_concurrency)
 */
RunConfig parseArgs(int argc, char* argv[]) {
    RunConfig config;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-j" || arg == "--judge") {
            config.judge_mode  = true;
            config.interactive = false;

        } else if ((arg == "-i" || arg == "--input") && i + 1 < argc) {
            config.input_file = argv[++i];

        } else if ((arg == "-l" || arg == "--log")) {
            if (i + 1 < argc) {
                config.log_file = argv[++i];
            } else {
                config.log_file = "";
            }
            if (config.log_file.empty()) config.to_file = false;

        } else if (arg == "-v" || arg == "--verbose") {
            config.verbose_flag = true;

        } else if (arg == "-g" || arg == "--gui_flag") {
            config.gui_flag = true;

        } else if ((arg == "-s" || arg == "--seed") && i + 1 < argc) {
            config.seed = static_cast<std::uint32_t>(std::atoi(argv[++i]));

        } else if (arg == "-p" || arg == "--parallel") {
            config.parallel_flag = true;

        } else if (arg == "--threads" && i + 1 < argc) {
            config.num_threads = std::atoi(argv[++i]);

        } else if (arg == "-h" || arg == "--help") {
            config.is_help = true;
        }
    }

    return config;
}

std::ostream& operator<<(std::ostream& os, const RunConfig& config) {
    os << std::boolalpha;
    os << "RunConfig {\n"
       << "  interactive:   " << config.interactive   << "\n"
       << "  judge_mode:    " << config.judge_mode    << "\n"
       << "  input_file:    \"" << config.input_file  << "\"\n"
       << "  to_file:       " << config.to_file       << "\n"
       << "  log_file:      \"" << config.log_file    << "\"\n"
       << "  verbose_flag:  " << config.verbose_flag  << "\n"
       << "  gui_flag:      " << config.gui_flag      << "\n"
       << "  seed:          " << config.seed          << "\n"
       << "  parallel_flag: " << config.parallel_flag << "\n"
       << "  num_threads:   " << config.num_threads   << "\n"
       << "  is_help:       " << config.is_help       << "\n"
       << "}";
    return os;
}

std::string RunConfig::toString() const {
    std::stringstream ss;
    ss << *this;
    return ss.str();
}

std::string configHelpStr() {
    std::stringstream ss;
    ss << "Tic-tac-toe Game -- Level 3 (FP) (Version: " << VERSION << ")\n";
    ss << "Usage:\n"
       << "\t--judge,    -j           Enable judge mode (no UI, stdout only)\n"
       << "\t--input,    -i <path>    Path to input file\n"
       << "\t--log,      -l <path>    Path to log file (default: log.txt)\n"
       << "\t--verbose,  -v           Verbose log (DEBUG level)\n"
       << "\t--gui_flag, -g           Enable GUI (SDL -- optional in level 3)\n"
       << "\t--seed,     -s <n>       Seed for RNG (default: 8702)\n"
       << "\t--parallel, -p           Enable parallel bot HARD (level 3 -- track B)\n"
       << "\t--threads      <n>       Number of threads (0 = auto)\n"
       << "\t--help,     -h           Show this help message\n";
    return ss.str();
}
