//
// Created by Leo Gogichaishvili on 14.02.22.
//

#ifndef GOSCHESS_AICALCULATIONS_HH
#define GOSCHESS_AICALCULATIONS_HH

#include "GosChess/Board.hh"
#include "GosChess/Move.hh"

namespace GosChess {

    // The best move the engine can find for `ai_color` in `board`. The colour
    // is a parameter rather than a global because the board no longer has a
    // near side and a far side: the search has to be told which army it is
    // playing. The board is taken by value because the search mutates it.
    action_t GetBestMove(board_t board, color_t ai_color);

    extern int ai_level;
}


#endif //GOSCHESS_AICALCULATIONS_HH
