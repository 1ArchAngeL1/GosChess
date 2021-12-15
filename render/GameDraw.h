
#ifndef PROJECT_NAME_GAMEDRAW_H
#define PROJECT_NAME_GAMEDRAW_H

#include "../computation/GameObjects.h"
#include "SFML/Graphics.hpp"
#include "../global/BoardAtributtes.h"


namespace GosChess {


    void DrawingConfig();

    void DrawBoard(GosChess::Board board, sf::RenderWindow &game_window);

    void DrawFiuge(GosChess::Figure, float x, float y, sf::RenderWindow &game_window);


}

#endif
