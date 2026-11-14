#ifndef GOSCHESS_PROTOCOL_HH
#define GOSCHESS_PROTOCOL_HH

#include <cstdint>
#include <string>

#include <SFML/Network/Packet.hpp>

#include "GosChess/Board.hh"
#include "GosChess/Move.hh"

// The wire format, defined once and linked by both ends.
//
// Shape of the conversation:
//
//   client                        server
//   ------                        ------
//   FindGame        ----------->
//                   <-----------  Waiting            (no opponent yet)
//                   <-----------  GameStart          (paired, colours drawn)
//                   <-----------  GameState          (opening position)
//   Move            ----------->
//                   <-----------  GameState          (to both players)
//   Resign          ----------->
//                   <-----------  GameOver           (to both players)
//
// GameState carries the whole position, not a delta. A move is two bytes and
// a position is about seventy, so at this game's traffic the difference is
// noise - and in exchange the client can never drift from the server, and
// rejoining a game in progress is the same code path as starting one.

namespace GosChess::Protocol {

    // Bumped whenever the meaning of anything below changes. The server
    // refuses a client that does not match, because a silent mismatch shows up
    // as inexplicable game behaviour rather than as a connection error.
    inline constexpr std::uint16_t VERSION = 1;

    inline constexpr unsigned short DEFAULT_PORT = 2001;

    enum class ClientMessage : std::uint8_t {
        FIND_GAME = 1,
        MOVE,
        RESIGN
    };

    enum class ServerMessage : std::uint8_t {
        WAITING = 1,
        GAME_START,
        GAME_STATE,
        GAME_OVER,
        REJECTED
    };

    enum class GameOutcome : std::uint8_t {
        WHITE_WINS = 1,
        BLACK_WINS,
        DRAW
    };

    enum class GameOverReason : std::uint8_t {
        CHECKMATE = 1,
        STALEMATE,
        TIMEOUT,
        RESIGNATION,
        OPPONENT_LEFT,
        PROTOCOL_ERROR
    };

    // Why the server would not play a move. Sent alongside the authoritative
    // state, so a client that gets one knows both that it was wrong and what
    // the position really is.
    enum class RejectReason : std::uint8_t {
        NOT_IN_GAME = 1,
        NOT_YOUR_TURN,
        STALE_PLY,        // the move was for a position that has moved on
        ILLEGAL_MOVE,
        GAME_FINISHED
    };

    // --- client to server --------------------------------------------------

    struct FindGame {
        std::uint16_t version = VERSION;
        // Zero means no clock.
        std::uint16_t time_limit_minutes = 0;
    };

    struct MoveRequest {
        // Which position this move is for. The server rejects anything that
        // does not match its own count, so a duplicated or delayed packet
        // cannot be applied twice.
        std::uint32_t ply = 0;
        std::int8_t move_from = -1;
        std::int8_t move_to = -1;
    };

    // --- server to client --------------------------------------------------

    struct GameStart {
        std::uint32_t game_id = 0;
        // The side this recipient plays. The opponent gets the other one.
        color_t your_color = color_t::WHITE;
        std::uint16_t time_limit_minutes = 0;
    };

    struct GameStateUpdate {
        std::uint32_t ply = 0;
        std::string fen;
        color_t color_to_play = color_t::WHITE;
        // The move that produced this position, for highlighting it. Both -1
        // in the opening position.
        std::int8_t last_move_from = -1;
        std::int8_t last_move_to = -1;
        // Per side, so a client never has to work out which clock is its own
        // from a point of view the server does not share.
        std::uint32_t white_millis_left = 0;
        std::uint32_t black_millis_left = 0;
    };

    struct GameOver {
        GameOutcome outcome = GameOutcome::DRAW;
        GameOverReason reason = GameOverReason::CHECKMATE;
    };

    struct Rejected {
        RejectReason reason = RejectReason::ILLEGAL_MOVE;
        // The position the client should snap back to.
        GameStateUpdate state;
    };

    // --- helpers -----------------------------------------------------------

    const char *Describe(RejectReason reason);

    const char *Describe(GameOverReason reason);

    const char *Describe(GameOutcome outcome);

    // The outcome when `loser` has lost, however it happened - mated, flagged,
    // resigned or gone. Used for all four so that the mapping from a losing
    // colour to a result exists in exactly one place.
    GameOutcome OutcomeForLosingSide(color_t loser);

}

// Packet operators. Declared in the global namespace, which is where
// sf::Packet's own operator<< overloads live and so where overload resolution
// for `packet << value` will look.

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::FindGame &message);

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::FindGame &message);

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::MoveRequest &message);

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::MoveRequest &message);

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::GameStart &message);

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::GameStart &message);

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::GameStateUpdate &message);

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::GameStateUpdate &message);

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::GameOver &message);

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::GameOver &message);

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::Rejected &message);

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::Rejected &message);

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::ClientMessage &type);

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::ClientMessage &type);

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::ServerMessage &type);

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::ServerMessage &type);

#endif
