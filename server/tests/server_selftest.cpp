// Checks the server's authority over a single game, with no network involved.
//
// Match is where every claim about the new architecture actually lives: that a
// move which is not yours is refused, that a replayed packet cannot be applied
// twice, that the result is decided by the rules rather than announced by a
// player, and that the clock belongs to the server. Each of those is checked
// here directly, because driving them through a socket would test the socket.

#include <cstdio>
#include <string>

#include "Match.hh"

namespace {

    using namespace GosChess;
    using namespace GosChess::Protocol;

    int failures = 0;

    void Check(bool condition, const std::string &what) {
        if (condition) return;
        std::printf("  FAIL  %s\n", what.c_str());
        failures++;
    }

    constexpr std::int8_t Sq(char file, int rank) {
        return static_cast<std::int8_t>((rank - 1) * board_t::ROW_LENGTH + (file - 'a'));
    }

    MoveRequest At(std::uint32_t ply, char from_file, int from_rank, char to_file, int to_rank,
                   std::uint8_t promotion = 0) {
        MoveRequest request;
        request.ply = ply;
        request.move_from = Sq(from_file, from_rank);
        request.move_to = Sq(to_file, to_rank);
        request.promotion = promotion;
        return request;
    }

    void TestOpening() {
        std::printf("a fresh match\n");
        Match match(1, 0);
        Check(match.id() == 1, "keeps its id");
        Check(match.ply() == 0, "starts at ply zero");
        Check(match.colorToPlay() == color_t::WHITE, "starts with white to play");
        Check(!match.finished(), "is not over");
        Check(!match.hasClock(), "has no clock when the time limit is zero");
        Check(match.state().fen == INITIAL_FEN, "starts from the opening position");
        Check(match.state().history.empty(), "with no moves recorded");
    }

    void TestOnlyYourOwnMoves() {
        std::printf("moves that are not yours\n");
        Match match(1, 0);

        // The hole the peer to peer build had: a client could send any move at
        // all and the opponent would replay it without question.
        const Match::MoveResult wrong_side = match.applyMove(color_t::BLACK, At(0, 'a', 7, 'a', 6));
        Check(!wrong_side.accepted, "black cannot move while white is to play");
        Check(wrong_side.reject_reason == RejectReason::NOT_YOUR_TURN, "and is told why");

        const Match::MoveResult other_army = match.applyMove(color_t::WHITE, At(0, 'a', 7, 'a', 6));
        Check(!other_army.accepted, "white cannot move a black pawn");
        Check(other_army.reject_reason == RejectReason::ILLEGAL_MOVE, "and is told why");

        const Match::MoveResult nonsense = match.applyMove(color_t::WHITE, At(0, 'a', 1, 'a', 5));
        Check(!nonsense.accepted, "a rook cannot pass through its own pawn");

        Check(match.ply() == 0, "a refused move does not advance the game");
        Check(match.state().fen == INITIAL_FEN, "a refused move does not change the position");
        Check(match.state().history.empty(), "and is not written into the record");
    }

    // A move names the position it was made for, so a packet that arrives
    // twice - or late - cannot be played twice.
    void TestStalePlyIsRefused() {
        std::printf("replayed and stale moves\n");
        Match match(1, 0);
        const MoveRequest e4 = At(0, 'e', 2, 'e', 4);

        Check(match.applyMove(color_t::WHITE, e4).accepted, "the move is played once");
        Check(match.ply() == 1, "the game advances");
        Check(match.colorToPlay() == color_t::BLACK, "the turn passes");

        const Match::MoveResult replayed = match.applyMove(color_t::WHITE, e4);
        Check(!replayed.accepted, "the same packet arriving again is refused");
        Check(replayed.reject_reason == RejectReason::NOT_YOUR_TURN,
              "because it is no longer that player's turn");

        // Black's turn, but black is talking about the position before e4.
        const Match::MoveResult stale = match.applyMove(color_t::BLACK, At(0, 'e', 7, 'e', 5));
        Check(!stale.accepted, "a move for an older position is refused");
        Check(stale.reject_reason == RejectReason::STALE_PLY, "and is told why");

        Check(match.applyMove(color_t::BLACK, At(1, 'e', 7, 'e', 5)).accepted,
              "the same move with the right ply is played");
        Check(match.ply() == 2, "and the game advances");
    }

    void TestCheckmateDecidesTheResult() {
        std::printf("checkmate\n");
        Match match(1, 0);
        Check(match.applyMove(color_t::WHITE, At(0, 'f', 2, 'f', 3)).accepted, "1. f3");
        Check(match.applyMove(color_t::BLACK, At(1, 'e', 7, 'e', 5)).accepted, "1... e5");
        Check(match.applyMove(color_t::WHITE, At(2, 'g', 2, 'g', 4)).accepted, "2. g4");
        Check(!match.finished(), "not over yet");
        Check(match.applyMove(color_t::BLACK, At(3, 'd', 8, 'h', 4)).accepted, "2... Qh4#");

        Check(match.finished(), "the game is over");
        Check(match.result().reason == GameOverReason::CHECKMATE, "by checkmate");
        Check(match.result().outcome == GameOutcome::BLACK_WINS, "and black has won");

        const Match::MoveResult after = match.applyMove(color_t::WHITE, At(4, 'g', 1, 'h', 3));
        Check(!after.accepted, "no move is accepted after the game ends");
        Check(after.reject_reason == RejectReason::GAME_FINISHED, "and is told why");
    }

    void TestStalemateIsADraw() {
        std::printf("stalemate\n");
        Match match(1, 0);
        // Black king a8, white queen b6, white king c1: black has no move and
        // is not in check.
        Check(match.setFen("k7/8/1Q6/8/8/8/8/2K5 b - - 0 1"), "stalemate position");
        Check(match.finished(), "the game is over as soon as the position is set");
        Check(match.result().reason == GameOverReason::STALEMATE, "by stalemate");
        Check(match.result().outcome == GameOutcome::DRAW, "and it is a draw");
    }

    // The clock that used to be ticked by whichever player was hosting, and
    // mirrored to the other one as a number to be believed.
    void TestServerOwnsTheClock() {
        std::printf("clocks\n");
        Match match(1, 5);
        Check(match.hasClock(), "a time limit gives the match a clock");
        Check(match.state().white_millis_left == 5 * 60 * 1000, "white starts with five minutes");
        Check(match.state().black_millis_left == 5 * 60 * 1000, "black starts with five minutes");

        Check(!match.chargeTime(1000), "a second passes without ending anything");
        Check(match.state().white_millis_left == 5 * 60 * 1000 - 1000, "white is charged for it");
        Check(match.state().black_millis_left == 5 * 60 * 1000, "black is not");

        Check(match.applyMove(color_t::WHITE, At(0, 'e', 2, 'e', 4)).accepted, "white moves");
        Check(!match.chargeTime(2000), "two more seconds pass");
        Check(match.state().black_millis_left == 5 * 60 * 1000 - 2000,
              "now it is black being charged");
        Check(match.state().white_millis_left == 5 * 60 * 1000 - 1000, "white's clock is paused");
    }

    void TestFlagFall() {
        std::printf("running out of time\n");
        Match match(1, 1);
        Check(!match.chargeTime(59 * 1000), "white uses most of the minute");
        Check(match.chargeTime(2000), "and then runs past it");
        Check(match.finished(), "the game is over");
        Check(match.result().reason == GameOverReason::TIMEOUT, "on time");
        Check(match.result().outcome == GameOutcome::BLACK_WINS, "and black has won");
        Check(match.state().white_millis_left == 0, "white's clock reads zero");
        Check(!match.chargeTime(1000), "further time does not change a finished game");
    }

    void TestNoClockNeverExpires() {
        std::printf("games without a clock\n");
        Match match(1, 0);
        Check(!match.chargeTime(60u * 60u * 1000u), "an hour passes harmlessly");
        Check(!match.finished(), "an untimed game does not end on time");
    }

    void TestResignation() {
        std::printf("resignation\n");
        Match match(1, 0);
        match.resign(color_t::WHITE);
        Check(match.finished(), "the game is over");
        Check(match.result().reason == GameOverReason::RESIGNATION, "by resignation");
        Check(match.result().outcome == GameOutcome::BLACK_WINS, "and black has won");
    }

    void TestAbandonment() {
        std::printf("walking away\n");
        Match match(1, 0);
        match.abandon(color_t::BLACK);
        Check(match.finished(), "the game is over");
        Check(match.result().reason == GameOverReason::OPPONENT_LEFT, "because a player left");
        Check(match.result().outcome == GameOutcome::WHITE_WINS, "and the one who stayed won");
    }

    void TestStateReportsTheLastMove() {
        std::printf("reported state\n");
        Match match(1, 0);
        Check(match.state().last_move_from == -1 && match.state().last_move_to == -1,
              "the opening position has no last move");
        Check(match.applyMove(color_t::WHITE, At(0, 'e', 2, 'e', 4)).accepted, "white moves");
        Check(match.state().last_move_from == Sq('e', 2), "the origin is reported");
        Check(match.state().last_move_to == Sq('e', 4), "the destination is reported");
        Check(match.state().ply == 1, "the ply is reported");
        Check(match.state().color_to_play == color_t::BLACK, "the side to play is reported");
        Check(match.state().fen != INITIAL_FEN, "the position has changed");
        Check(match.state().history.size() == 1 && match.state().history[0] == "e4",
              "and the move is written into the record in algebraic notation");
    }

    // Two games in one process must not touch each other.
    // The promotion choice has to survive the trip through the server, or a
    // player who asked for a knight gets a queen and a lost endgame.
    void TestPromotionChoiceIsHonoured() {
        std::printf("promotion through the server\n");
        Match match(1, 0);
        Check(match.setFen("8/P7/4k3/8/8/8/8/4K3 w - - 0 1"), "a pawn one square from promoting");
        Check(match.applyMove(color_t::WHITE,
                              At(0, 'a', 7, 'a', 8, figure_types_t::KNIGHT)).accepted,
              "white promotes to a knight");
        Check(match.state().history.size() == 1 && match.state().history[0] == "a8=N",
              "and the record says so");

        Match no_choice(2, 0);
        no_choice.setFen("8/P7/4k3/8/8/8/8/4K3 w - - 0 1");
        Check(!no_choice.applyMove(color_t::WHITE, At(0, 'a', 7, 'a', 8)).accepted,
              "a promotion with no piece named is refused rather than guessed at");
    }

    void TestCastlingThroughTheServer() {
        std::printf("castling through the server\n");
        Match match(1, 0);
        Check(match.setFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"), "a castling position");
        Check(match.applyMove(color_t::WHITE, At(0, 'e', 1, 'g', 1)).accepted, "white castles");
        Check(match.state().history[0] == "O-O", "recorded as O-O");
        Check(match.state().fen.find("R4RK1") != std::string::npos ||
              match.state().fen.find("R4RK") != std::string::npos,
              "with the rook moved to f1 in the position the client is sent");
    }

    void TestDrawnGamesAreReported() {
        std::printf("drawn games\n");
        Match fifty(1, 0);
        fifty.setFen("4k3/8/8/8/8/8/8/R3K3 w - - 99 60");
        Check(fifty.applyMove(color_t::WHITE, At(0, 'a', 1, 'a', 2)).accepted, "a quiet rook move");
        Check(fifty.finished(), "ends the game");
        Check(fifty.result().reason == GameOverReason::FIFTY_MOVE_RULE, "by the fifty move rule");
        Check(fifty.result().outcome == GameOutcome::DRAW, "as a draw");

        Match material(2, 0);
        Check(material.setFen("4k3/8/8/8/8/8/8/3BK3 w - - 0 1"), "a lone bishop");
        Check(material.finished(), "cannot mate, so the game is over at once");
        Check(material.result().reason == GameOverReason::INSUFFICIENT_MATERIAL,
              "for insufficient material");

        Match repetition(3, 0);
        for (int round = 0; round < 2; round++) {
            repetition.applyMove(color_t::WHITE, At(repetition.ply(), 'g', 1, 'f', 3));
            repetition.applyMove(color_t::BLACK, At(repetition.ply(), 'g', 8, 'f', 6));
            repetition.applyMove(color_t::WHITE, At(repetition.ply(), 'f', 3, 'g', 1));
            repetition.applyMove(color_t::BLACK, At(repetition.ply(), 'f', 6, 'g', 8));
        }
        Check(repetition.finished(), "shuffling back to the same position three times ends it");
        Check(repetition.result().reason == GameOverReason::THREEFOLD_REPETITION,
              "by threefold repetition");
    }

    void TestMatchesAreIndependent() {
        std::printf("independence\n");
        Match first(1, 0);
        Match second(2, 0);
        Check(first.applyMove(color_t::WHITE, At(0, 'e', 2, 'e', 4)).accepted, "the first game moves");
        Check(second.ply() == 0, "the second game has not");
        Check(second.state().fen == INITIAL_FEN, "and still holds the opening position");
        Check(second.colorToPlay() == color_t::WHITE, "with white still to play");
    }

}

int main() {
    TestOpening();
    TestOnlyYourOwnMoves();
    TestStalePlyIsRefused();
    TestCheckmateDecidesTheResult();
    TestStalemateIsADraw();
    TestServerOwnsTheClock();
    TestFlagFall();
    TestNoClockNeverExpires();
    TestResignation();
    TestAbandonment();
    TestStateReportsTheLastMove();
    TestPromotionChoiceIsHonoured();
    TestCastlingThroughTheServer();
    TestDrawnGamesAreReported();
    TestMatchesAreIndependent();

    if (failures == 0) {
        std::printf("\nall checks passed\n");
        return 0;
    }
    std::printf("\n%d check(s) failed\n", failures);
    return 1;
}
