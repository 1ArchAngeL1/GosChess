
#ifndef PROJECT_NAME_GAMEDRAW_H
#define PROJECT_NAME_GAMEDRAW_H

#include "../global/GameObjects.h"
#include "SFML/Graphics.hpp"
#include "../global/GameGlobals.h"
#include "../computation/GamePlayFunctional.h"


namespace GosChess {

    void DrawingConfig();

    void ColoriseAvailableMoves(const int &index);

    void DrawBoard(sf::RenderWindow &game_window);

    void LoadChessFigureSprites();

    void DrawFigure(unsigned char figure_type, sf::Vector2f figure_pos, sf::RenderWindow &game_window);

    void DrawCurrentBoardState(const unsigned char *board, sf::RenderWindow &game_window);

    void ResetBoardColours();

}

#endif
