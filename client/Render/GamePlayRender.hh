
#ifndef GOSCHESS_GAMEPLAYRENDER_HH
#define GOSCHESS_GAMEPLAYRENDER_HH

#include "../chess/global/MainOjbects/GameObjects.hh"
#include "SFML/Graphics.hpp"
#include "../Chess/Global/GameGlobals.hh"
#include "../Chess/Computation/InGameCalculations.hh"


namespace GosChess {

    void ChessDrawingConfig();

    void BoardRenderConfig();

    void ColorizeAvailableMoves(const int &, board_t &);

    void DrawBoard(sf::RenderWindow &);

    void DrawTimer(sf::RenderWindow &, const std::string &, const std::string &);

    void LoadChessFigureSprites();

    void DrawFigure(unsigned char, sf::Vector2f, sf::RenderWindow &);

    void DrawCurrentBoardState(const unsigned char *, sf::RenderWindow &, const std::string &, const std::string &);

    void ResetBoardColours();

}

#endif
