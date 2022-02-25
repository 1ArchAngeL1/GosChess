//
// Created by Leo Gogichaishvili on 14.02.22.
//

#ifndef GOSCHESS_AICALCULATIONS_H
#define GOSCHESS_AICALCULATIONS_H

#include "../../global/main_objects/GameObjects.h"
#include "../computation_globals/ComputationGlobals.h"
#include <unordered_map>

namespace GosChess {

    Move GetBestMove(GosChess::Board board);
}


#endif //GOSCHESS_AICALCULATIONS_H
