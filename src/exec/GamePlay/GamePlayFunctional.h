//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
#define PROJECT_NAME_GAMEPLAYFUNCTIONAL_H

#include <iostream>
#include <SFML/Graphics.hpp>
#include "../../chess_api/global/MainObjects/GameObjects.h"
#include "../../chess_api/global/GameGlobals.h"
#include "../../render/GameDraw.h"
#include "../../chess_api/computation/InGameCalculations.h"
#include "../../util/GameInput.h"
#include <optional>

namespace GosChess {

    std::string GetInitialFenBoard();

    bool Play(GosChess::Board &, const GosChess::Move &);

    std::optional<GosChess::Cell> ChooseTrgFigure(GosChess::Board &, sf::Window &);

    std::optional<GosChess::Cell> ChooseSrcFigure(GosChess::Board &, sf::Window &);

    extern bool highlited;

}


#endif //PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
