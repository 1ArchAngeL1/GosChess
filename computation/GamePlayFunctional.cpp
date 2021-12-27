

#include "GamePlayFunctional.h"


GosChess::Offset *GosChess::PrecalculatedOffsets = new GosChess::Offset[GosChess::Board::BOARD_SIZE];

const std::int8_t *GosChess::DirectionOffsets = new std::int8_t[]{8, -8, -1, 1, 9, -7, 7, -9};

std::unordered_map<std::int8_t, std::vector<GosChess::Move>> GosChess::available_moves;

const unsigned char * GosChess::SlidingPieces = new unsigned char[]{GosChess::FigureTypes::BISHOP,
                                                          GosChess::FigureTypes::ROOK,
                                                          GosChess::FigureTypes::QUEEN};

static bool isSlidingPiece(const unsigned char & figure) {
    for(int i = 0;i < 3;i++) {
        if((int)figure == GosChess::SlidingPieces[i])return true;
    }
    return false;
}

static void GenerateSlidingMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    int direction_start_index = (piece.type == GosChess::FigureTypes::BISHOP) ? 4 : 0;
    int direction_end_index = (piece.type == GosChess::FigureTypes::ROOK) ? 4 : 8;

    for (int i = direction_start_index; i < direction_end_index; i++) {
        for (int j = 0; j < GosChess::PrecalculatedOffsets[index][i]; j++) {
            int target_index = index + (j + 1) * GosChess::DirectionOffsets[direction_start_index];
            if (board[target_index] != 0 && GosChess::Figure(board[target_index]).color == GosChess::color_to_play)
                break;
            else if(board[target_index] != 0) {
                GosChess::available_moves[index].push_back(GosChess::Move(index, target_index));
                break;
            }
            GosChess::available_moves[index].push_back(GosChess::Move(index, target_index));
        }
    }
}

void GosChess::GenerateOffsets() {
    int curr_y, curr_x;
    for (int i = 0; i < GosChess::Board::BOARD_SIZE; i++) {
        curr_y = i / GosChess::Board::ROW_NUM, curr_x = i % GosChess::Board::ROW_LENGTH;
        std::int8_t north = GosChess::Board::ROW_NUM - curr_y - 1, south = curr_y, west = curr_x,
                east = GosChess::Board::ROW_LENGTH - curr_x - 1;
        GosChess::PrecalculatedOffsets[i] = GosChess::Offset(north, south, west, east);
    }
}

void GosChess::calculateAvailableMoves(const unsigned char *game_board) {
    GosChess::available_moves.clear();
    for (int i = 0; i < Board::BOARD_SIZE; i++) {
        if (game_board[i] == 0)continue;
        Figure curr_fig(game_board[i]);
        if (curr_fig.color == GosChess::color_to_play) {
            if(isSlidingPiece(game_board[i])) {
                GenerateSlidingMoves(game_board,curr_fig,i);
            }
        }
    }
}









