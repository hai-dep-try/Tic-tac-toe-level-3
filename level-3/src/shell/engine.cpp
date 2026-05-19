/**
 * Engine implementation
 */

#include "engine.h"
#include <chrono>
#include <sstream>
#include <thread>
#include <future>
#include "../core/bot_pure.h"
#include "../core/logic.h"
#include "../utils/helper.h"

#ifdef USE_SDL
#include <SDL.h>
#include "../sdl/renderer.h"
#include "../sdl/interaction.h"
#endif

/* ---------- Definitions ---------- */

Engine::Engine(const RunConfig* config,
               I_Renderer* iRenderer,
               I_Interaction* iInteraction,
               Logger* logger,
               Rng* rng)
    : config_(config),
      iRenderer_(iRenderer),
      iInteraction_(iInteraction),
      logger_(logger),
      rng_(rng) {}

Engine::~Engine() = default;

/* ---------- init ---------- */

void Engine::init() {
    if (logger_) logger_->log("Engine initializing . . .");

    if (iRenderer_)    iRenderer_->init(*config_);

#ifdef USE_SDL
    if (config_->gui_flag && !config_->judge_mode) {
        SDLRenderer* sdlRenderer = dynamic_cast<SDLRenderer*>(iRenderer_);
        SDLInteraction* sdlInteraction = dynamic_cast<SDLInteraction*>(iInteraction_);
        if (sdlRenderer && sdlInteraction) {
            sdlInteraction->setSharedResources(sdlRenderer->getWindow(), sdlRenderer->getRenderer());
        }
    }
#endif

    if (iInteraction_) iInteraction_->init(*config_);

    if (logger_) logger_->log("Engine initialized!");
}

bool Engine::sanity_check() {
    bool ok = true;
    if (!iRenderer_) {
        if (logger_) logger_->log("Renderer not implemented!", Logger::Level::WARNING);
        ok = false;
    }
    if (!iInteraction_) {
        if (logger_) logger_->log("Interaction not implemented!", Logger::Level::WARNING);
        ok = false;
    }
    if (!logger_) {
        ok = false;
    }
    if (!rng_) {
        if (logger_) logger_->log("RNG not provided!", Logger::Level::ERROR);
        ok = false;
    }
    return ok;
}

/* ---------- startGame ---------- */

/**
 * Mô tả: Lấy input cấu hình từ user (size, goal, mode, bot levels).
 */
void Engine::startGame() {
    if (logger_) logger_->log("[Engine] Starting game . . .");

    if (!sanity_check()) {
        if (logger_) logger_->log("[Engine] Game stopped!", Logger::Level::ERROR);
        return;
    }

    iRenderer_->clearScreen();
    iRenderer_->showSelectMenu(SelectType::TITLE_UI);
    iInteraction_->pause();

    int size = 0;
    while (true) {
        iRenderer_->showSelectMenu(SelectType::SIZE_UI);
        if (iInteraction_->selectSize(&size)) {
            iRenderer_->showValidSelect(SelectType::SIZE_UI, size);
            break;
        }
        iRenderer_->showInvalidSelect(SelectType::SIZE_UI);
    }
    gameSetup_.size = size;

    int goal = 0;
    while (true) {
        iRenderer_->showSelectMenu(SelectType::GOAL_UI, size);
        if (iInteraction_->selectGoal(&goal, size)) {
            iRenderer_->showValidSelect(SelectType::GOAL_UI, goal);
            break;
        }
        iRenderer_->showInvalidSelect(SelectType::GOAL_UI);
    }
    gameSetup_.goal = goal;

    GameMode mode = GameMode::INVALID_MODE;
    while (true) {
        iRenderer_->showSelectMenu(SelectType::GAME_MODE_UI);
        if (iInteraction_->selectGameMode(&mode)) {
            iRenderer_->showValidSelect(SelectType::GAME_MODE_UI, static_cast<int>(mode));
            break;
        }
        iRenderer_->showInvalidSelect(SelectType::GAME_MODE_UI);
    }
    gameSetup_.mode = mode;

    if (mode == GameMode::PVE) {
        while (true) {
            iRenderer_->showSelectMenu(SelectType::BOT_LEVEL_UI);
            if (iInteraction_->selectBotLevel(gameSetup_.levels.data(), 1)) {
                iRenderer_->showValidSelect(SelectType::BOT_LEVEL_UI, static_cast<int>(gameSetup_.levels[1]));
                break;
            }
            iRenderer_->showInvalidSelect(SelectType::BOT_LEVEL_UI);
        }
    } else if (mode == GameMode::EVE) {
        while (true) {
            iRenderer_->showSelectMenu(SelectType::MUL_BOT_LEVEL_UI, 0);
            if (iInteraction_->selectBotLevel(gameSetup_.levels.data(), 0)) {
                iRenderer_->showValidSelect(SelectType::MUL_BOT_LEVEL_UI, static_cast<int>(gameSetup_.levels[0]));
                break;
            }
            iRenderer_->showInvalidSelect(SelectType::MUL_BOT_LEVEL_UI);
        }

        while (true) {
            iRenderer_->showSelectMenu(SelectType::MUL_BOT_LEVEL_UI, 1);
            if (iInteraction_->selectBotLevel(gameSetup_.levels.data(), 1)) {
                iRenderer_->showValidSelect(SelectType::MUL_BOT_LEVEL_UI, static_cast<int>(gameSetup_.levels[1]));
                break;
            }
            iRenderer_->showInvalidSelect(SelectType::MUL_BOT_LEVEL_UI);
        }
    }

    gameState_.board         = core::initBoard(gameSetup_.size);
    gameState_.currentPlayer = 0;
    gameState_.turn          = 0;
    gameState_.winner        = -1;
    gameState_.isFinished    = false;
}

/* ---------- playGame ---------- */

/**
 * Mô tả: Game loop CHÍNH (Đệ quy đuôi phong cách FP, KHÔNG mutate gameState_).
 */
std::pair<GameResult, GameState> Engine::playGame() {
    if (logger_) logger_->log("[Engine] Playing game . . .");

    if (!sanity_check()) {
        if (logger_) logger_->log("[Engine] Game stopped!", Logger::Level::ERROR);
        return {GameResult{-1, false, 0}, gameState_};
    }

#ifdef USE_SDL
    if (config_->gui_flag && !config_->judge_mode && gameSetup_.mode == GameMode::EVE) {
        SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
        SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
        SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
    }
#endif

    bot::BotFn bot0 = (gameSetup_.mode == GameMode::EVE)
                          ? bot::makeBot(gameSetup_.levels[0], gameSetup_.goal,
                                         config_->parallel_flag, config_->num_threads)
                          : bot::BotFn{};
    bot::BotFn bot1 = (gameSetup_.mode != GameMode::PVP)
                          ? bot::makeBot(gameSetup_.levels[1], gameSetup_.goal,
                                         config_->parallel_flag, config_->num_threads)
                          : bot::BotFn{};

    auto loop = [&](auto self, const GameState& s) -> std::pair<GameResult, GameState> {
        iRenderer_->clearScreen();
        iRenderer_->displayBoard(s.board);

        if (s.isFinished) {
            bool is_bot = false;
            if (s.winner == 0 && gameSetup_.mode == GameMode::EVE) is_bot = true;
            if (s.winner == 1 && gameSetup_.mode != GameMode::PVP) is_bot = true;

#ifdef USE_SDL
            if (config_->gui_flag && !config_->judge_mode) {
                SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
                SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
                SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
            }
#endif
            return {GameResult{s.winner, is_bot, s.turn}, s};
        }

        bool currentIsBot = (s.currentPlayer == 0 && gameSetup_.mode == GameMode::EVE) ||
                            (s.currentPlayer == 1 && gameSetup_.mode != GameMode::PVP);
        iRenderer_->showPlayer(s.currentPlayer, currentIsBot);

        Move m = INVALID_MOVE;
        if (currentIsBot) {
            bot::BotFn activeBot = (s.currentPlayer == 0) ? bot0 : bot1;
            std::string label = "Bot " + std::to_string(s.currentPlayer) + " (LV " +
                                botToString(static_cast<int>(gameSetup_.levels[s.currentPlayer])) + ") decision";

            // Run the bot computation in a background thread using std::async
            std::future<Move> botFuture = std::async(std::launch::async, [&]() {
                return measureExecutionTime(
                    label,
                    [&]{ return activeBot(s, *rng_); },
                    config_->verbose_flag,
                    [this](const std::string& msg){ logger_->log(msg, Logger::Level::DEBUG); }
                );
            });

            // Keep the SDL event queue completely responsive on the main thread
            while (botFuture.wait_for(std::chrono::milliseconds(10)) != std::future_status::ready) {
#ifdef USE_SDL
                if (config_->gui_flag && !config_->judge_mode) {
                    SDL_Event e;
                    while (SDL_PollEvent(&e)) {
                        if (e.type == SDL_QUIT) {
                            throw QuitException();
                        }
                    }
                }
#endif
            }

            m = botFuture.get();

            iRenderer_->showMove(m.row, m.col);
            iInteraction_->pause(SLEEP_TIME);
        } else {
            int r = 0, c = 0;
            while (true) {
                if (!iInteraction_->getPlayerMove(&r, &c)) {
                    throw QuitException();
                }
                Move humanMove{r, c};
                if (core::isValidMove(s.board, humanMove)) {
                    m = humanMove;
                    break;
                }
                iRenderer_->showInvalidMove();
            }
        }

        GameState nextState = core::applyTurn(s, m, gameSetup_.goal);
        return self(self, nextState);
    };

    try {
        return loop(loop, gameState_);
    } catch (const QuitException& e) {
        if (logger_) logger_->log("Game quit by user.", Logger::Level::INFO);
#ifdef USE_SDL
        if (config_->gui_flag && !config_->judge_mode) {
            SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_ENABLE);
            SDL_EventState(SDL_MOUSEBUTTONUP, SDL_ENABLE);
            SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
        }
#endif
        return {GameResult{-1, false, gameState_.turn}, gameState_};
    }
}

/* ---------- endGame ---------- */

/**
 * Mô tả: Hiển thị kết quả game.
 */
void Engine::endGame(const GameResult& gameResult, const GameState& finalState) {
    if (logger_) logger_->log("[Engine] Ending game . . .");

    if (!sanity_check()) {
        if (logger_) logger_->log("[Engine] Game stopped!", Logger::Level::ERROR);
        return;
    }

    iRenderer_->clearScreen();
    iRenderer_->displayBoard(finalState.board);

    std::optional<WinLine> wl = std::nullopt;
    if (gameResult.winner != -1) {
        char winSym = core::symbolOf(gameResult.winner);
        wl = core::getWinLine(finalState.board, winSym, gameSetup_.goal);
    }

    const WinLine* wl_ptr = wl.has_value() ? &wl.value() : nullptr;
    iRenderer_->showResult(gameResult.winner, gameResult.isBot, wl_ptr);
    iRenderer_->printResult(gameResult);

    if (logger_) {
        std::ostringstream ss;
        ss << "[Game Result] Winner: " << gameResult.winner
           << " (is_bot: " << (gameResult.isBot ? "true" : "false")
           << "), total turns: " << gameResult.turns;
        logger_->log(ss.str(), Logger::Level::INFO);
    }
    iInteraction_->pause();
}

/* ---------- close ---------- */

void Engine::close() {
    if (logger_) logger_->log("Engine closing . . .");

    if (iRenderer_)    iRenderer_->close();
    if (iInteraction_) iInteraction_->close();

    if (logger_) logger_->log("Engine closed!");
}
