

#include "ComputationGlobals.h"


GosChess::Offset *GosChess::precalculated_offsets = new GosChess::Offset[GosChess::Board::BOARD_SIZE];

const std::int8_t *const GosChess::direction_offsets = new std::int8_t[]{8, -8, -1, 1, 9, -7, 7, -9};


std::unordered_map<std::int8_t, std::unordered_set<GosChess::Move, GosChess::MoveHash>> GosChess::available_moves;


const unsigned char *GosChess::sliding_pieces = new unsigned char[]{GosChess::FigureTypes::BISHOP,
                                                                    GosChess::FigureTypes::ROOK,
                                                                    GosChess::FigureTypes::QUEEN};

