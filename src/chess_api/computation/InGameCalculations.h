
#ifndef PROJECT_NAME_INGAMECALCULATIONS_H
#define PROJECT_NAME_INGAMECALCULATIONS_H

#include "../global/MainObjects/GameObjects.h"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "../global/GameGlobals.h"
#include "globals/ComputationGlobals.h"


namespace GosChess {


    GosChess::Cell GetNode(const int &num);

    GosChess::Cell GetNodeFromScreen(const float &_y, const float &_x);

    int GetNumFromNode(const GosChess::Cell &_cell);

    bool MakeMove(GosChess::Move mv, GosChess::Board &brd);

    bool CanMakeMove(GosChess::Move mv);

    bool CheckMate(GosChess::Board &brd, GosChess::Color clr);

    void GenerateOffsets();

    void CalculateAvailableMoves(const unsigned char *game_board);

    void ChangeActiveColour();

}


#endif
