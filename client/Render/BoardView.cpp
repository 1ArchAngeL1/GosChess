#include "BoardView.hh"

#include "../Game/ClientGame.hh"

bool GosChess::BoardIsFlipped() {
    return GosChess::player_color == GosChess::color_t::BLACK;
}

GosChess::view_cell_t GosChess::BoardIndexToViewCell(int board_index) {
    int rank = board_index / GosChess::board_t::ROW_NUM;
    int file = board_index % GosChess::board_t::ROW_LENGTH;
    if (GosChess::BoardIsFlipped()) {
        rank = GosChess::board_t::ROW_NUM - rank - 1;
        file = GosChess::board_t::ROW_LENGTH - file - 1;
    }
    // Rank 0 belongs at the bottom of the drawn board, so ranks count back up
    // from the last row.
    return {GosChess::board_t::ROW_NUM - rank - 1, file};
}

int GosChess::ViewCellToBoardIndex(const GosChess::view_cell_t &cell) {
    int rank = GosChess::board_t::ROW_NUM - cell.y_ - 1;
    int file = cell.x_;
    if (GosChess::BoardIsFlipped()) {
        rank = GosChess::board_t::ROW_NUM - rank - 1;
        file = GosChess::board_t::ROW_LENGTH - file - 1;
    }
    return rank * GosChess::board_t::ROW_LENGTH + file;
}

bool GosChess::ViewCellOnBoard(const GosChess::view_cell_t &cell) {
    return GosChess::OnBoard(cell);
}
