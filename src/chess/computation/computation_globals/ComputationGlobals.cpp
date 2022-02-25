

#include "ComputationGlobals.h"


GosChess::Offset GosChess::precalculated_offsets[GosChess::Board::BOARD_SIZE];

const std::int8_t GosChess::direction_offsets[]{8, -8, -1, 1, 9, -7, 7, -9};

GosChess::MoveLocker GosChess::game_available_moves;

GosChess::MoveLocker GosChess::ai_available_moves;
