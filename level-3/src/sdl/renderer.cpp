/**
 * SDL Renderer implementation
 */

#include "renderer.h"
#include "interaction.h"

#include <iostream>
#include <sstream>

namespace {

SDL_Color COLOR_BG       = {30, 30, 40, 255};
SDL_Color COLOR_BOARD    = {200, 180, 160, 255};
SDL_Color COLOR_GRID     = {80, 70, 60, 255};
SDL_Color COLOR_X        = {60, 140, 220, 255};
SDL_Color COLOR_O        = {220, 60, 60, 255};
SDL_Color COLOR_TEXT     = {240, 240, 240, 255};
SDL_Color COLOR_TITLE    = {255, 220, 100, 255};
SDL_Color COLOR_SUCCESS  = {80, 200, 80, 255};
SDL_Color COLOR_ERROR    = {255, 80, 80, 255};
SDL_Color COLOR_WINLINE  = {255, 255, 100, 255};

}  // namespace

SDLRenderer::SDLRenderer()
    : window_(nullptr), renderer_(nullptr), font_(nullptr), fontLarge_(nullptr),
      boardSize_(0), goal_(0), cellSize_(0), offsetX_(0), offsetY_(0) {}

SDLRenderer::~SDLRenderer() {
    close();
}

void SDLRenderer::init(const RunConfig& config) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return;
    }
    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        return;
    }

    window_ = SDL_CreateWindow("Tic-Tac-Toe / Gomoku - Level 3 (FP)",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                config.screenWidth, config.screenHeight, 0);
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << "\n";
        return;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << "\n";
        return;
    }

    // Try to load a font - use system fonts as fallback
    const char* fontPaths[] = {
        "assets/fonts/arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/consola.ttf",
    };

    for (const char* path : fontPaths) {
        font_ = TTF_OpenFont(path, 24);
        if (font_) break;
    }
    if (!font_) {
        std::cerr << "Warning: No font found, text rendering may fail\n";
    }

    fontLarge_ = TTF_OpenFont(fontPaths[1], 36);
    if (!fontLarge_) {
        fontLarge_ = font_;
    }

    boardSize_ = 0;
}

void SDLRenderer::clearScreen() {
    if (!renderer_) return;
    SDL_SetRenderDrawColor(renderer_, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, COLOR_BG.a);
    SDL_RenderClear(renderer_);
    SDL_RenderPresent(renderer_);
}

void SDLRenderer::showSelectMenu(SelectType selectType, int context) {
    if (!renderer_) return;

    std::string title, subtitle;

    switch (selectType) {
        case SelectType::TITLE_UI:
            title = "TIC-TAC-TOE / GOMOKU";
            subtitle = "Level 3 - Functional Programming\n\nPress any key to continue...";
            break;
        case SelectType::SIZE_UI:
            title = "SELECT BOARD SIZE";
            subtitle = "Enter board size (3-12):";
            break;
        case SelectType::GOAL_UI:
            title = "SELECT GOAL";
            subtitle = "Enter goal (3-" + std::to_string(std::min(context, GOAL_MAX)) + "):";
            break;
        case SelectType::GAME_MODE_UI:
            title = "SELECT GAME MODE";
            subtitle = "1. PVP (Player vs Player)\n2. PVE (Player vs Bot)\n3. EVE (Bot vs Bot)";
            break;
        case SelectType::BOT_LEVEL_UI:
            title = "SELECT BOT LEVEL";
            subtitle = "1. EASY\n2. MEDIUM\n3. HARD";
            break;
        case SelectType::PLAYER_UI:
            title = "CHOOSE SIDE";
            subtitle = "1. Play as X (go first)\n2. Play as O (go second)";
            break;
        case SelectType::MUL_BOT_LEVEL_UI:
            title = "SELECT BOT " + std::to_string(context + 1) + " LEVEL";
            subtitle = "1. EASY\n2. MEDIUM\n3. HARD";
            break;
        default:
            break;
    }

    renderMenu(title.c_str(), subtitle.c_str());
    SDLInteraction::setMenuInfo(title, subtitle);
    presentAndWait();
}

void SDLRenderer::showInvalidSelect(SelectType /*selectType*/, int /*context*/) {
    if (!renderer_) return;
    clearScreen();
    renderText("Invalid selection! Try again.", 50, 300, COLOR_ERROR, fontLarge_);
    presentAndWait(1000);
}

void SDLRenderer::showValidSelect(SelectType selectType, int context) {
    if (!renderer_) return;

    std::string msg;
    switch (selectType) {
        case SelectType::SIZE_UI:
            msg = "Board size: " + std::to_string(context) + "x" + std::to_string(context);
            break;
        case SelectType::GOAL_UI:
            msg = "Goal: " + std::to_string(context);
            break;
        case SelectType::GAME_MODE_UI:
            msg = "Mode: " + modeToString(context);
            break;
        case SelectType::BOT_LEVEL_UI:
        case SelectType::MUL_BOT_LEVEL_UI:
            msg = "Bot level: " + botToString(context);
            break;
        default:
            return;
    }

    clearScreen();
    renderText(msg.c_str(), 50, 300, COLOR_SUCCESS, fontLarge_);
    presentAndWait(800);
}

void SDLRenderer::displayBoard(const Board& board) {
    if (!renderer_) return;
    boardSize_ = board.size;
    cellSize_ = std::min(600 / boardSize_, 80);
    offsetX_ = (800 - cellSize_ * boardSize_) / 2;
    offsetY_ = 120;

    clearScreen();
    renderBoard(board);
    presentAndWait();
}

void SDLRenderer::showMove(int row, int col) {
    if (!renderer_) return;
    std::string msg = "Move: (" + std::to_string(row) + ", " + std::to_string(col) + ")";
    renderText(msg.c_str(), 50, 750, COLOR_TEXT);
    SDL_RenderPresent(renderer_);
}

void SDLRenderer::showInvalidMove() {
    if (!renderer_) return;
    renderText("Invalid move!", 50, 750, COLOR_ERROR);
    SDL_RenderPresent(renderer_);
}

void SDLRenderer::showPlayer(int player, bool is_bot) {
    if (!renderer_) return;

    char symbol = (player == 0) ? SYMBOL_X : SYMBOL_O;
    SDL_Color color = (player == 0) ? COLOR_X : COLOR_O;
    std::string type = is_bot ? " [BOT]" : "";
    std::string msg = "Player " + std::string(1, symbol) + type + "'s turn";

    renderText(msg.c_str(), 50, 750, color, fontLarge_);
    SDL_RenderPresent(renderer_);
}

void SDLRenderer::showResult(int winner, bool is_bot, const WinLine* /*winLine*/) {
    if (!renderer_) return;

    clearScreen();

    // Redraw board if we have one
    if (boardSize_ > 0) {
        // Board would need to be stored; for now just show result
    }

    if (winner == DRAW_RESULT) {
        renderText("=== DRAW! ===", 150, 300, COLOR_TITLE, fontLarge_);
    } else {
        char symbol = (winner == 0) ? SYMBOL_X : SYMBOL_O;
        SDL_Color color = (winner == 0) ? COLOR_X : COLOR_O;
        std::string type = is_bot ? " (BOT)" : "";
        std::string msg = "=== PLAYER " + std::string(1, symbol) + type + " WINS! ===";
        renderText(msg.c_str(), 100, 300, color, fontLarge_);
    }
    presentAndWait();
}

void SDLRenderer::printResult(const GameResult& gameResult) {
    if (!renderer_) return;

    std::string msg;
    SDL_Color color = COLOR_TEXT;

    if (gameResult.winner == DRAW_RESULT) {
        msg = "Result: DRAW";
        color = COLOR_TITLE;
    } else {
        char symbol = (gameResult.winner == 0) ? SYMBOL_X : SYMBOL_O;
        std::string type = gameResult.isBot ? " (BOT)" : "";
        msg = "Winner: Player " + std::string(1, symbol) + type;
        color = (gameResult.winner == 0) ? COLOR_X : COLOR_O;
    }

    renderText(msg.c_str(), 150, 400, color, fontLarge_);

    std::string turns = "Total turns: " + std::to_string(gameResult.turns);
    renderText(turns.c_str(), 150, 460, COLOR_TEXT);

    renderText("Press any key to exit...", 150, 550, COLOR_TEXT);
    presentAndWait();
}

void SDLRenderer::close() {
    if (fontLarge_ && fontLarge_ != font_) TTF_CloseFont(fontLarge_);
    if (font_) TTF_CloseFont(font_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
    TTF_Quit();
    SDL_Quit();

    window_ = nullptr;
    renderer_ = nullptr;
    font_ = nullptr;
    fontLarge_ = nullptr;
}

void SDLRenderer::renderText(const char* text, int x, int y, SDL_Color color, TTF_Font* font) {
    if (!renderer_ || !font || !text) return;

    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};

    SDL_RenderCopy(renderer_, texture, nullptr, &dst);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void SDLRenderer::renderBoard(const Board& board, const WinLine* winLine) {
    if (!renderer_ || boardSize_ == 0) return;

    // Draw board background
    SDL_Rect boardRect = {offsetX_ - 10, offsetY_ - 10,
                          cellSize_ * boardSize_ + 20,
                          cellSize_ * boardSize_ + 20};
    SDL_SetRenderDrawColor(renderer_, COLOR_BOARD.r, COLOR_BOARD.g, COLOR_BOARD.b, COLOR_BOARD.a);
    SDL_RenderFillRect(renderer_, &boardRect);

    // Draw grid lines
    SDL_SetRenderDrawColor(renderer_, COLOR_GRID.r, COLOR_GRID.g, COLOR_GRID.b, COLOR_GRID.a);
    for (int i = 0; i <= boardSize_; ++i) {
        // Vertical
        SDL_RenderDrawLine(renderer_,
            offsetX_ + i * cellSize_, offsetY_,
            offsetX_ + i * cellSize_, offsetY_ + boardSize_ * cellSize_);
        // Horizontal
        SDL_RenderDrawLine(renderer_,
            offsetX_, offsetY_ + i * cellSize_,
            offsetX_ + boardSize_ * cellSize_, offsetY_ + i * cellSize_);
    }

    // Draw cells
    for (int r = 0; r < boardSize_; ++r) {
        for (int c = 0; c < boardSize_; ++c) {
            char cell = board.at(r, c);
            if (cell == EMPTY_CELL) continue;

            SDL_Color color = (cell == SYMBOL_X) ? COLOR_X : COLOR_O;
            std::string s(1, cell);

            int cx = offsetX_ + c * cellSize_ + cellSize_ / 2;
            int cy = offsetY_ + r * cellSize_ + cellSize_ / 2;

            // Center text
            TTF_Font* cellFont = fontLarge_;
            SDL_Surface* surface = TTF_RenderText_Solid(cellFont, s.c_str(), color);
            if (surface) {
                SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
                SDL_Rect dst = {cx - surface->w / 2, cy - surface->h / 2, surface->w, surface->h};
                SDL_RenderCopy(renderer_, texture, nullptr, &dst);
                SDL_DestroyTexture(texture);
                SDL_FreeSurface(surface);
            }
        }
    }

    // Draw win line highlight
    if (winLine && !winLine->cells.empty()) {
        SDL_SetRenderDrawColor(renderer_, COLOR_WINLINE.r, COLOR_WINLINE.g, COLOR_WINLINE.b, 128);
        for (const auto& cell : winLine->cells) {
            SDL_Rect rect = {offsetX_ + cell.second * cellSize_,
                             offsetY_ + cell.first * cellSize_,
                             cellSize_, cellSize_};
            SDL_RenderFillRect(renderer_, &rect);
        }
    }
}

void SDLRenderer::renderMenu(const char* title, const char* subtitle) {
    if (!renderer_) return;

    clearScreen();

    if (title) {
        renderText(title, 150, 200, COLOR_TITLE, fontLarge_);
    }
    if (subtitle) {
        renderText(subtitle, 150, 280, COLOR_TEXT);
    }
}

void SDLRenderer::presentAndWait(int delayMs) {
    if (!renderer_) return;
    SDL_RenderPresent(renderer_);
    if (delayMs > 0) {
        SDL_Delay(delayMs);
    }
}
