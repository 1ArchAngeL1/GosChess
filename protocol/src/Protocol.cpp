#include "GosChess/Protocol.hh"

namespace {

    // sf::Packet has no overload for an 8 bit enum, and reading one back as an
    // int would silently accept out of range values. Everything goes through
    // these two so the byte width on the wire is fixed in one place.
    template<typename Enum>
    sf::Packet &WriteEnum(sf::Packet &packet, const Enum &value) {
        return packet << static_cast<std::uint8_t>(value);
    }

    template<typename Enum>
    sf::Packet &ReadEnum(sf::Packet &packet, Enum &value) {
        std::uint8_t raw = 0;
        packet >> raw;
        value = static_cast<Enum>(raw);
        return packet;
    }

}

const char *GosChess::Protocol::Describe(GosChess::Protocol::RejectReason reason) {
    switch (reason) {
        case RejectReason::NOT_IN_GAME:   return "not in a game";
        case RejectReason::NOT_YOUR_TURN: return "not your turn";
        case RejectReason::STALE_PLY:     return "move was for an older position";
        case RejectReason::ILLEGAL_MOVE:  return "illegal move";
        case RejectReason::GAME_FINISHED: return "game already finished";
    }
    return "unknown reason";
}

const char *GosChess::Protocol::Describe(GosChess::Protocol::GameOverReason reason) {
    switch (reason) {
        case GameOverReason::CHECKMATE:      return "checkmate";
        case GameOverReason::STALEMATE:      return "stalemate";
        case GameOverReason::TIMEOUT:        return "out of time";
        case GameOverReason::RESIGNATION:    return "resignation";
        case GameOverReason::OPPONENT_LEFT:  return "opponent left";
        case GameOverReason::PROTOCOL_ERROR: return "protocol error";
        case GameOverReason::FIFTY_MOVE_RULE: return "the fifty move rule";
        case GameOverReason::THREEFOLD_REPETITION: return "threefold repetition";
        case GameOverReason::INSUFFICIENT_MATERIAL: return "insufficient material";
    }
    return "unknown reason";
}

const char *GosChess::Protocol::Describe(GosChess::Protocol::GameOutcome outcome) {
    switch (outcome) {
        case GameOutcome::WHITE_WINS: return "white wins";
        case GameOutcome::BLACK_WINS: return "black wins";
        case GameOutcome::DRAW:       return "draw";
    }
    return "unknown outcome";
}

GosChess::Protocol::GameOutcome GosChess::Protocol::OutcomeForLosingSide(GosChess::color_t loser) {
    return loser == color_t::WHITE ? GameOutcome::BLACK_WINS : GameOutcome::WHITE_WINS;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::ClientMessage &type) {
    return WriteEnum(packet, type);
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::ClientMessage &type) {
    return ReadEnum(packet, type);
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::ServerMessage &type) {
    return WriteEnum(packet, type);
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::ServerMessage &type) {
    return ReadEnum(packet, type);
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::FindGame &message) {
    return packet << message.version << message.time_limit_minutes;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::FindGame &message) {
    return packet >> message.version >> message.time_limit_minutes;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::MoveRequest &message) {
    return packet << message.ply << message.move_from << message.move_to << message.promotion;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::MoveRequest &message) {
    return packet >> message.ply >> message.move_from >> message.move_to >> message.promotion;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::GameStart &message) {
    packet << message.game_id;
    WriteEnum(packet, message.your_color);
    return packet << message.time_limit_minutes;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::GameStart &message) {
    packet >> message.game_id;
    ReadEnum(packet, message.your_color);
    return packet >> message.time_limit_minutes;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::GameStateUpdate &message) {
    packet << message.ply << message.fen;
    WriteEnum(packet, message.color_to_play);
    packet << message.last_move_from << message.last_move_to << message.in_check
           << message.white_millis_left << message.black_millis_left;
    packet << static_cast<std::uint32_t>(message.history.size());
    for (const std::string &move: message.history) packet << move;
    return packet;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::GameStateUpdate &message) {
    packet >> message.ply >> message.fen;
    ReadEnum(packet, message.color_to_play);
    packet >> message.last_move_from >> message.last_move_to >> message.in_check
           >> message.white_millis_left >> message.black_millis_left;
    std::uint32_t moves = 0;
    packet >> moves;
    message.history.clear();
    // A length read off the wire is a length a peer chose, so it is grown into
    // rather than reserved up front.
    for (std::uint32_t i = 0; i < moves; i++) {
        std::string move;
        if (!(packet >> move)) break;
        message.history.push_back(move);
    }
    return packet;
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::GameOver &message) {
    WriteEnum(packet, message.outcome);
    return WriteEnum(packet, message.reason);
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::GameOver &message) {
    ReadEnum(packet, message.outcome);
    return ReadEnum(packet, message.reason);
}

sf::Packet &operator<<(sf::Packet &packet, const GosChess::Protocol::Rejected &message) {
    WriteEnum(packet, message.reason);
    return packet << message.state;
}

sf::Packet &operator>>(sf::Packet &packet, GosChess::Protocol::Rejected &message) {
    ReadEnum(packet, message.reason);
    return packet >> message.state;
}
