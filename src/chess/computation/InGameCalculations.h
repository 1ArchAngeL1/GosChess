
#ifndef GOSCHESS_INGAMECALCULATIONS_H
#define GOSCHESS_INGAMECALCULATIONS_H

#include "../global/MainObjects/GameObjects.h"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "../global/GameGlobals.h"
#include "computation_globals/ComputationGlobals.h"


namespace GosChess {

    GosChess::Square GetSquare(const int &);

    GosChess::Square GetNodeFromScreen(const float &, const float &);

    int GetNumFromNode(const GosChess::Square &);

    bool MakeMove(GosChess::Move, GosChess::Board &);

    void MakeMoveForce(GosChess::Move, GosChess::Board&);

    bool CanMakeMove(GosChess::Move);

    bool CheckMate(GosChess::Board &, GosChess::Color);

    void GenerateOffsets();

    void CalculateAvailableMoves(const unsigned char *);

    void ChangeActiveColour(GosChess::Board&);

    GosChess::Move InvertMove(GosChess::Move);

}


#endif
