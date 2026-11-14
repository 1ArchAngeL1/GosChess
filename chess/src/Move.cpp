#include "GosChess/Move.hh"

constinit GosChess::offset_t GosChess::precalculated_offsets[GosChess::board_t::BOARD_SIZE]{};

const std::int8_t GosChess::direction_offsets[8]{8, -8, -1, 1, 9, -7, 7, -9};

void GosChess::GenerateOffsets() {
    for (int i = 0; i < GosChess::board_t::BOARD_SIZE; i++) {
        const int curr_y = i / GosChess::board_t::ROW_NUM;
        const int curr_x = i % GosChess::board_t::ROW_LENGTH;
        const std::int8_t north = GosChess::board_t::ROW_NUM - curr_y - 1;
        const std::int8_t south = curr_y;
        const std::int8_t west = curr_x;
        const std::int8_t east = GosChess::board_t::ROW_LENGTH - curr_x - 1;
        GosChess::precalculated_offsets[i] = GosChess::offset_t(north, south, west, east);
    }
}
