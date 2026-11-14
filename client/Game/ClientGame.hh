#ifndef GOSCHESS_CLIENTGAME_HH
#define GOSCHESS_CLIENTGAME_HH

#include <cstdint>
#include <string>

#include "GosChess/Board.hh"
#include "GosChess/GameState.hh"
#include "GosChess/Protocol.hh"

namespace GosChess {

    // The client's copy of the game.
    //
    // It is a copy, not the original: the server holds the position that
    // counts. This one exists so the board can be drawn, so legal destinations
    // can be highlighted without asking anybody, and so a move the player
    // makes appears at once instead of after a round trip. Every state the
    // server sends replaces it wholesale, which is what keeps a prediction
    // from quietly becoming a disagreement.
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

    void SetPlayerColor(color_t color);

    bool PlayerToMove();

    // Replaces the local position with the server's, whatever ours said.
    void AdoptServerState(const Protocol::GameStateUpdate &update);

    // Plays a move locally for immediate feedback, without claiming it is
    // final. Returns false if our own copy of the rules already refuses it, in
    // which case there is no point troubling the server.
    bool PredictMove(action_t move);

    void ResetClientGame();

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

    extern std::uint16_t time_limit_minutes;

}

#endif
