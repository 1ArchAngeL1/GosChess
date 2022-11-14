

#include "ComputationGlobals.hh"


GosChess::offset_t GosChess::precalculated_offsets[GosChess::board_t::BOARD_SIZE];

const std::int8_t GosChess::direction_offsets[]{8, -8, -1, 1, 9, -7, 7, -9};

GosChess::action_map_t GosChess::game_available_moves;
