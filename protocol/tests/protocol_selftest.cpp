// Round trip checks for the wire format.
//
// Every message is written into a packet and read back out, and the result
// compared field by field. These are cheap and they catch the failure mode
// that is hardest to debug from the outside: a writer and a reader that
// disagree about field order or width, which does not fail - it quietly
// returns the wrong numbers.

#include <cstdio>
#include <string>

#include "GosChess/Protocol.hh"

namespace {

    using namespace GosChess::Protocol;

    int failures = 0;

    void Check(bool condition, const std::string &what) {
        if (condition) return;
        std::printf("  FAIL  %s\n", what.c_str());
        failures++;
    }

    template<typename Message>
    Message RoundTrip(const Message &sent) {
        sf::Packet packet;
        packet << sent;
        Message received;
        packet >> received;
        Check(packet.endOfPacket(), "the reader consumed exactly what the writer wrote");
        return received;
    }

    void TestFindGame() {
        std::printf("find game\n");
        FindGame sent;
        sent.time_limit_minutes = 10;
        const FindGame got = RoundTrip(sent);
        Check(got.version == VERSION, "version survives");
        Check(got.time_limit_minutes == 10, "time limit survives");
    }

    void TestMoveRequest() {
        std::printf("move request\n");
        MoveRequest sent;
        sent.ply = 41;
        sent.move_from = 12;
        sent.move_to = 28;
        sent.promotion = GosChess::figure_types_t::KNIGHT;
        const MoveRequest got = RoundTrip(sent);
        Check(got.ply == 41, "ply survives");
        Check(got.move_from == 12, "origin survives");
        Check(got.move_to == 28, "destination survives");
        Check(got.promotion == GosChess::figure_types_t::KNIGHT,
              "the promotion choice survives rather than defaulting to a queen");

        // Squares are signed on purpose: an unset move is -1, and reading it
        // back as unsigned would turn it into a plausible looking square.
        MoveRequest unset;
        const MoveRequest unset_got = RoundTrip(unset);
        Check(unset_got.move_from == -1 && unset_got.move_to == -1,
              "an unset move stays negative rather than becoming square 255");
    }

    void TestGameStart() {
        std::printf("game start\n");
        GameStart sent;
        sent.game_id = 7;
        sent.your_color = GosChess::color_t::BLACK;
        sent.time_limit_minutes = 3;
        const GameStart got = RoundTrip(sent);
        Check(got.game_id == 7, "game id survives");
        Check(got.your_color == GosChess::color_t::BLACK, "colour survives");
        Check(got.time_limit_minutes == 3, "time limit survives");
    }

    void TestGameState() {
        std::printf("game state\n");
        GameStateUpdate sent;
        sent.ply = 3;
        sent.fen = GosChess::INITIAL_FEN;
        sent.color_to_play = GosChess::color_t::BLACK;
        sent.last_move_from = 11;
        sent.last_move_to = 27;
        sent.in_check = true;
        sent.white_millis_left = 598000;
        sent.black_millis_left = 600000;
        sent.history = {"e4", "e5", "Nf3"};
        const GameStateUpdate got = RoundTrip(sent);
        Check(got.in_check, "the check flag survives");
        Check(got.history.size() == 3, "the whole move list survives");
        Check(got.history[0] == "e4" && got.history[2] == "Nf3", "in order");
        Check(got.ply == 3, "ply survives");
        Check(got.fen == GosChess::INITIAL_FEN, "position survives");
        Check(got.color_to_play == GosChess::color_t::BLACK, "side to play survives");
        Check(got.last_move_from == 11 && got.last_move_to == 27, "last move survives");
        Check(got.white_millis_left == 598000, "white clock survives");
        Check(got.black_millis_left == 600000, "black clock survives");
    }

    void TestGameOver() {
        std::printf("game over\n");
        GameOver sent;
        sent.outcome = GameOutcome::BLACK_WINS;
        sent.reason = GameOverReason::TIMEOUT;
        const GameOver got = RoundTrip(sent);
        Check(got.outcome == GameOutcome::BLACK_WINS, "outcome survives");
        Check(got.reason == GameOverReason::TIMEOUT, "reason survives");
    }

    void TestRejected() {
        std::printf("rejection\n");
        Rejected sent;
        sent.reason = RejectReason::STALE_PLY;
        sent.state.ply = 9;
        sent.state.fen = GosChess::INITIAL_FEN;
        sent.state.color_to_play = GosChess::color_t::BLACK;
        const Rejected got = RoundTrip(sent);
        Check(got.reason == RejectReason::STALE_PLY, "reason survives");
        Check(got.state.ply == 9, "the enclosed state survives");
        Check(got.state.fen == GosChess::INITIAL_FEN, "the enclosed position survives");
    }

    // A single packet holds a tag then a body, so the reader has to pick the
    // body type from the tag. Getting that pairing wrong is the other classic
    // protocol bug.
    void TestTaggedMessages() {
        std::printf("tagged messages\n");
        sf::Packet packet;
        MoveRequest move;
        move.ply = 2;
        move.move_from = 8;
        move.move_to = 16;
        packet << ClientMessage::MOVE << move;

        ClientMessage type = ClientMessage::FIND_GAME;
        packet >> type;
        Check(type == ClientMessage::MOVE, "the tag reads back as itself");
        MoveRequest got;
        packet >> got;
        Check(got.ply == 2 && got.move_from == 8 && got.move_to == 16, "the body follows the tag");
        Check(packet.endOfPacket(), "nothing is left over");
    }

    void TestOutcomeHelper() {
        std::printf("outcome helper\n");
        Check(OutcomeForLosingSide(GosChess::color_t::WHITE) == GameOutcome::BLACK_WINS,
              "white losing means black wins");
        Check(OutcomeForLosingSide(GosChess::color_t::BLACK) == GameOutcome::WHITE_WINS,
              "black losing means white wins");
    }

}

int main() {
    TestFindGame();
    TestMoveRequest();
    TestGameStart();
    TestGameState();
    TestGameOver();
    TestRejected();
    TestTaggedMessages();
    TestOutcomeHelper();

    if (failures == 0) {
        std::printf("\nall checks passed\n");
        return 0;
    }
    std::printf("\n%d check(s) failed\n", failures);
    return 1;
}
