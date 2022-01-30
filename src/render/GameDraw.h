
#ifndef PROJECT_NAME_GAMEDRAW_H
#define PROJECT_NAME_GAMEDRAW_H

#include "../chess_api/global/MainObjects/GameObjects.h"
#include "SFML/Graphics.hpp"
#include "../chess_api/global/GameGlobals.h"
#include "../chess_api/computation/InGameCalculations.h"


namespace GosChess {

    void DrawingConfig();

    void ColoriseAvailableMoves(const int &);

    void DrawBoard(sf::RenderWindow &);

    void LoadChessFigureSprites();

    void DrawFigure(unsigned char, sf::Vector2f, sf::RenderWindow &);

    void DrawCurrentBoardState(const unsigned char *, sf::RenderWindow &);

    void ResetBoardColours();

}

#endif
