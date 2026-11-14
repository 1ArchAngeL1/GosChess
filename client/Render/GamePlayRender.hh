
#ifndef GOSCHESS_GAMEPLAYRENDER_HH
#define GOSCHESS_GAMEPLAYRENDER_HH

#include "GosChess/Board.hh"
#include "GosChess/Rules.hh"
#include "SFML/Graphics.hpp"
#include "Globals/RenderGlobals.hh"


namespace GosChess {

    void ChessDrawingConfig();

    void BoardRenderConfig();

    // Highlights every legal destination of the piece standing on the given
    // square. Reads the current game; the square is a board index, not a cell
    // on screen.
    void ColorizeAvailableMoves(int board_index);

    void DrawBoard(sf::RenderWindow &);

    void DrawTimer(sf::RenderWindow &, const std::string &, const std::string &);

    void LoadChessFigureSprites();

    void DrawFigure(unsigned char, sf::Vector2f, sf::RenderWindow &);

    void DrawCurrentBoardState(const unsigned char *, sf::RenderWindow &, const std::string &, const std::string &);

    void ResetBoardColours();

}

#endif
