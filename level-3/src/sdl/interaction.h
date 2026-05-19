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
    void setSharedResources(SDL_Window* window, SDL_Renderer* renderer) {
        window_ = window;
        renderer_ = renderer;
        shared_ = true;
    }

    static void setMenuInfo(const std::string& title, const std::string& subtitle);

    void close() override;

   private:
    static SDLInteraction* s_instance;
    std::string currentTitle_;
    std::string currentSubtitle_;

    SDL_Window* window_;
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    std::string inputBuffer_;
    bool shared_{false};

    void renderInputPrompt(const char* prompt);
    int waitForNumberInput(int minVal, int maxVal);
    int waitForChoice();
};
