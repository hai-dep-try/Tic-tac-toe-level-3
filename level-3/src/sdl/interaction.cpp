/**
 * SDL Interaction implementation
 */

#include "interaction.h"

#include <iostream>
#include <sstream>

namespace {

SDL_Color COLOR_BG       = {30, 30, 40, 255};
SDL_Color COLOR_TEXT     = {240, 240, 240, 255};
SDL_Color COLOR_INPUT    = {255, 255, 200, 255};
SDL_Color COLOR_CURSOR   = {255, 255, 255, 255};

}  // namespace

SDLInteraction* SDLInteraction::s_instance = nullptr;

SDLInteraction::SDLInteraction()
    : window_(nullptr), renderer_(nullptr), font_(nullptr) {
    s_instance = this;
}

void SDLInteraction::setMenuInfo(const std::string& title, const std::string& subtitle) {
    if (s_instance) {
        s_instance->currentTitle_ = title;
        s_instance->currentSubtitle_ = subtitle;
    }
}

SDLInteraction::~SDLInteraction() {
    close();
}

void SDLInteraction::init(const RunConfig& config) {
    if (shared_) {
        // We already have a shared window and renderer, just load the font!
        const char* fontPaths[] = {
            "assets/fonts/arial.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
            "C:/Windows/Fonts/consola.ttf",
        };
        for (const char* path : fontPaths) {
            font_ = TTF_OpenFont(path, 28);
            if (font_) break;
        }
        return;
    }

    // SDLInteraction shares the same SDL window/renderer as SDLRenderer
    // In practice, we create our own minimal window for input
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
        return;
    }
    if (TTF_Init() < 0) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        return;
    }

    window_ = SDL_CreateWindow("Tic-Tac-Toe - Input",
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

    const char* fontPaths[] = {
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/consola.ttf",
    };
    for (const char* path : fontPaths) {
        font_ = TTF_OpenFont(path, 28);
        if (font_) break;
    }
}

void SDLInteraction::pause(int timeout) {
    if (!renderer_) return;

    SDL_RenderPresent(renderer_);

    if (timeout > 0) {
        Uint32 start = SDL_GetTicks();
        SDL_Event e;
        while (SDL_GetTicks() - start < static_cast<Uint32>(timeout)) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_QUIT) {
                    return;
                }
                if (e.type == SDL_WINDOWEVENT) {
                    if (e.window.event == SDL_WINDOWEVENT_EXPOSED ||
                        e.window.event == SDL_WINDOWEVENT_SHOWN ||
                        e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                        SDL_RenderPresent(renderer_);
                    }
                }
            }
            SDL_Delay(10);
        }
    } else {
        // Flush any buffered old mouse/keyboard events before waiting for a new click/keypress
        SDL_Event flushEvent;
        while (SDL_PollEvent(&flushEvent)) {
            if (flushEvent.type == SDL_QUIT) return;
        }

        // Wait for any key or mouse click
        SDL_Event e;
        while (true) {
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) {
                    return;
                }
                if (e.type == SDL_QUIT) return;
                if (e.type == SDL_WINDOWEVENT) {
                    if (e.window.event == SDL_WINDOWEVENT_EXPOSED ||
                        e.window.event == SDL_WINDOWEVENT_SHOWN ||
                        e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                        SDL_RenderPresent(renderer_);
                    }
                }
            }
            SDL_Delay(10);
        }
    }
}

bool SDLInteraction::selectSize(int* size) {
    int val = waitForNumberInput(BOARD_N_MIN, BOARD_N_MAX);
    if (val >= 0) {
        *size = val;
        return true;
    }
    return false;
}

bool SDLInteraction::selectGoal(int* goal, int size) {
    int maxGoal = std::min(size, GOAL_MAX);
    int val = waitForNumberInput(3, maxGoal);
    if (val >= 0) {
        *goal = val;
        return true;
    }
    return false;
}

bool SDLInteraction::selectGameMode(GameMode* mode) {
    int val = waitForChoice();
    if (val < 0) return false;

    switch (val) {
        case 1: *mode = GameMode::PVP; return true;
        case 2: *mode = GameMode::PVE; return true;
        case 3: *mode = GameMode::EVE; return true;
        default: return false;
    }
}

bool SDLInteraction::selectBotLevel(BotLevel* levels, int index) {
    int val = waitForChoice();
    if (val < 0) return false;

    switch (val) {
        case 1: levels[index] = BotLevel::EASY;   return true;
        case 2: levels[index] = BotLevel::MEDIUM;  return true;
        case 3: levels[index] = BotLevel::HARD;    return true;
        default: return false;
    }
}

bool SDLInteraction::getPlayerMove(int* row, int* col) {
    if (!renderer_) return false;

    // Flush any buffered old mouse/keyboard events
    SDL_Event flushEvent;
    while (SDL_PollEvent(&flushEvent)) {}

    inputBuffer_.clear();
    renderInputPrompt("Enter row and col (e.g. 1 2) or ESC to quit: ");

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return false;
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_EXPOSED ||
                    e.window.event == SDL_WINDOWEVENT_SHOWN ||
                    e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    renderInputPrompt("Enter row and col (e.g. 1 2) or ESC to quit: ");
                }
            }

            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;

                if (key == SDLK_ESCAPE) return false;
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    // Parse input
                    std::istringstream iss(inputBuffer_);
                    int r, c;
                    if (iss >> r >> c) {
                        *row = r;
                        *col = c;
                        return true;
                    }
                    inputBuffer_.clear();
                    renderInputPrompt("Invalid format. Enter row and col (e.g. 1 2): ");
                    continue;
                }
                if (key == SDLK_BACKSPACE) {
                    if (!inputBuffer_.empty()) {
                        inputBuffer_.pop_back();
                    }
                } else if (key >= SDLK_SPACE && key <= SDLK_BACKQUOTE) {
                    inputBuffer_ += static_cast<char>(key);
                } else if (key >= SDLK_KP_0 && key <= SDLK_KP_9) {
                    inputBuffer_ += static_cast<char>('0' + (key - SDLK_KP_0));
                } else if (key == SDLK_KP_MINUS || key == SDLK_MINUS) {
                    inputBuffer_ += '-';
                }

                renderInputPrompt("Enter row and col (e.g. 1 2) or ESC to quit: ");
            }
        }
        SDL_Delay(10);
    }
}

void SDLInteraction::close() {
    if (font_) {
        TTF_CloseFont(font_);
        font_ = nullptr;
    }
    if (!shared_) {
        if (renderer_) SDL_DestroyRenderer(renderer_);
        if (window_) SDL_DestroyWindow(window_);
        TTF_Quit();
        SDL_Quit();
    }

    window_ = nullptr;
    renderer_ = nullptr;
}

void SDLInteraction::renderInputPrompt(const char* prompt) {
    if (!renderer_ || !font_) return;

    SDL_SetRenderDrawColor(renderer_, COLOR_BG.r, COLOR_BG.g, COLOR_BG.b, COLOR_BG.a);
    SDL_RenderClear(renderer_);

    // Draw the active menu title if set
    if (!currentTitle_.empty()) {
        SDL_Surface* titleSurf = TTF_RenderText_Solid(font_, currentTitle_.c_str(), {255, 220, 100, 255}); // COLOR_TITLE
        if (titleSurf) {
            SDL_Texture* titleTex = SDL_CreateTextureFromSurface(renderer_, titleSurf);
            SDL_Rect titleDst = {150, 150, titleSurf->w, titleSurf->h};
            SDL_RenderCopy(renderer_, titleTex, nullptr, &titleDst);
            SDL_DestroyTexture(titleTex);
            SDL_FreeSurface(titleSurf);
        }
    }

    // Draw the active menu subtitle/options if set
    if (!currentSubtitle_.empty()) {
        std::istringstream iss(currentSubtitle_);
        std::string line;
        int currentY = 220;
        while (std::getline(iss, line, '\n')) {
            SDL_Surface* subSurf = TTF_RenderText_Solid(font_, line.c_str(), COLOR_TEXT);
            if (subSurf) {
                SDL_Texture* subTex = SDL_CreateTextureFromSurface(renderer_, subSurf);
                SDL_Rect subDst = {150, currentY, subSurf->w, subSurf->h};
                SDL_RenderCopy(renderer_, subTex, nullptr, &subDst);
                SDL_DestroyTexture(subTex);
                SDL_FreeSurface(subSurf);
            }
            currentY += 40;
        }
    }

    // Render prompt text (drawn lower down to avoid overlapping the menu options)
    SDL_Surface* surface = TTF_RenderText_Solid(font_, prompt, COLOR_TEXT);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_Rect dst = {150, 420, surface->w, surface->h};
        SDL_RenderCopy(renderer_, texture, nullptr, &dst);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    // Render input buffer
    if (!inputBuffer_.empty()) {
        SDL_Surface* inputSurface = TTF_RenderText_Solid(font_, inputBuffer_.c_str(), COLOR_INPUT);
        if (inputSurface) {
            SDL_Texture* inputTexture = SDL_CreateTextureFromSurface(renderer_, inputSurface);
            SDL_Rect inputDst = {150, 480, inputSurface->w, inputSurface->h};
            SDL_RenderCopy(renderer_, inputTexture, nullptr, &inputDst);
            SDL_DestroyTexture(inputTexture);
            SDL_FreeSurface(inputSurface);
        }
    }

    // Draw cursor
    SDL_SetRenderDrawColor(renderer_, COLOR_CURSOR.r, COLOR_CURSOR.g, COLOR_CURSOR.b, COLOR_CURSOR.a);
    int cursorX = 150 + (int)inputBuffer_.length() * 14;
    SDL_RenderDrawLine(renderer_, cursorX, 480, cursorX, 510);

    SDL_RenderPresent(renderer_);
}

int SDLInteraction::waitForNumberInput(int minVal, int maxVal) {
    if (!renderer_) return -1;

    // Flush any buffered old mouse/keyboard events
    SDL_Event flushEvent;
    while (SDL_PollEvent(&flushEvent)) {}

    inputBuffer_.clear();
    std::string prompt = "Enter number (" + std::to_string(minVal) + "-" + std::to_string(maxVal) + ") or ESC to quit: ";
    renderInputPrompt(prompt.c_str());

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return -1;
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_EXPOSED ||
                    e.window.event == SDL_WINDOWEVENT_SHOWN ||
                    e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    renderInputPrompt(prompt.c_str());
                }
            }

            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;

                if (key == SDLK_ESCAPE) return -1;
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    if (!inputBuffer_.empty()) {
                        try {
                            int val = std::stoi(inputBuffer_);
                            if (val >= minVal && val <= maxVal) {
                                return val;
                            }
                        } catch (...) {}
                    }
                    inputBuffer_.clear();
                    renderInputPrompt(prompt.c_str());
                    continue;
                }
                if (key == SDLK_BACKSPACE) {
                    if (!inputBuffer_.empty()) {
                        inputBuffer_.pop_back();
                    }
                } else if ((key >= SDLK_0 && key <= SDLK_9) ||
                           (key >= SDLK_KP_0 && key <= SDLK_KP_9)) {
                    char digit = (key >= SDLK_0 && key <= SDLK_9)
                                     ? static_cast<char>(key)
                                     : static_cast<char>('0' + (key - SDLK_KP_0));
                    inputBuffer_ += digit;
                } else if (key == SDLK_MINUS) {
                    if (inputBuffer_.empty()) {
                        inputBuffer_ += '-';
                    }
                }

                renderInputPrompt(prompt.c_str());
            }
        }
        SDL_Delay(10);
    }
}

int SDLInteraction::waitForChoice() {
    if (!renderer_) return -1;

    // Flush any buffered old mouse/keyboard events
    SDL_Event flushEvent;
    while (SDL_PollEvent(&flushEvent)) {}

    inputBuffer_.clear();
    renderInputPrompt("Enter choice (1-3) or ESC to quit: ");

    SDL_Event e;
    while (true) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) return -1;
            if (e.type == SDL_WINDOWEVENT) {
                if (e.window.event == SDL_WINDOWEVENT_EXPOSED ||
                    e.window.event == SDL_WINDOWEVENT_SHOWN ||
                    e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                    renderInputPrompt("Enter choice (1-3) or ESC to quit: ");
                }
            }

            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;

                if (key == SDLK_ESCAPE) return -1;
                if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                    if (!inputBuffer_.empty()) {
                        try {
                            int val = std::stoi(inputBuffer_);
                            if (val >= 1 && val <= 3) {
                                return val;
                            }
                        } catch (...) {}
                    }
                    inputBuffer_.clear();
                    renderInputPrompt("Invalid. Enter choice (1-3) or ESC to quit: ");
                    continue;
                }
                if (key == SDLK_BACKSPACE) {
                    if (!inputBuffer_.empty()) {
                        inputBuffer_.pop_back();
                    }
                } else if ((key >= SDLK_1 && key <= SDLK_3) ||
                           (key >= SDLK_KP_1 && key <= SDLK_KP_3)) {
                    char digit = (key >= SDLK_1 && key <= SDLK_3)
                                     ? static_cast<char>(key)
                                     : static_cast<char>('1' + (key - SDLK_KP_1));
                    inputBuffer_ = digit;  // Only accept single digit
                    renderInputPrompt("Enter choice (1-3) or ESC to quit: ");
                }
            }
        }
        SDL_Delay(10);
    }
}
