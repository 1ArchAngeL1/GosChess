
#ifndef GOSCHESS_INGAMECALCULATIONS_H
#define GOSCHESS_INGAMECALCULATIONS_H

#include "../global/main_objects/GameObjects.h"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "../global/GameGlobals.h"
#include "computation_globals/ComputationGlobals.h"

namespace GosChess {

    GosChess::board_square GetSquare(const int &);

    GosChess::board_square GetNodeFromScreen(const float &, const float &);

    GosChess::Move InvertMove(GosChess::Move);

    int GetNumFromNode(const GosChess::board_square &);

    bool MakeMove(GosChess::Move, GosChess::Board &, GosChess::Color);

    bool UndoMove(GosChess::Board &);

//    template<class It>
//    typename std::iterator_traits<It>::value_type CanMakeMove(GosChess::Move move, It begin, It end) {
//        while (begin != end) {
//            if (*begin == move)return true;
//        }
//        return false;
//    }
    bool CanMakeMove(GosChess::Move);

    bool CheckMate(GosChess::Board &, GosChess::Color);

    bool CheckForDraw(GosChess::Board &, GosChess::Color);

    bool CheckIndexForAttackers(const unsigned char *, const int &);

    void GenerateOffsets();

    void CalculateAvailableMoves(const unsigned char *, GosChess::Color);

    void ChangeActiveColour(GosChess::Board &);

    void MakeMoveForce(GosChess::Move, GosChess::Board &);

    void PromotePawns(GosChess::Board &);
}


#endif
