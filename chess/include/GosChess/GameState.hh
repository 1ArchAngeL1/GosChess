#ifndef GOSCHESS_GAMESTATE_HH
#define GOSCHESS_GAMESTATE_HH

#include <cstdint>
#include <string>
#include <vector>

#include "Board.hh"
#include "Move.hh"
#include "Rules.hh"

namespace GosChess {

    enum class GameStatus {
        IN_PROGRESS,
        CHECKMATE,                   // side to play is mated, so the other side won
        STALEMATE,
        DRAW_FIFTY_MOVE,
        DRAW_REPETITION,
        DRAW_INSUFFICIENT_MATERIAL
    };

    constexpr bool IsDraw(GameStatus status) {
        return status == GameStatus::STALEMATE ||
               status == GameStatus::DRAW_FIFTY_MOVE ||
               status == GameStatus::DRAW_REPETITION ||
               status == GameStatus::DRAW_INSUFFICIENT_MATERIAL;
    }

    const char *Describe(GameStatus status);

    // One game of chess: the position, whose turn it is, the moves that side
    // may play, and enough history to know when the game has drawn itself.
    // Owning this as an object rather than a set of globals is what allows a
    // single process to hold more than one game.
    class GameState {
    public:
        GameState();

        // Returns to the opening position with White to play.
        void reset();

        // Sets up an arbitrary position. Returns false when the FEN cannot be
        // read, leaving the game untouched.
        bool setFen(const std::string &fen);

        // The authoritative move entry point: rejects moves that are out of
        // range, are not the mover's, are not legal, or leave the mover in
        // check. On success the turn passes, the notation is recorded and the
        // status is recomputed.
        bool applyMove(action_t move);

        [[nodiscard]] color_t colorToPlay() const { return color_to_play_; }

        [[nodiscard]] const board_t &board() const { return board_; }

        [[nodiscard]] board_t &board() { return board_; }

        // Pseudo-legal moves: the illegal ones are weeded out by MakeMove.
        [[nodiscard]] const action_map_t &availableMoves() const { return available_moves_; }

        // The pseudo-legal moves from `square`, or an empty set. Never
        // inserts, so it is safe on a const game.
        [[nodiscard]] const actions_set_t &movesFrom(int square) const;

        // The moves from `square` that can actually be played. This is what a
        // board should highlight - offering a move that will be refused is
        // worse than not offering it.
        [[nodiscard]] actions_set_t legalMovesFrom(int square);

        [[nodiscard]] GameStatus status() const { return status_; }

        [[nodiscard]] bool finished() const { return status_ != GameStatus::IN_PROGRESS; }

        [[nodiscard]] bool inCheck() const { return in_check_; }

        [[nodiscard]] std::string fen() const;

        [[nodiscard]] std::uint16_t fullmoveNumber() const { return fullmove_number_; }

        // Half-moves played from the position the game started in.
        [[nodiscard]] std::uint32_t ply() const { return ply_; }

        [[nodiscard]] action_t lastMove() const { return last_move_; }

        // Algebraic notation for the last move, empty before the first one.
        [[nodiscard]] const std::string &lastMoveNotation() const { return last_move_notation_; }

        // Every move so far in algebraic notation.
        [[nodiscard]] const std::vector<std::string> &history() const { return history_; }

        // Replaces the recorded move list outright. For a mirror of someone
        // else's game - a client following a server - where the authority's
        // record is the truth and the local one may hold a move that was
        // refused. Does not touch the position.
        void replaceHistory(std::vector<std::string> moves) { history_ = std::move(moves); }

    private:
        void refresh();

        void rebuildFrom(const fen_t &parsed);

        board_t board_;
        color_t color_to_play_ = color_t::WHITE;
        action_map_t available_moves_;
        GameStatus status_ = GameStatus::IN_PROGRESS;
        bool in_check_ = false;
        std::uint16_t fullmove_number_ = 1;
        std::uint32_t ply_ = 0;
        action_t last_move_;
        std::string last_move_notation_;
        std::vector<std::string> history_;
        // One entry per position seen, including the starting one, so a third
        // occurrence can be spotted.
        std::vector<std::uint64_t> position_keys_;
    };

}

#endif
