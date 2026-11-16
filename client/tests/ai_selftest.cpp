// Checks on the computer opponent.
//
// The engine is searched, not scripted, so the only things worth asserting are
// the ones that must hold whatever it decides: that what it hands back is a
// move that can actually be played, that it takes a piece left hanging in
// front of it, and that it finishes a game it has already won. A search that
// returns an illegal move is far worse than a weak one - it stalls the game.

#include <cstdio>
#include <string>
#include <vector>

#include "GosChess/GameState.hh"
#include "GosChess/Notation.hh"
#include "GosChess/Rules.hh"
#include "../Chess/AI/AICalculations.hh"

namespace {

    using namespace GosChess;

    int failures = 0;

    void Check(bool condition, const std::string &what) {
        if (condition) return;
        std::printf("  FAIL  %s\n", what.c_str());
        failures++;
    }

    bool IsLegalIn(GameState &game, action_t move) {
        for (const auto &[square, moves]: game.availableMoves()) {
            for (const action_t &candidate: moves) {
                if (candidate == move) return TryMove(move, game.board(), game.colorToPlay()) &&
                                              (UndoMove(game.board()), true);
            }
        }
        return false;
    }

    void TestAlwaysReturnsALegalMove() {
        std::printf("every level returns a playable move\n");
        const char *positions[] = {
                INITIAL_FEN,
                "r1bqkbnr/pppp1ppp/2n5/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 0 1",
                "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        };
        const int saved_level = ai_level;
        for (const char *fen: positions) {
            for (int level = 1; level <= MAX_AI_LEVEL; level++) {
                ai_level = level;
                GameState game;
                Check(game.setFen(fen), "position loads");
                const action_t move = GetBestMove(game.board(), game.colorToPlay());
                Check(IsLegalIn(game, move),
                      std::string("level ") + std::to_string(level) + " returns a legal move (" +
                      ToCoordinateNotation(move) + ")");
                // And the game must actually accept it, which is the thing the
                // game loop depends on.
                Check(game.applyMove(move), "and the game accepts it");
            }
        }
        ai_level = saved_level;
    }

    void TestTakesFreeMaterial() {
        std::printf("taking what is offered\n");
        const int saved_level = ai_level;
        ai_level = 3;
        GameState game;
        // A black queen sitting en prise on d5 with nothing defending it.
        Check(game.setFen("4k3/8/8/3q4/8/8/8/3QK3 w - - 0 1"), "position loads");
        const action_t move = GetBestMove(game.board(), color_t::WHITE);
        Check(move.move_to == SquareFromName("d5"),
              std::string("the hanging queen is taken, not ") + ToCoordinateNotation(move));
        ai_level = saved_level;
    }

    void TestFindsMateInOne() {
        std::printf("finishing a won game\n");
        const int saved_level = ai_level;
        ai_level = 3;
        GameState game;
        // Back rank mate: Ra8 is mate and nothing else is.
        Check(game.setFen("6k1/5ppp/8/8/8/8/8/R3K3 w - - 0 1"), "position loads");
        const action_t move = GetBestMove(game.board(), color_t::WHITE);
        Check(game.applyMove(move), "the move is played");
        Check(game.status() == GameStatus::CHECKMATE,
              std::string("mate in one is found, not ") + game.lastMoveNotation());
        ai_level = saved_level;
    }

    // A search that cannot see a move - because every one of them is illegal -
    // must not hand back a square off the board for the game to index with.
    void TestStuckPositionIsSafe() {
        std::printf("a position with no moves\n");
        GameState game;
        Check(game.setFen("k7/8/1Q6/8/8/8/8/2K5 b - - 0 1"), "a stalemated position");
        const action_t move = GetBestMove(game.board(), color_t::BLACK);
        Check(!game.applyMove(move), "whatever comes back is refused rather than played");
    }

}

int main() {
    TestAlwaysReturnsALegalMove();
    TestTakesFreeMaterial();
    TestFindsMateInOne();
    TestStuckPositionIsSafe();

    if (failures == 0) {
        std::printf("\nall checks passed\n");
        return 0;
    }
    std::printf("\n%d check(s) failed\n", failures);
    return 1;
}
