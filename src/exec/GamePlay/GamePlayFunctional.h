//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
#define PROJECT_NAME_GAMEPLAYFUNCTIONAL_H

#include <iostream>
#include <SFML/Graphics.hpp>
#include "../../global/GameObjects.h"
#include "../../global/GameGlobals.h"
#include "../../render/GameDraw.h"
#include "../../computation/InGameCalculations.h"
#include "../../util/GameInput.h"
#include <optional>

namespace GosChess {

    bool Play(GosChess::Board & brd,const GosChess::Move &move);

    std::optional<GosChess::Cell> ChooseTrgFigure(GosChess::Board &brd, sf::Window &window);

    std::optional<GosChess::Cell> ChooseSrcFigure(GosChess::Board &brd, sf::Window &window);

    extern bool highlited;

}


#endif //PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
