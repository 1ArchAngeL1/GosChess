#ifndef GOSCHESS_RULES_HH
#define GOSCHESS_RULES_HH

#include <vector>

#include "Board.hh"
#include "Move.hh"

// The chess rules, with no global game state of its own: every entry point
// takes the position it should work on. That is what lets one process (the
// server) run many games at once, and what lets these functions be unit
// tested without a window.

namespace GosChess {

    // Generates every pseudo-legal move for `color` into `out`, keyed by origin
    // square. Moves that would leave the mover's own king in check are *not*
    // filtered here - MakeMove rejects them. Castling is the exception: it is
    // only generated when it is fully legal, because the rule about passing
    // through check cannot be expressed as "try it and see".
    //
    // Takes the whole board rather than the bare squares, because castling
    // rights and the en passant square are as much a part of the position as
    // the pieces are.
    void CalculateAvailableMoves(const board_t &board, color_t color, action_map_t &out);

    // The same moves as a flat list. This is the one a search or a node count
    // should use: the map form allocates a hash table and a hash set per call,
    // which is fine once a turn for a user interface and ruinous a million
    // times over inside a search.
    void GenerateMoveList(const board_t &board, color_t color, std::vector<action_t> &out);

    // The same, with the illegal ones removed. Slower, and what anything that
    // counts or names moves should use.
    void CalculateLegalMoves(board_t &board, color_t color, action_map_t &out);

    bool CanMakeMove(action_t move, const actions_set_t &available_moves);

    // Applies `move` if it is present in `available_moves` and does not leave
    // `color`'s king in check. Pushes an undo snapshot on success.
    bool MakeMove(action_t move, board_t &board, color_t color, const actions_set_t &available_moves);

    // Plays a move already known to have been generated for this position, and
    // takes it back if it turns out to leave the mover's own king in check.
    // Returns false in that case, with the board exactly as it was.
    bool TryMove(action_t move, board_t &board, color_t color);

    bool UndoMove(board_t &board);

    bool IsKingInCheck(const board_t &board, color_t color);

    // Where `color`'s king stands, or -1 if it is not on the board. Exposed
    // because a board that wants to mark a king in check needs to know which
    // square to mark.
    int KingSquare(const board_t &board, color_t color);

    // True when a piece of `defender` standing on `square` would be attacked.
    // The defender's colour is a parameter rather than being read off the
    // board, because the square in question is often empty - a castling
    // transit square, or somewhere a piece is thinking of going.
    bool IsSquareAttacked(const unsigned char *board, int square, color_t defender);

    // True when `color` has no move that leaves its own king safe. Combined
    // with IsKingInCheck this distinguishes mate from stalemate.
    bool HasNoLegalMoves(board_t &board, color_t color, const action_map_t &available_moves);

    bool IsCheckmate(board_t &board, color_t color, const action_map_t &available_moves);

    bool IsStalemate(board_t &board, color_t color, const action_map_t &available_moves);

    // True when neither side has the material to force a mate, so the game is
    // drawn whatever either player does.
    bool IsInsufficientMaterial(const board_t &board);

    // Where the rook starts and ends for a castling move by `color`.
    struct castling_rook_move_t {
        int from = -1;
        int to = -1;
    };

    // Returns both squares as -1 when the move is not a castle.
    castling_rook_move_t CastlingRookMove(const board_t &board, action_t move);

    bool IsEnPassantCapture(const board_t &board, action_t move);

    // Counts the leaf nodes of the legal move tree to the given depth. The
    // standard way to prove a move generator correct: the numbers for known
    // positions are published, and an engine that matches them is not quietly
    // missing castling, en passant or a promotion.
    std::uint64_t Perft(board_t &board, color_t color, int depth);

}

#endif
