//
// Created by Leo Gogichaishvili on 14.02.22.
//

#ifndef GOSCHESS_AICALCULATIONS_H
#define GOSCHESS_AICALCULATIONS_H

#include "../../global/MainObjects/GameObjects.h"
#include "../computation_globals/ComputationGlobals.h"
#include <unordered_map>

namespace GosChess {


    class IChessAi {
    private:

    public:
        virtual GosChess::Move GetBestMove(GosChess::Board) = 0;
    };

    //AI using mini-max and alpha beta prun
    class BasicChessAi : public IChessAi {
    private:
    public:
        GosChess::Move GetBestMove(GosChess::Board) override;
    };

    int CountScore(const unsigned char *, GosChess::Color);

    std::vector<GosChess::Move> MergeColorMoves(GosChess::Color);

    int Evaluate(const unsigned char *);

    int Search(GosChess::Board &board, int depth, GosChess::Color);


}


#endif //GOSCHESS_AICALCULATIONS_H
