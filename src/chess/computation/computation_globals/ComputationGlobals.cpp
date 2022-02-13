

#include "ComputationGlobals.h"


GosChess::Offset GosChess::precalculated_offsets[GosChess::Board::BOARD_SIZE];

const std::int8_t GosChess::direction_offsets[]{8, -8, -1, 1, 9, -7, 7, -9};

std::unordered_map<std::int8_t, std::unordered_set<GosChess::Move, GosChess::MoveHash>> GosChess::available_moves;

unsigned char GosChess::sliding_pieces[]{GosChess::FigureTypes::BISHOP,
                                         GosChess::FigureTypes::ROOK,
                                         GosChess::FigureTypes::QUEEN};


