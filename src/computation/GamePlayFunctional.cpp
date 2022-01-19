

#include "GamePlayFunctional.h"


GosChess::Offset *GosChess::precalculated_offsets = new GosChess::Offset[GosChess::Board::BOARD_SIZE];

const std::int8_t const *GosChess::direction_offsets = new std::int8_t[]{8, -8, -1, 1, 9, -7, 7, -9};


std::unordered_map<std::int8_t, std::unordered_set<GosChess::Move, GosChess::MoveHash>> GosChess::available_moves;


const unsigned char *GosChess::sliding_pieces = new unsigned char[]{GosChess::FigureTypes::BISHOP,
                                                                    GosChess::FigureTypes::ROOK,
                                                                    GosChess::FigureTypes::QUEEN};


static const std::int8_t const *KnightMoves = new std::int8_t[]{17, 10, -6, -15, -17, -10, 6, 15
};


static bool IsPawn(const GosChess::Figure &_fig) {
    return _fig.type == GosChess::FigureTypes::PAWN;
}


static bool IsKing(const GosChess::Figure &_fig) {
    return _fig.type == GosChess::FigureTypes::KING;
}

static bool IsKnight(const GosChess::Figure &_fig) {
    return _fig.type == GosChess::FigureTypes::KNIGHT;
}


static bool IsSlidingPiece(const GosChess::Figure &_fig) {
    for (int i = 0; i < 3; i++) {
        if (_fig.type == GosChess::sliding_pieces[i])return true;
    }
    return false;
}


static void GenerateSlidingMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    int direction_start_index = (piece.type == GosChess::FigureTypes::BISHOP) ? 4 : 0;
    int direction_end_index = (piece.type == GosChess::FigureTypes::ROOK) ? 4 : 8;

    for (int i = direction_start_index; i < direction_end_index; i++) {
        for (int j = 0; j < GosChess::precalculated_offsets[index][i]; j++) {
            int target_index = index + (j + 1) * GosChess::direction_offsets[i];
            if (board[target_index] != 0 && GosChess::Figure(board[target_index]).color == GosChess::color_to_play)
                break;
            else if (board[target_index] != 0) {
                GosChess::available_moves[index].insert(GosChess::Move(index, target_index));
                break;
            }
            GosChess::available_moves[index].insert(GosChess::Move(index, target_index));
        }
    }
}


static void GeneratePawnMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    if (piece.type != GosChess::FigureTypes::PAWN)return;
    int multiplier = piece.color == GosChess::Color::BLACK ? -1 : 1;
    if (index + 8 * multiplier < GosChess::Board::BOARD_SIZE && index + 8 * multiplier >= 0) {
        GosChess::available_moves[index].insert(GosChess::Move(index, index + 8 * multiplier));
    }
}

static void GenerateKingMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    if (piece.type != GosChess::FigureTypes::KING)return;
    for (int i = 0; i < 8; i++) {
        int curr_move = GosChess::direction_offsets[i];
        if (index + curr_move < GosChess::Board::BOARD_SIZE && index + curr_move >= 0) {
            GosChess::available_moves[index].insert(GosChess::Move(index, index + curr_move));
        }
    }
}

static void GenerateKnightMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    if (piece.type != GosChess::FigureTypes::KNIGHT)return;
    for (int i = 0; i < 8; i++) {
        int curr_move = KnightMoves[i];
        if (index + curr_move < GosChess::Board::BOARD_SIZE && index + curr_move >= 0) {
            GosChess::available_moves[index].insert(GosChess::Move(index, index + curr_move));
        }
    }
}


GosChess::Cell GosChess::GetNode(const int &num) {
    int x = num % GosChess::Board::ROW_LENGTH;
    int y = num / GosChess::Board::ROW_NUM;
    return {y, x};
}

void GosChess::GenerateOffsets() {
    int curr_y, curr_x;
    for (int i = 0; i < GosChess::Board::BOARD_SIZE; i++) {
        curr_y = i / GosChess::Board::ROW_NUM, curr_x = i % GosChess::Board::ROW_LENGTH;
        std::int8_t north = GosChess::Board::ROW_NUM - curr_y - 1, south = curr_y, west = curr_x,
                east = GosChess::Board::ROW_LENGTH - curr_x - 1;
        GosChess::precalculated_offsets[i] = GosChess::Offset(north, south, west, east);
    }
}

void GosChess::CalculateAvailableMoves(const unsigned char *game_board) {
    GosChess::available_moves.clear();
    for (int i = 0; i < Board::BOARD_SIZE; i++) {
        if (game_board[i] == 0)continue;
        Figure curr_fig(game_board[i]);
        if (curr_fig.color == GosChess::color_to_play) {
            if (IsSlidingPiece(curr_fig)) {
                GenerateSlidingMoves(game_board, curr_fig, i);
            } else if (IsPawn(curr_fig)) {
                GeneratePawnMoves(game_board, curr_fig, i);
            } else if (IsKing(curr_fig)) {
                GenerateKingMoves(game_board, curr_fig, i);
            } else if (IsKnight(curr_fig)) {
                GenerateKnightMoves(game_board, curr_fig, i);
            }
        }
    }
}


GosChess::Cell GosChess::GetNodeFromScreen(const float &_y, const float &_x) {
    int real_y = (int) (GosChess::window_height - _y) / (int) GosChess::square_size;
    int real_x = (int) _x / (int) GosChess::square_size;
    return GosChess::Cell(real_y, real_x);
}


int GosChess::GetNumFromNode(const GosChess::Cell &_cell) {
    return _cell.y * GosChess::Board::ROW_NUM + _cell.x;
}


bool GosChess::CanMakeMove(GosChess::Move mv) {
    return GosChess::available_moves[mv.move_from].find(mv) != GosChess::available_moves[mv.move_from].end();
}

void GosChess::MakeMove(GosChess::Move mv, GosChess::Board &brd) {
    if (CanMakeMove(mv)) {
        unsigned char current = brd.GetPosition(mv.move_from).full_type;
        brd.SetPosition(mv.move_from, 0);
        brd.SetPosition(mv.move_to, current);
    }
}


void GosChess::ChangeActiveColour() {
    if(GosChess::color_to_play == GosChess::Color::WHITE) {
        color_to_play = GosChess::Color::BLACK;
    } else {
        color_to_play = GosChess::Color::WHITE;
    }
}



