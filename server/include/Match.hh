#ifndef GOSCHESS_MATCH_HH
#define GOSCHESS_MATCH_HH

#include <cstdint>

#include "GosChess/GameState.hh"
#include "GosChess/Protocol.hh"

namespace GosChess {

    // One game of chess as the server sees it: the only copy of the position
    // that counts, plus the two clocks.
    //
    // Deliberately knows nothing about sockets. The socket loop decides who to
    // tell; this decides what is true. That split is what lets a game be driven
    // to mate, to stalemate and to a flag fall in a test with no network in
    // sight - and it is the reason the clocks can finally be trusted, since
    // neither player's machine is holding them.
    class Match {
    public:
        Match(std::uint32_t id, std::uint16_t time_limit_minutes);

        struct MoveResult {
            bool accepted = false;
            // Meaningful only when the move was not accepted.
            Protocol::RejectReason reject_reason = Protocol::RejectReason::ILLEGAL_MOVE;
        };

        [[nodiscard]] std::uint32_t id() const { return id_; }

        // How many moves have been played. A move request has to name the ply
        // it was made for, so a duplicated or delayed packet cannot be applied
        // a second time.
        [[nodiscard]] std::uint32_t ply() const { return ply_; }

        [[nodiscard]] color_t colorToPlay() const { return game_.colorToPlay(); }

        [[nodiscard]] bool finished() const { return finished_; }

        [[nodiscard]] std::uint16_t timeLimitMinutes() const { return time_limit_minutes_; }

        [[nodiscard]] bool hasClock() const { return has_clock_; }

        // Starts the game from a given position rather than from the opening.
        // This is how a saved or adjourned game is resumed - something only an
        // authoritative server can offer, since it is the one that has the
        // position in the first place. Resets the ply count.
        void setPosition(const std::string &fen, color_t color_to_play);

        // Validates and plays a move on behalf of `mover`.
        MoveResult applyMove(color_t mover, const Protocol::MoveRequest &request);

        // Charges elapsed wall time to whichever side is on the move. Returns
        // true when that charge is what ended the game.
        bool chargeTime(std::uint32_t elapsed_millis);

        void resign(color_t who);

        void abandon(color_t who_left);

        [[nodiscard]] Protocol::GameStateUpdate state() const;

        // Meaningful only once finished() is true.
        [[nodiscard]] Protocol::GameOver result() const;

    private:
        void finish(Protocol::GameOutcome outcome, Protocol::GameOverReason reason);

        void finishIfPositionIsTerminal();

        [[nodiscard]] std::uint32_t &clockFor(color_t color);

        std::uint32_t id_;
        std::uint16_t time_limit_minutes_;
        bool has_clock_;
        GameState game_;
        std::uint32_t ply_ = 0;
        std::int8_t last_move_from_ = -1;
        std::int8_t last_move_to_ = -1;
        std::uint32_t white_millis_ = 0;
        std::uint32_t black_millis_ = 0;
        bool finished_ = false;
        Protocol::GameOutcome outcome_ = Protocol::GameOutcome::DRAW;
        Protocol::GameOverReason reason_ = Protocol::GameOverReason::CHECKMATE;
    };

}

#endif
