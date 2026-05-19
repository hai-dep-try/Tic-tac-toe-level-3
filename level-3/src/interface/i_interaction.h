/**
 * I Interaction header file
 *
 * Mô tả:
 *   Interface cho input layer. Giữ tinh thần level 2.
 *   Tầng này thuộc `imperative shell` -- được phép có side-effect IO.
 */

#pragma once

/* ---------- Importing ---------- */

#include "../core/types.h"
#include "../utils/config.h"

/* ---------- Declarations ---------- */

class I_Interaction {
   public:
    I_Interaction();
    virtual ~I_Interaction();

    virtual void init(const RunConfig& config) = 0;
    virtual void pause(int timeout = 0) = 0;

    virtual bool selectSize(int* size) = 0;
    virtual bool selectGoal(int* goal, int size) = 0;
    virtual bool selectGameMode(GameMode* mode) = 0;
    virtual bool selectBotLevel(BotLevel* levels, int index) = 0;

    virtual bool getPlayerMove(int* row, int* col) = 0;

    virtual void close() = 0;
};
