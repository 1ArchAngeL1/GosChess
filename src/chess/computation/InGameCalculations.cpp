

#include "InGameCalculations.h"


static constexpr unsigned char sliding_pieces[]{GosChess::FigureTypes::BISHOP,
                                                GosChess::FigureTypes::ROOK,
                                                GosChess::FigureTypes::QUEEN};


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
        if (_fig.type == sliding_pieces[i])return true;
    }
    return false;
}

static int GetKingPosition(const unsigned char *board, GosChess::Color clr) {
    GosChess::Figure fig_check;
    for (int king_ind = 0; king_ind < GosChess::Board::BOARD_SIZE; king_ind++) {
        fig_check.full_type = board[king_ind];
        if (fig_check.type == GosChess::FigureTypes::KING && fig_check.color == clr)return king_ind;
    }
    return -1;
}

static bool CheckIndexForSlidingPieceAttacks(const unsigned char *brd, GosChess::Color color, int figure_index) {
    GosChess::Figure test_fig;
    for (int direction_index = 0; direction_index < 8; direction_index++) {
        for (int offset = 0; offset < GosChess::precalculated_offsets[figure_index][direction_index]; offset++) {
            int target_index = figure_index + (offset + 1) * GosChess::direction_offsets[direction_index];
            test_fig.full_type = brd[target_index];;
            if (test_fig.full_type != 0) {
                if (test_fig.color != color) {
                    if (direction_index < 4) {
                        if (test_fig.type == GosChess::FigureTypes::QUEEN ||
                            test_fig.type == GosChess::FigureTypes::ROOK)
                            return true;
                    } else {
                        if (test_fig.type == GosChess::FigureTypes::QUEEN ||
                            test_fig.type == GosChess::FigureTypes::BISHOP)
                            return true;
                    }
                }
                break;
            }
        }
    }
    return false;
}

static bool CheckIndexForPawnAttacks(const unsigned char *brd, GosChess::Color color, int figure_index) {
    GosChess::Figure test_fig;
    int direction_start_index = color == GosChess::player_color ? 4 : 5;
    for (int direction_index = direction_start_index; direction_index < 8; direction_index += 2) {
        if (GosChess::precalculated_offsets[figure_index][direction_index] >= 1) {
            unsigned char target_fig = brd[figure_index + GosChess::direction_offsets[direction_index]];
            if (target_fig != 0) {
                test_fig.full_type = target_fig;
                if (test_fig.color != color && test_fig.type == GosChess::FigureTypes::PAWN)
                    return true;
            }

        }
    }
    return false;
}

static bool CheckIndexForKingAttacks(const unsigned char *brd, GosChess::Color color, int figure_index) {
    GosChess::Figure test_fig;
    for (int direction_index = 0; direction_index < 8; direction_index++) {
        if (GosChess::precalculated_offsets[figure_index][direction_index] >= 1) {
            unsigned char targ = brd[figure_index + GosChess::direction_offsets[direction_index]];
            if (targ != 0) {
                test_fig.full_type = targ;
                if (test_fig.color != color && test_fig.type == GosChess::FigureTypes::KING)return true;
            }
        }
    }
    return false;
}

static bool CheckIndexForKnightAttacks(const unsigned char *brd, GosChess::Color color, int figure_index) {
    int additional_moves[] = {3, 3, 2, 2};
    GosChess::Figure test_fig;
    for (int direction_index = 4; direction_index < 8; direction_index++) {
        if (GosChess::precalculated_offsets[figure_index][direction_index] >= 1) {
            int targ_index = figure_index + GosChess::direction_offsets[direction_index];
            if (direction_index % 2 == 0) {
                if (GosChess::precalculated_offsets[targ_index][0] >= 1) {
                    int curr_index = targ_index + GosChess::direction_offsets[0];
                    if (brd[curr_index] != 0) {
                        test_fig.full_type = brd[curr_index];
                        if (test_fig.color != color && test_fig.type == GosChess::FigureTypes::KNIGHT)return true;
                    }
                }
                if (GosChess::precalculated_offsets[targ_index][additional_moves[direction_index - 4]] >= 1) {
                    int curr_index = targ_index + GosChess::direction_offsets[additional_moves[direction_index - 4]];
                    if (brd[curr_index] != 0) {
                        test_fig.full_type = brd[curr_index];
                        if (test_fig.color != color && test_fig.type == GosChess::FigureTypes::KNIGHT)return true;
                    }
                }
            } else {
                if (GosChess::precalculated_offsets[targ_index][1] >= 1) {
                    int curr_index = targ_index + GosChess::direction_offsets[1];
                    if (brd[curr_index] != 0) {
                        test_fig.full_type = brd[curr_index];
                        if (test_fig.color != color && test_fig.type == GosChess::FigureTypes::KNIGHT)return true;
                    }
                }
                if (GosChess::precalculated_offsets[targ_index][additional_moves[direction_index - 4]] >= 1) {
                    int curr_index = targ_index + GosChess::direction_offsets[additional_moves[direction_index - 4]];
                    if (brd[curr_index] != 0) {
                        test_fig.full_type = brd[curr_index];
                        if (test_fig.color != color && test_fig.type == GosChess::FigureTypes::KNIGHT)return true;
                    }
                }
            }
        }
    }
    return false;
}

static bool CheckForKingCheck(const unsigned char *brd, GosChess::Color clr) {
    int king_index = GetKingPosition(brd, clr);
    GosChess::Figure arg_kng(brd[king_index]);
    GosChess::Color king_color = static_cast<GosChess::Color>(arg_kng.color);
    return CheckIndexForPawnAttacks(brd, king_color, king_index) ||
           CheckIndexForSlidingPieceAttacks(brd, king_color, king_index) ||
           CheckIndexForKnightAttacks(brd, king_color, king_index) ||
           CheckIndexForKingAttacks(brd, king_color, king_index);
}


static void GenerateSlidingMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    int direction_start_index = (piece.type == GosChess::FigureTypes::BISHOP) ? 4 : 0;
    int direction_end_index = (piece.type == GosChess::FigureTypes::ROOK) ? 4 : 8;

    for (int i = direction_start_index; i < direction_end_index; i++) {
        for (int offset = 0; offset < GosChess::precalculated_offsets[index][i]; offset++) {
            int target_index = index + (offset + 1) * GosChess::direction_offsets[i];
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

static void GeneratePawnKillMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    int start_index = piece.color == GosChess::player_color ? 4 : 5;
    if (GosChess::precalculated_offsets[index][start_index] >= 1) {
        unsigned char fig_rep = board[index + GosChess::direction_offsets[start_index]];
        if (fig_rep != 0) {
            GosChess::Figure targ_fig(board[index + GosChess::direction_offsets[start_index]]);
            if (targ_fig.color != GosChess::color_to_play) {
                GosChess::available_moves[index].insert(
                        GosChess::Move(index, index + GosChess::direction_offsets[start_index]));
            }
        }
    }
    if (GosChess::precalculated_offsets[index][start_index + 2] >= 1) {
        unsigned char fig_rep = board[index + GosChess::direction_offsets[start_index + 2]];
        if (fig_rep != 0) {
            GosChess::Figure targ_fig(board[index + GosChess::direction_offsets[start_index + 2]]);
            if (targ_fig.color != GosChess::color_to_play) {
                GosChess::available_moves[index].insert(
                        GosChess::Move(index, index + GosChess::direction_offsets[start_index + 2]));
            }
        }
    }
}

static void GeneratePawnMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    if (piece.type != GosChess::FigureTypes::PAWN)return;
    int multiplier = piece.color == GosChess::enemy_color ? -1 : 1;
    if (index + 8 * multiplier < GosChess::Board::BOARD_SIZE && index + 8 * multiplier >= 0) {
        if (board[index + 8 * multiplier] == 0)
            GosChess::available_moves[index].insert(GosChess::Move(index, index + 8 * multiplier));
        if ((piece.color == GosChess::player_color && index / GosChess::Board::ROW_NUM == 1) ||
            (piece.color == GosChess::enemy_color && index / GosChess::Board::ROW_NUM == 6)) {
            if (board[index + 8 * multiplier] == 0 && board[index + 8 * 2 * multiplier] == 0)
                GosChess::available_moves[index].insert(GosChess::Move(index, index + 8 * 2 * multiplier));
        }
    }
    GeneratePawnKillMoves(board, piece, index);
}

static void GenerateKingMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    if (piece.type != GosChess::FigureTypes::KING)return;
    for (int i = 0; i < 8; i++) {
        if (GosChess::precalculated_offsets[index][i] >= 1) {
            if (board[index + GosChess::direction_offsets[i]] == 0 ||
                GosChess::Figure(board[index + GosChess::direction_offsets[i]]).color != piece.color) {
                GosChess::available_moves[index].insert(
                        GosChess::Move(index, index + GosChess::direction_offsets[i]));
            }
        }
    }
}

static void GenerateKnightMoves(const unsigned char *board, GosChess::Figure piece, int index) {
    static constexpr int8_t knight_moves_y[]{2, 2, 1, -1, 1, -1, -2, -2};
    static constexpr int8_t knight_moves_x[]{1, -1, 2, 2, -2, -2, 1, -1};
    if (piece.type != GosChess::FigureTypes::KNIGHT)return;
    for (int i = 0; i < 8; i++) {
        GosChess::Vector2i curr = GosChess::GetSquare(index);
        GosChess::Vector2i res = GosChess::Vector2i(knight_moves_y[i], knight_moves_x[i]) + curr;
        if ((res.x >= 0 && res.x < GosChess::Board::ROW_LENGTH && res.y >= 0 && res.y < GosChess::Board::ROW_NUM) &&
            (board[GosChess::GetNumFromNode(res)] == 0 ||
             GosChess::Figure(board[GosChess::GetNumFromNode(res)]).color != GosChess::color_to_play)) {
            GosChess::available_moves[index].insert(GosChess::Move(index, GosChess::GetNumFromNode(res)));
        }
    }
}

GosChess::Vector2i GosChess::GetSquare(const int &num) {
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

void GosChess::CalculateAvailableMoves(const unsigned char *game_board, GosChess::Color clr) {
    GosChess::available_moves.clear();
    for (int i = 0; i < Board::BOARD_SIZE; i++) {
        if (game_board[i] == 0)continue;
        Figure curr_fig(game_board[i]);
        if (curr_fig.color == clr) {
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

GosChess::Vector2i GosChess::GetNodeFromScreen(const float &_y, const float &_x) {
    int real_y = static_cast<int>(static_cast<float>(GosChess::window_height) - _y - GosChess::board_offset_ver) /
                 static_cast<float>(GosChess::square_size);
    int real_x = static_cast<int>(_x - GosChess::board_position.x) / static_cast<int>(GosChess::square_size);
    return GosChess::Vector2i(real_y, real_x);
}

int GosChess::GetNumFromNode(const GosChess::Vector2i &cell) {
    return cell.y * GosChess::Board::ROW_NUM + cell.x;
}

bool GosChess::CanMakeMove(GosChess::Move mv) {
    return GosChess::available_moves[mv.move_from].find(mv) != GosChess::available_moves[mv.move_from].end();
}


bool GosChess::MakeMove(GosChess::Move mv, GosChess::Board &brd) {
    if (!CanMakeMove(mv)) return false;
    brd.SaveState();
    unsigned char current = brd.At(mv.move_from).full_type;
    brd.SetPosition(mv.move_from, 0);
    brd.SetPosition(mv.move_to, current);
    if (CheckForKingCheck(brd.GetRawBoard(), GosChess::color_to_play)) {
        brd.Undo();
        return false;
    }
    return true;
}


void GosChess::ChangeActiveColour(GosChess::Board &board) {
    GosChess::color_to_play = static_cast<GosChess::Color>(!static_cast<int>(GosChess::color_to_play));
    GosChess::CalculateAvailableMoves(board.GetRawBoard(), color_to_play);
}


bool GosChess::CheckMate(GosChess::Board &brd, GosChess::Color clr) {
    GosChess::Figure test_fig;
    for (auto it = GosChess::available_moves.begin(); it != GosChess::available_moves.end(); it++) {
        test_fig.full_type = brd.GetRawBoard()[it->first];
        if (test_fig.color == clr) {
            for (auto &move: GosChess::available_moves[it->first]) {
                if (GosChess::MakeMove(move, brd)) {
                    brd.Undo();
                    return false;
                }
            }
        }
    }
    return true;
}

void GosChess::MakeMoveForce(GosChess::Move mv, GosChess::Board &board) {
    unsigned char current = board.At(mv.move_from).full_type;
    board.SetPosition(mv.move_from, 0);
    board.SetPosition(mv.move_to, current);
}


GosChess::Move GosChess::InvertMove(GosChess::Move move) {
    GosChess::Vector2i move_from = GosChess::GetSquare(move.move_from);
    GosChess::Vector2i move_to = GosChess::GetSquare(move.move_to);
    move_from.y = GosChess::Board::ROW_NUM - move_from.y - 1;
    move_to.y = GosChess::Board::ROW_NUM - move_to.y - 1;
    return GosChess::Move(GosChess::GetNumFromNode(move_from), GosChess::GetNumFromNode(move_to));
}

bool GosChess::CheckIndexForAttackets(const unsigned char *board, const int &index) {
    GosChess::Figure trg_fig(board[index]);
    GosChess::Color king_color = static_cast<GosChess::Color>(trg_fig.color);
    return CheckIndexForPawnAttacks(board, king_color, index) ||
           CheckIndexForSlidingPieceAttacks(board, king_color, index) ||
           CheckIndexForKnightAttacks(board, king_color, index) ||
           CheckIndexForKingAttacks(board, king_color, index);

}