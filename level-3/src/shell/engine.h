/**
 * Engine header file (Level 3 -- shell layer)
 *
 * Mô tả:
 *   Engine ở level 3 nằm ở "imperative shell": là nơi phối hợp các pure
 *   function ở `core/` với IO ở `interface/` + `terminal/` + `sdl/`.
 *
 *   Khác biệt then chốt so với level 2:
 *
 *   ┌──────────────────────┬─────────────────────┬─────────────────────────┐
 *   │ Khía cạnh            │ Level 2             │ Level 3                 │
 *   ├──────────────────────┼─────────────────────┼─────────────────────────┤
 *   │ Logger               │ global (inline)     │ INJECT qua constructor  │
 *   │ Random generator     │ global (inline)     │ INJECT qua constructor  │
 *   │ Board                │ mutate trong loop   │ IMMUTABLE -- applyTurn  │
 *   │ Bot                  │ class hierarchy     │ BotFn (function value)  │
 *   │ Game loop            │ while+mutate        │ fold / đệ quy đuôi      │
 *   └──────────────────────┴─────────────────────┴─────────────────────────┘
 *
 *   Mức 1 yêu cầu (task 3): Logger được DI.
 *   Mức 2 yêu cầu (Engine): không mutate board, fold/đệ quy.
 */

#pragma once

#include <utility>

/* ---------- Importing ---------- */

#include "../core/types.h"
#include "../interface/i_interaction.h"
#include "../interface/i_renderer.h"
#include "../utils/config.h"
#include "logger.h"
#include "rng.h"

/* ---------- Declarations ---------- */

class Engine {
   public:
    /**
     * Mô tả:
     *   Constructor -- nhận TẤT CẢ dependency từ ngoài (DI).
     *
     * Đầu vào:
     *   - config        : cấu hình runtime (không sở hữu).
     *   - iRenderer     : implementation hiển thị (không sở hữu).
     *   - iInteraction  : implementation input (không sở hữu).
     *   - logger        : Logger được tạo ở main.cpp (không sở hữu).
     *   - rng           : RNG được tạo ở main.cpp (không sở hữu).
     */
    Engine(const RunConfig* config,
           I_Renderer* iRenderer,
           I_Interaction* iInteraction,
           Logger* logger,
           Rng* rng);

    ~Engine();

    /**
     * Mô tả: Khởi tạo engine (delegate xuống renderer/interaction).
     */
    void init();

    /**
     * Mô tả: Setup ban đầu -- lấy size/goal/mode/bot levels từ user.
     *   Phần này chủ yếu IO → ở shell, không thay đổi triết lý so với LV2.
     */
    void startGame();

    /**
     * Mô tả:
     *   Game loop chính -- TRẢ VỀ GameResult và GameState cuối cùng.
     *
     * Mức 2 yêu cầu:
     *   - Không mutate `gameSetup.board`. Mỗi turn: gọi `applyTurn(state, move, goal)`
     *     từ `core/logic.h` → state mới.
     *   - Diễn đạt loop bằng đệ quy đuôi hoặc `std::ranges::fold` (C++23) hoặc
     *     `std::accumulate` qua các turn.
     *   - Side-effect duy nhất ở đây là LOG và RENDER -- đều cô lập rõ ràng.
     *   - KHÔNG mutate gameState_ member -- trả về state mới thay vì gán.
     */
    std::pair<GameResult, GameState> playGame();

    /**
     * Mô tả: Hiển thị kết quả + log thống kê.
     * @param gameResult Kết quả game
     * @param finalState Trạng thái bàn cờ cuối cùng (không mutate)
     */
    void endGame(const GameResult& gameResult, const GameState& finalState);

    /**
     * Mô tả: Dọn dẹp.
     */
    void close();

   private:
    const RunConfig* config_;
    I_Renderer* iRenderer_;
    I_Interaction* iInteraction_;
    Logger* logger_;  // INJECT -- không global (yêu cầu mức 1)
    Rng* rng_;        // INJECT -- không global (yêu cầu mức 1)

    GameSetup gameSetup_;
    GameState gameState_;  // trạng thái hiện tại của game (immutable steps)

    /**
     * Mô tả: Kiểm tra dependency có hợp lệ không (logger, renderer, interaction…).
     */
    bool sanity_check();
};
