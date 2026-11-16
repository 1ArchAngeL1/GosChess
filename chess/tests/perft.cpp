// Node counts for known positions.
//
// A move generator can look healthy in hand-written tests and still be quietly
// wrong: a castle allowed through check, an en passant capture that forgets to
// remove the pawn, a promotion that only ever offers a queen. Perft catches all
// of it, because the leaf count of the legal move tree is one number that every
// one of those mistakes changes, and the numbers for these positions are
// published and agreed.
//
// Run with --deep for the deeper counts. The default depths are the ones that
// keep this quick enough to live in the normal test run; the deep ones have all
// been checked to match too, and are what to reach for after touching move
// generation.

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "GosChess/Board.hh"
#include "GosChess/GameState.hh"
#include "GosChess/Rules.hh"

namespace {

    int failures = 0;

    struct Case {
        const char *name;
        const char *fen;
        // Expected node count at depth 1, 2, 3, ... as far as it is listed.
        std::vector<std::uint64_t> expected;
        // How far to go normally; the rest is only visited with --deep.
        std::size_t quick_depth;
    };

    void Run(const Case &test_case, bool deep) {
        GosChess::GameState game;
        if (!game.setFen(test_case.fen)) {
            std::printf("  FAIL  %s: the FEN could not be read\n", test_case.name);
            failures++;
            return;
        }
        std::printf("%s\n", test_case.name);
        const std::size_t limit = deep ? test_case.expected.size()
                                       : std::min(test_case.quick_depth, test_case.expected.size());
        for (std::size_t i = 0; i < limit; i++) {
            const int depth = static_cast<int>(i) + 1;
            const auto started = std::chrono::steady_clock::now();
            const std::uint64_t nodes = GosChess::Perft(game.board(), game.colorToPlay(), depth);
            const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - started).count();
            const std::uint64_t want = test_case.expected[i];
            if (nodes == want) {
                std::printf("  depth %d  %10llu nodes  %6lld ms\n", depth,
                            static_cast<unsigned long long>(nodes), static_cast<long long>(millis));
                continue;
            }
            std::printf("  FAIL  depth %d: got %llu, expected %llu\n", depth,
                        static_cast<unsigned long long>(nodes),
                        static_cast<unsigned long long>(want));
            failures++;
            // Deeper counts would only repeat the same discrepancy.
            return;
        }
    }

}

int main(int argc, char **argv) {
    bool deep = false;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--deep") == 0) deep = true;
    }

    const std::vector<Case> cases = {
            {"opening position",
             GosChess::INITIAL_FEN,
             {20, 400, 8902, 197281, 4865609},
             3},

            // Castling both ways for both sides, pieces pinned against both
            // kings, and checks available: the position everyone uses to find
            // castling bugs.
            {"kiwipete - castling, pins and checks",
             "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
             {48, 2039, 97862, 4085603},
             3},

            // Turns on en passant captures that expose the king along a rank,
            // which is the case a naive en passant implementation gets wrong.
            {"endgame - en passant and discovered checks",
             "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
             {14, 191, 2812, 43238, 674624},
             4},

            // Pawns on the seventh with four promotion choices each, and
            // captures that promote.
            {"promotions and a trapped king",
             "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
             {6, 264, 9467, 422333},
             3},

            {"castling rights lost under attack",
             "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
             {44, 1486, 62379, 2103487},
             3},
    };

    for (const Case &test_case: cases) Run(test_case, deep);

    if (failures == 0) {
        std::printf("\nall node counts match%s\n", deep ? " (deep)" : "");
        return 0;
    }
    std::printf("\n%d count(s) wrong\n", failures);
    return 1;
}
