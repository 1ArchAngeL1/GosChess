
#ifndef GOSCHESS_GAMEDRAW_H
#define GOSCHESS_GAMEDRAW_H

#include "../chess/global/MainObjects/GameObjects.h"
#include "SFML/Graphics.hpp"
#include "../chess/global/GameGlobals.h"
#include "../chess/computation/InGameCalculations.h"


namespace GosChess {

    void ChessDrawingConfig();

    void ColoriseAvailableMoves(const int &);

    void DrawBoard(sf::RenderWindow &);

    void LoadChessFigureSprites();

    void DrawFigure(unsigned char, sf::Vector2f, sf::RenderWindow &);

    void DrawCurrentBoardState(const unsigned char *, sf::RenderWindow &);

    void ResetBoardColours();

}

#endif
