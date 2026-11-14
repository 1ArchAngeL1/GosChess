#ifndef GOSCHESS_RULES_HH
#define GOSCHESS_RULES_HH

#include "Board.hh"
#include "Move.hh"

// The chess rules, with no global game state of its own: every entry point
// takes the position it should work on. That is what lets one process (the
// server) run many games at once, and what lets these functions be unit
// tested without a window.

namespace GosChess {

    // Generates every pseudo-legal move for `color` into `out`, keyed by origin
    // square. Moves that would leave the mover's own king in check are *not*
    // filtered here - MakeMove rejects them.
    void CalculateAvailableMoves(const unsigned char *board, color_t color, action_map_t &out);

    bool CanMakeMove(action_t move, const actions_set_t &available_moves);

    // Applies `move` if it is present in `available_moves` and does not leave
    // `color`'s king in check. Pushes an undo snapshot on success.
    bool MakeMove(action_t move, board_t &board, color_t color, const actions_set_t &available_moves);

    // Applies `move` without any validation. Only for replaying a move that has
    // already been validated by the authority.
    void MakeMoveForce(action_t move, board_t &board);

    bool UndoMove(board_t &board);

    bool IsKingInCheck(const unsigned char *board, color_t color);

    // True when `color` has no move that leaves its own king safe. Combined
    // with IsKingInCheck this distinguishes mate from stalemate.
    bool HasNoLegalMoves(board_t &board, color_t color, const action_map_t &available_moves);

    bool IsCheckmate(board_t &board, color_t color, const action_map_t &available_moves);

    bool IsStalemate(board_t &board, color_t color, const action_map_t &available_moves);

    // True when the piece standing on `index` is attacked by the opposing side.
    bool IsSquareAttacked(const unsigned char *board, const int &index);

    // Promotes any pawn that has reached its far rank to a queen.
    void PromotePawns(board_t &board);

}

#endif
