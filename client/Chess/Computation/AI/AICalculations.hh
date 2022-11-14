//
// Created by Leo Gogichaishvili on 14.02.22.
//

#ifndef GOSCHESS_AICALCULATIONS_HH
#define GOSCHESS_AICALCULATIONS_HH

#include "../../Global/MainOjbects/GameObjects.hh"
#include "../ComputationGlobals/ComputationGlobals.hh"
#include <unordered_map>

namespace GosChess {

    action_t GetBestMove(GosChess::board_t board);

    extern int ai_level;
}


#endif //GOSCHESS_AICALCULATIONS_HH
