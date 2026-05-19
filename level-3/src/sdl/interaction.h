/**
 * SDL Interaction header file
 */

#pragma once

#include "../interface/i_interaction.h"

#include <SDL.h>
#include <SDL_ttf.h>

class SDLInteraction : public I_Interaction {
   public:
    SDLInteraction();
    ~SDLInteraction() override;

    void init(const RunConfig& config) override;
    void pause(int timeout = 0) override;
    bool selectSize(int* size) override;
    bool selectGoal(int* goal, int size) override;
    bool selectGameMode(GameMode* mode) override;
    bool selectBotLevel(BotLevel* levels, int index) override;
    bool getPlayerMove(int* row, int* col) override;
    void close() override;

   private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    std::string inputBuffer_;

    void renderInputPrompt(const char* prompt);
    int waitForNumberInput(int minVal, int maxVal);
    int waitForChoice();
};
