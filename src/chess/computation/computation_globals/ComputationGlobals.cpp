

#include "ComputationGlobals.h"


std::unique_ptr<GosChess::Offset[]> GosChess::precalculated_offsets(new GosChess::Offset[GosChess::Board::BOARD_SIZE]);

const std::unique_ptr<const std::int8_t[]> GosChess::direction_offsets(new std::int8_t[]{8, -8, -1, 1, 9, -7, 7, -9});

std::unordered_map<std::int8_t, std::unordered_set<GosChess::Move, GosChess::MoveHash>> GosChess::available_moves;

std::unique_ptr<unsigned char[]> GosChess::sliding_pieces(new unsigned char[]{GosChess::FigureTypes::BISHOP,
                                                                    GosChess::FigureTypes::ROOK,
                                                                    GosChess::FigureTypes::QUEEN});

