/**
 * Tic-tac-toe Game in C++ -- Level 3 (Functional Programming)
 *
 * Mo ta:
 *   File `main.cpp` la "composition root" cua chuong trinh:
 *   noi DUY NHAT chung ta tao ra Logger, RNG, Renderer, Interaction...
 *   roi INJECT chung xuong Engine.
 *
 *   Sau khi sinh vien hoan thanh muc 1 task 3, KHONG co bien toan cuc
 *   nao lien quan den Logger / RNG nua -- moi phu thuoc duoc khai bao
 *   tuong minh o day.
 *
 * level 3.0 (FP paradigm -- pure core / imperative shell)
 * base from level 2 (OOP) v0.5 13/05/2026
 */

/* ---------- Importing ---------- */

#include <iostream>
#include <memory>

#include "interface/i_interaction.h"
#include "interface/i_renderer.h"
#include "shell/engine.h"
#include "shell/logger.h"
#include "shell/rng.h"
#include "terminal/interaction.h"
#include "terminal/renderer.h"
#include "utils/config.h"
#include "core/property_test.h"
#include "core/parallel_test.h"

// SDL -- chi include neu sinh vien muon giu phan GUI tu level 2.
#ifdef USE_SDL
#include "sdl/interaction.h"
#include "sdl/renderer.h"
#endif

#ifdef _WIN32
#include <windows.h>
#undef ERROR
void disableQuickEdit() {
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    DWORD prev_mode;
    GetConsoleMode(hInput, &prev_mode);
    SetConsoleMode(hInput, prev_mode & ~ENABLE_QUICK_EDIT_MODE);
}
#endif

/**
 * Mo ta: Entry point.
 */
int main(int argc, char* argv[]) {
#ifdef _WIN32
    disableQuickEdit();
#endif

    // ── 1. Parse config ────────────────────────────────────────────────
    RunConfig config = parseArgs(argc, argv);

    if (config.is_help) {
        std::cout << configHelpStr();
        return 0;
    }

    // ── 2. Tao Logger (KHONG global, duoc inject xuong Engine) ─────────
    Logger logger;
    logger.init(config.judge_mode, config.to_file, config.log_file, config.verbose_flag);
    logger.log("Logger initialized!");
    logger.log(config.toString(), Logger::Level::DEBUG);

    // ── 3. Tao RNG (KHONG global, duoc inject xuong Engine/Bot) ────────
    Rng rng = makeRng(config.seed);
    logger.log("RNG initialized with seed = " + std::to_string(config.seed));

    // ── 3.5. Chay Property-based tests (Muc 3 - Nhanh A) ────────────────
    bool propertyTestPassed = core::test::runPropertyTests(&logger, rng);
    if (!propertyTestPassed) {
        logger.log("[Property Test] FAILED! Exiting...", Logger::Level::ERROR);
        return 1;
    }

    // ── 3.6. Chay Parallel equality test (Muc 3 - Nhanh B) ─────────────
    try {
        bool parallelTestPassed = core::test::runParallelEqualityTest(&logger, rng, 100);
        if (!parallelTestPassed) {
            logger.log("[Parallel Test] Equality FAILED! Exiting...", Logger::Level::ERROR);
            return 1;
        }
    } catch (const std::exception& e) {
        logger.log(std::string("[Parallel Test] EXCEPTION: ") + e.what(), Logger::Level::ERROR);
        return 1;
    }

    // ── 3.7. Benchmark parallel vs sequential (verbose mode) ────────────
    if (config.verbose_flag) {
        auto benchmarkResults = core::test::runParallelBenchmark(&logger, rng, 30, config.num_threads);
        logger.log("[Parallel Benchmark] Complete. See log for details.", Logger::Level::INFO);
    }

    // ── 4. Tao Renderer & Interaction theo che do ──────────────────────
    std::unique_ptr<I_Renderer> iRenderer;
    std::unique_ptr<I_Interaction> iInteraction;

    if (!config.gui_flag || config.judge_mode) {
        iRenderer = std::make_unique<TerminalRenderer>();
        iInteraction = std::make_unique<TerminalInteraction>();
        logger.log("Terminal renderer & interaction initialized!");
    } else {
#ifdef USE_SDL
        iRenderer    = std::make_unique<SDLRenderer>();
        iInteraction = std::make_unique<SDLInteraction>();
        logger.log("SDL renderer & interaction initialized!");
#else
        logger.log("SDL chua duoc kich hoat trong starter level 3 -- fallback Terminal.",
                   Logger::Level::WARNING);
        iRenderer = std::make_unique<TerminalRenderer>();
        iInteraction = std::make_unique<TerminalInteraction>();
#endif
    }

    // ── 5. Tao Engine -- INJECT moi dependency ──────────────────────────
    Engine engine(&config, iRenderer.get(), iInteraction.get(), &logger, &rng);

    try {
        engine.init();
        engine.startGame();
        auto [result, finalState] = engine.playGame();
        engine.endGame(result, finalState);
    } catch (const QuitException&) {
        logger.log("Quit signal received. Cleaning up...", Logger::Level::WARNING);
    } catch (const NotImplementedException& e) {
        logger.log(std::string("Some functions are not implemented yet: ") + e.what(),
                   Logger::Level::WARNING);
    }

    // ── 6. Cleanup (RAII tu lo phan con lai) ───────────────────────────
    engine.close();
    logger.log("Engine destroyed!");
    logger.log("Logger closing . . .");
    logger.close();

    return 0;
}

/* ---------- Compile ---------- */
/**
 * Open terminal at folder 'level-3':
 *
 * 1 - Prepare build folder:
 *     mkdir build && cd build
 *
 * 2 - Config project:
 *     cmake ..
 *
 * 3 - Compile:
 *     cmake --build .
 *
 * 4 - Run (IMPORTANT: set PATH to avoid DLL conflict):
 *     PATH="/f/mingw64/bin:$PATH" ./game.exe
 */
