/**
 * Core types header file
 *
 * Mô tả:
 *   Định nghĩa các kiểu dữ liệu cốt lõi của game ở dạng IMMUTABLE.
 *   Đây là file chuẩn của tầng `core/` -- tuyệt đối không phụ thuộc IO,
 *   không log, không random, không gì gây side-effect.
 *
 * Triết lý FP:
 *   - Mọi struct ở đây đều thuần dữ liệu (POD-like).
 *   - Các phép biến đổi đều trả về giá trị mới, không sửa tại chỗ.
 *   - Có thể truyền/copy thoải mái -- nhỏ và rẻ.
 */

#pragma once

/* ---------- Importing ---------- */

#include <array>
#include <exception>
#include <string>
#include <vector>

/* ---------- Constants ---------- */

inline constexpr int BOARD_N_MIN = 3;    // kích thước bàn cờ nhỏ nhất
inline constexpr int BOARD_N_MAX = 12;   // kích thước bàn cờ lớn nhất
inline constexpr int GOAL_MAX = 5;       // số quân cần để thắng tối đa
inline constexpr int SLEEP_TIME = 1500;  // delay (ms) khi bot chơi (chỉ dùng ở shell)

inline constexpr int SCORE_INF = 1000;     // giá trị "vô cùng" (dùng trong minimax)
inline constexpr int EVALUATE_SCORE = 10;  // điểm đánh giá cơ bản

inline constexpr char EMPTY_CELL = '.';  // ký tự ô trống
inline constexpr char SYMBOL_X = 'X';    // ký tự player 0
inline constexpr char SYMBOL_O = 'O';    // ký tự player 1

inline constexpr int DRAW_RESULT = -1;  // giá trị biểu diễn hòa
inline constexpr int NO_CONTEXT = -1;   // không có context

/* ---------- Enum ---------- */

/**
 * Mô tả: Mức độ khó của bot.
 */
enum class BotLevel {
    EASY,
    MEDIUM,
    HARD,
    INVALID_LV
};

/**
 * Mô tả: Chuyển BotLevel sang string để hiển thị/log.
 */
inline std::string botToString(int v) {
    switch ((BotLevel)v) {
        case BotLevel::EASY:   return "EASY";
        case BotLevel::MEDIUM: return "MEDIUM";
        case BotLevel::HARD:   return "HARD";
        default:               return "?";
    }
}

/**
 * Mô tả: Chế độ chơi của game.
 */
enum class GameMode {
    PVP,  // Player vs Player
    PVE,  // Player vs Bot
    EVE,  // Bot vs Bot
    INVALID_MODE
};

/**
 * Mô tả: Chuyển GameMode sang string.
 */
inline std::string modeToString(int v) {
    switch ((GameMode)v) {
        case GameMode::PVP: return "PVP";
        case GameMode::PVE: return "PVE";
        case GameMode::EVE: return "EVE";
        default:            return "?";
    }
}

/**
 * Mô tả:
 *   Loại UI selection đang hiển thị. Renderer dựa vào đây để vẽ.
 */
enum class SelectType {
    TITLE_UI,
    SIZE_UI,
    GOAL_UI,
    GAME_MODE_UI,
    BOT_LEVEL_UI,
    PLAYER_UI,
    MUL_BOT_LEVEL_UI,
    INVALID_UI
};

/**
 * Mô tả: Quy tắc kết thúc (dùng trong logic Gomoku).
 */
enum class EndRule {
    NONE,      // không kiểm tra đầu mở
    OPEN_ONE,  // có ít nhất 1 đầu mở
    OPEN_TWO   // có đúng 2 đầu mở
};

/* ---------- Type Definitions ---------- */

// alias cho cặp toạ độ (row, col)
using pII = std::pair<int, int>;

/**
 * Mô tả:
 *   Kiểu Move biểu diễn một nước đi.
 *   Bất biến: row, col cố định khi đã tạo.
 */
struct Move {
    int row;
    int col;

    constexpr bool operator==(const Move& o) const noexcept {
        return row == o.row && col == o.col;
    }
};

inline constexpr Move INVALID_MOVE{-1, -1};

/**
 * Mô tả:
 *   Kiểu Board IMMUTABLE -- bao bọc 1 ma trận tĩnh + kích thước thực tế.
 *
 * Lý do dùng std::array (kích thước cố định) thay vì std::vector:
 *   - Cấp phát trên stack → copy nhanh, dễ pass-by-value.
 *   - Phù hợp với BOARD_N_MAX nhỏ (12×12 = 144 ô).
 *
 * NOTE quan trọng cho FP:
 *   - Hãy LUÔN truyền `const Board&` hoặc `Board` (by-value, copy là rẻ).
 *   - KHÔNG bao giờ truyền `Board&` trừ khi thực sự cần mutate (ở imperative shell).
 *   - Mọi phép biến đổi nên trả về `Board` mới (xem core/logic.h).
 */
struct Board {
    std::array<std::array<char, BOARD_N_MAX>, BOARD_N_MAX> grid{};
    int size{0};

    constexpr char at(int r, int c) const noexcept {
        return grid[r][c];
    }

    constexpr bool inRange(int r, int c) const noexcept {
        return 0 <= r && r < size && 0 <= c && c < size;
    }

    bool operator==(const Board& o) const noexcept {
        if (size != o.size) return false;
        for (int r = 0; r < size; ++r)
            for (int c = 0; c < size; ++c)
                if (grid[r][c] != o.grid[r][c]) return false;
        return true;
    }
};

/**
 * Mô tả:
 *   GameState -- toàn bộ trạng thái game ở một thời điểm.
 *   Bất biến: mỗi turn tạo ra một GameState mới (xem applyTurn ở mức 2).
 *
 * Đầu vào: dùng để làm fold-state trong game loop FP-style.
 */
struct GameState {
    Board board;          // trạng thái bàn cờ
    int currentPlayer;    // 0 hoặc 1
    int turn;             // số lượt đã đi
    int winner;           // -1 nếu chưa kết thúc / hòa; 0 hoặc 1 nếu có người thắng
    bool isFinished;      // game đã kết thúc hay chưa
};

/**
 * Mô tả: Cấu hình ban đầu của game (do startGame thiết lập).
 */
struct GameSetup {
    int size{0};                 // kích thước board
    int goal{0};                 // số quân cần để thắng
    GameMode mode{GameMode::INVALID_MODE};
    std::array<BotLevel, 2> levels{BotLevel::INVALID_LV, BotLevel::INVALID_LV};
};

/**
 * Mô tả: Kết quả cuối cùng của một trận.
 */
struct GameResult {
    int winner;   // 0 hoặc 1 là player thắng, -1 là hòa
    bool isBot;   // người thắng có phải bot không
    int turns;    // số lượt đã chơi

    constexpr GameResult(int _winner, bool _isBot, int _turns) noexcept
        : winner(_winner), isBot(_isBot), turns(_turns) {}
};

/**
 * Mô tả:
 *   WinLine -- danh sách các ô tạo thành đường thắng.
 *   Dùng để renderer highlight đường thắng.
 */
struct WinLine {
    std::vector<pII> cells;
};

/* ---------- Exceptions ---------- */

/**
 * Mô tả: Tín hiệu user yêu cầu thoát game (dùng để break ra khỏi loop).
 */
class QuitException : public std::exception {
   public:
    const char* what() const noexcept override {
        return "User requested quit";
    }
};

/**
 * Mô tả: Tín hiệu chức năng chưa cài đặt (placeholder cho stub).
 */
class NotImplementedException : public std::exception {
   public:
    const char* what() const noexcept override {
        return "Functionality not implemented yet";
    }
};
