
#ifndef GOSCHESS_INGAMECALCULATIONS_H
#define GOSCHESS_INGAMECALCULATIONS_H

#include "../global/MainObjects/GameObjects.h"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "../global/GameGlobals.h"
#include "computation_globals/ComputationGlobals.h"

namespace GosChess {

    GosChess::Vector2i GetSquare(const int &);

    GosChess::Vector2i GetNodeFromScreen(const float &, const float &);

    GosChess::Move InvertMove(GosChess::Move);

    int GetNumFromNode(const GosChess::Vector2i &);

    bool MakeMove(GosChess::Move, GosChess::Board &);

    bool UndoMove(GosChess::Board &);

    bool CanMakeMove(GosChess::Move);

    bool CheckMate(GosChess::Board &, GosChess::Color);

    bool CheckForDraw(GosChess::Board &, GosChess::Color);

    bool CheckIndexForAttackers(const unsigned char *, const int &);

    void GenerateOffsets();

    void CalculateAvailableMoves(const unsigned char *, GosChess::Color);

    void ChangeActiveColour(GosChess::Board &);

    void MakeMoveForce(GosChess::Move, GosChess::Board &);

}


#endif
