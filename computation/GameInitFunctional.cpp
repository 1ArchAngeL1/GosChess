

#include "GameInitFunctional.h"


GosChess::offsets *GosChess::precalculated_offsets = new GosChess::offsets[GosChess::Board::BOARD_SIZE];


void GosChess::GenerateOffsets() {
    int curr_y, curr_x;
    for (int i = 0; i < GosChess::Board::BOARD_SIZE; i++) {
        curr_y = i / GosChess::Board::ROW_NUM, curr_x = i % GosChess::Board::ROW_LENGTH;
        std::int8_t north = GosChess::Board::ROW_NUM - curr_y - 1, south = curr_y, west = curr_x,
                east = GosChess::Board::ROW_LENGTH - curr_x - 1;
        GosChess::precalculated_offsets[i] = GosChess::offsets(north, south, west, east);
    }
}







