/**
 * SDL Renderer header file
 */

#pragma once

#include "../interface/i_renderer.h"

#include <SDL.h>
#include <SDL_ttf.h>

class SDLRenderer : public I_Renderer {
   public:
    SDLRenderer();
    ~SDLRenderer() override;

    void init(const RunConfig& config) override;
    void clearScreen() override;
    void showSelectMenu(SelectType selectType, int context = NO_CONTEXT) override;
    void showInvalidSelect(SelectType selectType, int context = NO_CONTEXT) override;
    void showValidSelect(SelectType selectType, int context = NO_CONTEXT) override;
    void displayBoard(const Board& board) override;
    void showMove(int row, int col) override;
    void showInvalidMove() override;
    void showPlayer(int player, bool is_bot) override;
    void showResult(int winner, bool is_bot, const WinLine* winLine = nullptr) override;
    void printResult(const GameResult& gameResult) override;
    void close() override;

   private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    TTF_Font* fontLarge_;
    int boardSize_;
    int goal_;
    int cellSize_;
    int offsetX_;
    int offsetY_;

    void renderText(const char* text, int x, int y, SDL_Color color, TTF_Font* font = nullptr);
    void renderBoard(const Board& board, const WinLine* winLine = nullptr);
    void renderMenu(const char* title, const char* subtitle = nullptr);
    void presentAndWait(int delayMs = 0);
};
