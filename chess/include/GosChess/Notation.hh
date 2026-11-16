#ifndef GOSCHESS_NOTATION_HH
#define GOSCHESS_NOTATION_HH

#include <string>

#include "Board.hh"
#include "Move.hh"

namespace GosChess {

    // Standard algebraic notation for `move`, played by `color` in the
    // position the board is in *before* the move - "Nf3", "exd5", "O-O",
    // "e8=Q", "Qxf7#".
    //
    // Produced by whoever is the authority for the position, which is why it
    // lives next to the rules rather than in the client: the piece letter,
    // the disambiguation between two knights that can both reach a square,
    // and the check or mate mark at the end are all facts about the position,
    // and a client working them out for itself could get a different answer
    // from its opponent's client.
    std::string ToSan(board_t &board, color_t color, action_t move);

    // "e2e4", or "e7e8q" for a promotion. Compact, unambiguous and free of
    // any dependence on the position, so it is what a log or a bug report
    // should carry.
    std::string ToCoordinateNotation(action_t move);

}

#endif
