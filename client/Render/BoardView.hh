#ifndef GOSCHESS_BOARDVIEW_HH
#define GOSCHESS_BOARDVIEW_HH

#include "GosChess/Board.hh"

// Where a square sits on the drawn board.
//
// Squares are stored white-side-up (see GosChess/Board.hh); a player sitting
// on the black side sees that same board rotated by 180 degrees. That rotation
// lives here and nowhere else, so the rules, the wire format and the two
// clients all agree on what square 0 is while each player still looks at the
// board from their own chair.
//
// Deliberately free of SFML: this is arithmetic, it is the part that is easy
// to get backwards, and keeping it out of the rendering code is what lets it
// be checked without opening a window.

namespace GosChess {

    // A cell of the drawn board, counted from its top left corner:
    // y_ is rows downwards, x_ is columns rightwards.
    typedef board_node_t view_cell_t;

    bool BoardIsFlipped();

    view_cell_t BoardIndexToViewCell(int board_index);

    int ViewCellToBoardIndex(const view_cell_t &cell);

    // Clicks land anywhere in the window, including beside the board, so a
    // cell has to be range checked before it is turned into a square index.
    bool ViewCellOnBoard(const view_cell_t &cell);

}

#endif
