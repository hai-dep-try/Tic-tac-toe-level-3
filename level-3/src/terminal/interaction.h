/**
 * Terminal Interaction header file
 *
 * Mô tả:
 *   Cài đặt I_Interaction cho terminal. Giữ tinh thần level 2.
 */

#pragma once

/* ---------- Importing ---------- */

#include "../interface/i_interaction.h"

/* ---------- Declarations ---------- */

class TerminalInteraction : public I_Interaction {
   public:
    TerminalInteraction();
    ~TerminalInteraction() override;

    void init(const RunConfig& config) override;
    void pause(int timeout = 0) override;

    bool selectSize(int* size) override;
    bool selectGoal(int* goal, int size) override;
    bool selectGameMode(GameMode* mode) override;
    bool selectBotLevel(BotLevel* levels, int index) override;

    bool getPlayerMove(int* row, int* col) override;

    void close() override;
};
