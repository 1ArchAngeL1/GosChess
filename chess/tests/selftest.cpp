// Sanity checks for the headless rules engine.
//
// The board used to be stored mirrored for whoever was sitting on the black
// side, which meant move generation could only be right for one of the two
// players at a time. These checks pin down the absolute orientation that
// replaced it: White moves towards higher indices, Black towards lower ones,
// a1 is 0 and h8 is 63, for both sides at once.

#include <cstdio>
#include <string>

#include "GosChess/Board.hh"
#include "GosChess/GameState.hh"
#include "GosChess/Rules.hh"

namespace {

    int failures = 0;

    void Check(bool condition, const std::string &what) {
        if (condition) return;
        std::printf("  FAIL  %s\n", what.c_str());
        failures++;
    }

    // "e1" -> 4, "a1" -> 0, "h8" -> 63.
    constexpr int Sq(char file, int rank) {
        return (rank - 1) * GosChess::board_t::ROW_LENGTH + (file - 'a');
    }

    std::size_t CountMoves(const GosChess::action_map_t &moves) {
        std::size_t total = 0;
        for (const auto &[square, from_square]: moves) total += from_square.size();
        return total;
    }

    bool Has(const GosChess::action_map_t &moves, int from, int to) {
        const auto found = moves.find(static_cast<std::int8_t>(from));
        if (found == moves.end()) return false;
        return found->second.count(GosChess::action_t(static_cast<std::int8_t>(from),
                                                      static_cast<std::int8_t>(to))) != 0;
    }

    void TestInitialPosition() {
        std::printf("initial position\n");
        GosChess::GameState game;

        Check(game.colorToPlay() == GosChess::color_t::WHITE, "white starts");
        Check(game.status() == GosChess::GameStatus::IN_PROGRESS, "game is in progress");
        Check(CountMoves(game.availableMoves()) == 20, "white has 20 opening moves");

        Check(game.board().getFigure(Sq('e', 1)).type_ == GosChess::figure_types_t::KING,
              "white king on e1");
        Check(game.board().getFigure(Sq('e', 1)).color_ == GosChess::color_t::WHITE,
              "e1 king is white");
        Check(game.board().getFigure(Sq('e', 8)).type_ == GosChess::figure_types_t::KING,
              "black king on e8");
        Check(game.board().getFigure(Sq('e', 8)).color_ == GosChess::color_t::BLACK,
              "e8 king is black");

        Check(game.board().boardStateToFen() == GosChess::INITIAL_FEN, "fen round trips");
    }

    // The orientation check that matters: both colours' pawns must run the
    // right way on one and the same board.
    void TestPawnDirections() {
        std::printf("pawn directions\n");
        GosChess::GameState game;

        Check(Has(game.availableMoves(), Sq('a', 2), Sq('a', 3)), "white pawn pushes one up");
        Check(Has(game.availableMoves(), Sq('a', 2), Sq('a', 4)), "white pawn pushes two up");
        Check(!Has(game.availableMoves(), Sq('a', 2), Sq('a', 1)), "white pawn never moves down");

        Check(game.applyMove(GosChess::action_t(Sq('a', 2), Sq('a', 3))), "white opens a3");
        Check(game.colorToPlay() == GosChess::color_t::BLACK, "turn passes to black");
        Check(CountMoves(game.availableMoves()) == 20, "black has 20 opening moves");

        Check(Has(game.availableMoves(), Sq('a', 7), Sq('a', 6)), "black pawn pushes one down");
        Check(Has(game.availableMoves(), Sq('a', 7), Sq('a', 5)), "black pawn pushes two down");
        Check(!Has(game.availableMoves(), Sq('a', 7), Sq('a', 8)), "black pawn never moves up");
    }

    void TestTurnOwnership() {
        std::printf("turn ownership\n");
        GosChess::GameState game;

        // The property the old peer to peer build lacked entirely: a move that
        // is not yours is refused instead of being replayed on trust.
        Check(!game.applyMove(GosChess::action_t(Sq('a', 7), Sq('a', 6))),
              "white cannot move a black pawn");
        Check(!game.applyMove(GosChess::action_t(Sq('a', 1), Sq('a', 5))),
              "rook cannot jump through its own pawn");
        Check(!game.applyMove(GosChess::action_t(Sq('d', 4), Sq('d', 5))),
              "empty square cannot move");
        // A move off the wire is whatever bytes arrived, so out of range
        // squares have to bounce off rather than index the board.
        Check(!game.applyMove(GosChess::action_t(100, 3)), "move from off the board is refused");
        Check(!game.applyMove(GosChess::action_t(3, -7)), "move to off the board is refused");
        Check(!game.applyMove(GosChess::action_t()), "a move that was never filled in is refused");
        Check(game.board().boardStateToFen() == GosChess::INITIAL_FEN,
              "rejected moves leave the board untouched");
        Check(game.colorToPlay() == GosChess::color_t::WHITE, "rejected moves do not pass the turn");
    }

    void TestFoolsMate() {
        std::printf("fool's mate\n");
        GosChess::GameState game;

        Check(game.applyMove(GosChess::action_t(Sq('f', 2), Sq('f', 3))), "1. f3");
        Check(game.applyMove(GosChess::action_t(Sq('e', 7), Sq('e', 5))), "1... e5");
        Check(game.applyMove(GosChess::action_t(Sq('g', 2), Sq('g', 4))), "2. g4");
        Check(game.status() == GosChess::GameStatus::IN_PROGRESS, "not over before the mate");
        Check(game.applyMove(GosChess::action_t(Sq('d', 8), Sq('h', 4))), "2... Qh4#");

        Check(game.status() == GosChess::GameStatus::CHECKMATE, "white is mated");
        Check(game.colorToPlay() == GosChess::color_t::WHITE, "white is the side to play when mated");
        Check(!game.applyMove(GosChess::action_t(Sq('g', 1), Sq('h', 3))),
              "no move is accepted after mate");
    }

    void TestCheckEvasionIsForced() {
        std::printf("check evasion\n");
        GosChess::GameState game;

        // Same idea as the mate above, but with the g pawn left at home so
        // white still has g3 to interpose.
        Check(game.applyMove(GosChess::action_t(Sq('f', 2), Sq('f', 3))), "1. f3");
        Check(game.applyMove(GosChess::action_t(Sq('e', 7), Sq('e', 5))), "1... e5");
        Check(game.applyMove(GosChess::action_t(Sq('a', 2), Sq('a', 3))), "2. a3");
        Check(game.applyMove(GosChess::action_t(Sq('d', 8), Sq('h', 4))), "2... Qh4+");

        Check(GosChess::IsKingInCheck(game.board().getRawBoard(), GosChess::color_t::WHITE),
              "white king is in check");
        Check(game.status() == GosChess::GameStatus::IN_PROGRESS, "check is not mate");
        Check(!game.applyMove(GosChess::action_t(Sq('b', 2), Sq('b', 3))),
              "a move that ignores the check is refused");
        Check(game.applyMove(GosChess::action_t(Sq('g', 2), Sq('g', 3))),
              "g3 interposes and resolves the check");
        Check(!GosChess::IsKingInCheck(game.board().getRawBoard(), GosChess::color_t::WHITE),
              "white king is safe again");
    }

    void TestPromotion() {
        std::printf("promotion\n");

        // White king a1, white pawn a7, black king h8. Ranks are listed a1 first.
        GosChess::GameState white_game;
        white_game.setPosition("K7/8/8/8/8/8/P7/7k/", GosChess::color_t::WHITE);
        Check(white_game.applyMove(GosChess::action_t(Sq('a', 7), Sq('a', 8))), "white pawn reaches a8");
        Check(white_game.board().getFigure(Sq('a', 8)).type_ == GosChess::figure_types_t::QUEEN,
              "white pawn promotes to a queen");
        Check(white_game.board().getFigure(Sq('a', 8)).color_ == GosChess::color_t::WHITE,
              "promoted queen stays white");

        // Mirror image: black king a8, black pawn a2, white king h1.
        GosChess::GameState black_game;
        black_game.setPosition("7K/p7/8/8/8/8/8/k7/", GosChess::color_t::BLACK);
        Check(black_game.applyMove(GosChess::action_t(Sq('a', 2), Sq('a', 1))), "black pawn reaches a1");
        Check(black_game.board().getFigure(Sq('a', 1)).type_ == GosChess::figure_types_t::QUEEN,
              "black pawn promotes to a queen");
        Check(black_game.board().getFigure(Sq('a', 1)).color_ == GosChess::color_t::BLACK,
              "promoted queen stays black");
    }

    void TestStalemate() {
        std::printf("stalemate\n");

        // Black king a8, white queen b6, white king c1. Black to play has no
        // move and is not in check.
        GosChess::GameState game;
        game.setPosition("2K5/8/8/8/8/1Q6/8/k7/", GosChess::color_t::BLACK);
        Check(game.status() == GosChess::GameStatus::STALEMATE, "black is stalemated");
        Check(!GosChess::IsKingInCheck(game.board().getRawBoard(), GosChess::color_t::BLACK),
              "stalemated king is not in check");
    }

    // Two games in one process must not influence each other. This is the
    // whole point of the state object, and what a server depends on.
    void TestGamesAreIndependent() {
        std::printf("game independence\n");
        GosChess::GameState first;
        GosChess::GameState second;

        Check(first.applyMove(GosChess::action_t(Sq('e', 2), Sq('e', 4))), "first game plays e4");
        Check(first.colorToPlay() == GosChess::color_t::BLACK, "first game is on black");
        Check(second.colorToPlay() == GosChess::color_t::WHITE, "second game is still on white");
        Check(second.board().boardStateToFen() == GosChess::INITIAL_FEN,
              "second game board is untouched");
        Check(second.applyMove(GosChess::action_t(Sq('e', 2), Sq('e', 4))), "second game plays e4");
    }

}

int main() {
    TestInitialPosition();
    TestPawnDirections();
    TestTurnOwnership();
    TestFoolsMate();
    TestCheckEvasionIsForced();
    TestPromotion();
    TestStalemate();
    TestGamesAreIndependent();

    if (failures == 0) {
        std::printf("\nall checks passed\n");
        return 0;
    }
    std::printf("\n%d check(s) failed\n", failures);
    return 1;
}
