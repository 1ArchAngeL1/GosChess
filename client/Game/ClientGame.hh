#ifndef GOSCHESS_CLIENTGAME_HH
#define GOSCHESS_CLIENTGAME_HH

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "GosChess/Board.hh"
#include "GosChess/GameState.hh"
#include "GosChess/Protocol.hh"

namespace GosChess {

    // The client's copy of the game.
    //
    // It is a copy, not the original: in an online game the server holds the
    // position that counts. This one exists so the board can be drawn, so
    // legal destinations can be highlighted without asking anybody, and so a
    // move the player makes appears at once instead of after a round trip.
    // Every state the server sends replaces it wholesale, which is what keeps
    // a prediction from quietly becoming a disagreement.
    extern GameState game;

    // Which side the person at this keyboard plays. A point of view, not a
    // rule - the board is always stored white-side-up regardless.
    extern color_t player_color;

    extern color_t enemy_color;

    // The last ply the server confirmed. A move is sent naming this, so a
    // packet that arrives twice cannot be played twice.
    extern std::uint32_t confirmed_ply;

    // True while a move of ours has been played locally but not yet confirmed.
    extern bool awaiting_confirmation;

    // The move the position came from, for showing where the opponent went.
    // Both -1 at the start of a game.
    extern std::int8_t last_move_from;

    extern std::int8_t last_move_to;

    // True once the game has ended, for any reason.
    //
    // GameState only knows the endings the rules describe - mate and the
    // draws. A resignation, a flag fall or an opponent walking out all end the
    // game without changing its status, so asking game.finished() left the
    // board still offering Resign and the clock still running.
    extern bool game_over;

    void SetGameOver(bool over);

    // --- sound cues --------------------------------------------------------
    //
    // What just happened, for whoever is in a position to make a noise about
    // it. This module deliberately does not play the sound itself: it is the
    // mirror of the game state, it is linked into tests that have no business
    // opening an audio device, and keeping it free of SFML's audio module is
    // what lets those tests stay small.

    enum class SoundCue {
        NONE,
        MOVE,
        CAPTURE,
        CHECK,
        GAME_OVER
    };

    extern SoundCue pending_sound;

    // --- move animation ----------------------------------------------------
    //
    // A piece that teleports between squares is hard to follow, and in an
    // online game the opponent's move simply appears with nothing to say where
    // it came from. The piece slides instead, and the board draws it in flight
    // rather than on its destination until it arrives.

    struct move_animation_t {
        bool active = false;
        int from = -1;
        int to = -1;
        unsigned char piece = 0;
        // A castling rook travels with its king, so two pieces move at once.
        int rook_from = -1;
        int rook_to = -1;
        unsigned char rook_piece = 0;
        float elapsed = 0.f;
        float duration = 0.f;
    };

    extern move_animation_t move_animation;

    // Called after a move has been applied: the piece is read off its
    // destination square.
    void StartMoveAnimation(int from, int to);

    void TickMoveAnimation(float delta_seconds);

    // How far along, eased, in 0..1.
    float MoveAnimationProgress();

    void SetPlayerColor(color_t color);

    bool PlayerToMove();

    // Replaces the local position with the server's, whatever ours said.
    void AdoptServerState(const Protocol::GameStateUpdate &update);

    // Plays a move locally for immediate feedback, without claiming it is
    // final. Returns false if our own copy of the rules already refuses it, in
    // which case there is no point troubling the server.
    bool PredictMove(action_t move);

    // Plays a move with this client as the authority, for the game against the
    // computer where there is no server to ask.
    bool PlayLocalMove(action_t move);

    void ResetClientGame();

    // The move list as it should be read: one row per full move, White's then
    // Black's. The server sends a flat list of plies, and turning it into
    // numbered rows is presentation, so it happens here.
    struct history_row_t {
        int move_number = 0;
        std::string white;
        std::string black;
    };

    std::vector<history_row_t> HistoryRows();

    // --- promotion ---------------------------------------------------------
    //
    // A pawn reaching the far rank is four different moves, and the player has
    // to be asked which. The move is held here between the click that earns
    // the promotion and the click that chooses the piece.

    // The move awaiting a choice of piece, if any. Its promotion field is 0.
    extern std::optional<action_t> pending_promotion;

    // True when the given move would be a promotion and so needs asking about.
    bool MoveNeedsPromotionChoice(action_t move);

    // --- clocks ------------------------------------------------------------
    //
    // The server owns the clocks and sends them with every position. Between
    // those updates the displayed value is wound down locally so the seconds
    // visibly tick, but that is animation: each update snaps it back to what
    // the server said. Nothing the client does here can affect the result.

    void TickClockDisplay(float delta_seconds);

    [[nodiscard]] std::string PlayerClockText();

    [[nodiscard]] std::string OpponentClockText();

    [[nodiscard]] bool GameHasClock();

    // True for whichever clock is counting down, so the board can show whose
    // turn it is without a second source of truth.
    [[nodiscard]] bool PlayerClockRunning();

    extern std::uint16_t time_limit_minutes;

    // Material left on the board, for the little advantage figure beside the
    // clocks.
    int MaterialBalanceForPlayer();

}

#endif
