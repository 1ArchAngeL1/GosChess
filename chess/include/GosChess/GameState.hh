#ifndef GOSCHESS_GAMESTATE_HH
#define GOSCHESS_GAMESTATE_HH

#include "Board.hh"
#include "Move.hh"
#include "Rules.hh"

namespace GosChess {

    enum class GameStatus {
        IN_PROGRESS,
        CHECKMATE,   // side to play is mated, so the side that just moved won
        STALEMATE
    };

    // One game of chess: the position, whose turn it is, and the moves that
    // side may play. Owning this as an object rather than a set of globals is
    // what allows a single process to hold more than one game.
    class GameState {
    public:
        GameState();

        // Returns to the opening position with White to play.
        void reset();

        void setPosition(const std::string &fen, color_t color_to_play);

        // The authoritative move entry point: rejects moves that are out of
        // range, are not the mover's, are not legal, or leave the mover in
        // check. On success the turn passes and the status is recomputed.
        bool applyMove(action_t move);

        [[nodiscard]] color_t colorToPlay() const { return color_to_play_; }

        [[nodiscard]] const board_t &board() const { return board_; }

        [[nodiscard]] board_t &board() { return board_; }

        [[nodiscard]] const action_map_t &availableMoves() const { return available_moves_; }

        // The moves originating from `square`, or an empty set if there are
        // none. Never inserts, so it is safe to call on a const game.
        [[nodiscard]] const actions_set_t &movesFrom(int square) const;

        [[nodiscard]] GameStatus status() const { return status_; }

        [[nodiscard]] bool finished() const { return status_ != GameStatus::IN_PROGRESS; }

    private:
        void refresh();

        board_t board_;
        color_t color_to_play_;
        action_map_t available_moves_;
        GameStatus status_;
    };

}

#endif
