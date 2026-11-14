// Checks the mapping between a square index and its place on the drawn board.
//
// This is the half of the orientation change that the rules engine cannot
// catch: the board is now stored white-side-up for both players, so the
// renderer is what has to put black's own pieces at the bottom of black's
// screen. Getting it backwards would still pass every rules check and simply
// show one player an upside down game.

#include <cstdio>
#include <string>

#include "GosChess/Board.hh"
#include "../Game/ClientGame.hh"
#include "../Render/BoardView.hh"

namespace {

    int failures = 0;

    void Check(bool condition, const std::string &what) {
        if (condition) return;
        std::printf("  FAIL  %s\n", what.c_str());
        failures++;
    }

    constexpr int Sq(char file, int rank) {
        return (rank - 1) * GosChess::board_t::ROW_LENGTH + (file - 'a');
    }

    constexpr int LAST_ROW = GosChess::board_t::ROW_NUM - 1;
    constexpr int LAST_COLUMN = GosChess::board_t::ROW_LENGTH - 1;

    void ExpectCell(int board_index, int row, int column, const std::string &what) {
        const GosChess::view_cell_t cell = GosChess::BoardIndexToViewCell(board_index);
        Check(cell.y_ == row && cell.x_ == column, what);
    }

    void TestWhiteView() {
        std::printf("white's view\n");
        GosChess::SetPlayerColor(GosChess::color_t::WHITE);
        Check(!GosChess::BoardIsFlipped(), "white does not see a flipped board");

        ExpectCell(Sq('a', 1), LAST_ROW, 0, "a1 sits bottom left");
        ExpectCell(Sq('h', 1), LAST_ROW, LAST_COLUMN, "h1 sits bottom right");
        ExpectCell(Sq('a', 8), 0, 0, "a8 sits top left");
        ExpectCell(Sq('h', 8), 0, LAST_COLUMN, "h8 sits top right");
    }

    void TestBlackView() {
        std::printf("black's view\n");
        GosChess::SetPlayerColor(GosChess::color_t::BLACK);
        Check(GosChess::BoardIsFlipped(), "black sees a flipped board");

        // The same squares, seen from the other chair: the board is turned
        // through 180 degrees, so each corner swaps with the one opposite.
        ExpectCell(Sq('a', 1), 0, LAST_COLUMN, "a1 sits top right");
        ExpectCell(Sq('h', 1), 0, 0, "h1 sits top left");
        ExpectCell(Sq('a', 8), LAST_ROW, LAST_COLUMN, "a8 sits bottom right");
        ExpectCell(Sq('h', 8), LAST_ROW, 0, "h8 sits bottom left");
    }

    // The point of the whole exercise: whichever colour you are, your own men
    // start on the row nearest you.
    void TestOwnPiecesAreNearest() {
        std::printf("own pieces sit nearest\n");
        for (GosChess::color_t color: {GosChess::color_t::WHITE, GosChess::color_t::BLACK}) {
            GosChess::SetPlayerColor(color);
            const int own_back_rank = color == GosChess::color_t::WHITE ? 0 : LAST_ROW;
            const std::string name = color == GosChess::color_t::WHITE ? "white" : "black";
            for (int file = 0; file < GosChess::board_t::ROW_LENGTH; file++) {
                const int square = own_back_rank * GosChess::board_t::ROW_LENGTH + file;
                const GosChess::view_cell_t cell = GosChess::BoardIndexToViewCell(square);
                Check(cell.y_ == LAST_ROW, name + "'s back rank is drawn on the bottom row");
            }
        }
    }

    void TestRoundTrip() {
        std::printf("round trip\n");
        for (GosChess::color_t color: {GosChess::color_t::WHITE, GosChess::color_t::BLACK}) {
            GosChess::SetPlayerColor(color);
            for (int square = 0; square < GosChess::board_t::BOARD_SIZE; square++) {
                const GosChess::view_cell_t cell = GosChess::BoardIndexToViewCell(square);
                Check(GosChess::ViewCellOnBoard(cell), "every square maps onto the board");
                Check(GosChess::ViewCellToBoardIndex(cell) == square,
                      "a square survives the trip to the screen and back");
            }
        }
    }

    // A 180 degree turn keeps row + column parity, which is why the light and
    // dark squares no longer have to be repainted per player.
    void TestSquareColoursAgree() {
        std::printf("square colours\n");
        for (int square = 0; square < GosChess::board_t::BOARD_SIZE; square++) {
            GosChess::SetPlayerColor(GosChess::color_t::WHITE);
            const GosChess::view_cell_t white_cell = GosChess::BoardIndexToViewCell(square);
            GosChess::SetPlayerColor(GosChess::color_t::BLACK);
            const GosChess::view_cell_t black_cell = GosChess::BoardIndexToViewCell(square);
            Check((white_cell.y_ + white_cell.x_) % 2 == (black_cell.y_ + black_cell.x_) % 2,
                  "a square keeps its colour when the board is turned round");
        }
        // h1 is a light square, and the board is painted light when row plus
        // column is even, so the two have to line up.
        GosChess::SetPlayerColor(GosChess::color_t::WHITE);
        const GosChess::view_cell_t h1 = GosChess::BoardIndexToViewCell(Sq('h', 1));
        Check((h1.y_ + h1.x_) % 2 == 0, "h1 is painted as a light square");
    }

    void TestOffBoardCellsRejected() {
        std::printf("clicks beside the board\n");
        Check(!GosChess::ViewCellOnBoard({-1, 0}), "above the board is off the board");
        Check(!GosChess::ViewCellOnBoard({0, -1}), "left of the board is off the board");
        Check(!GosChess::ViewCellOnBoard({GosChess::board_t::ROW_NUM, 0}), "below the board is off the board");
        Check(!GosChess::ViewCellOnBoard({0, GosChess::board_t::ROW_LENGTH}), "right of the board is off the board");
        Check(GosChess::ViewCellOnBoard({0, 0}), "the top left corner is on the board");
        Check(GosChess::ViewCellOnBoard({LAST_ROW, LAST_COLUMN}), "the bottom right corner is on the board");
    }

}

int main() {
    TestWhiteView();
    TestBlackView();
    TestOwnPiecesAreNearest();
    TestRoundTrip();
    TestSquareColoursAgree();
    TestOffBoardCellsRejected();

    if (failures == 0) {
        std::printf("\nall checks passed\n");
        return 0;
    }
    std::printf("\n%d check(s) failed\n", failures);
    return 1;
}
