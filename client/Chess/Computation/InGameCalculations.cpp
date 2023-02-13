

#include "InGameCalculations.hh"


static constexpr unsigned char sliding_pieces[]{GosChess::figure_types_t::BISHOP,
                                                GosChess::figure_types_t::ROOK,
                                                GosChess::figure_types_t::QUEEN};

static inline bool IsPawn(const GosChess::figure_t &fig) {
    return fig.type_ == GosChess::figure_types_t::PAWN;
}

static inline bool IsKing(const GosChess::figure_t &fig) {
    return fig.type_ == GosChess::figure_types_t::KING;
}

static inline bool IsKnight(const GosChess::figure_t &fig) {
    return fig.type_ == GosChess::figure_types_t::KNIGHT;
}

static inline bool IsSlidingPiece(const GosChess::figure_t &fig) {
    for (int i = 0; i < 3; i++) {
        if (fig.type_ == sliding_pieces[i])return true;
    }
    return false;
}

static inline int GetKingPosition(const unsigned char *board, GosChess::color_t clr) {
    GosChess::figure_t fig_check;
    for (int king_ind = 0; king_ind < GosChess::board_t::BOARD_SIZE; king_ind++) {
        fig_check.full_type = board[king_ind];
        if (fig_check.type_ == GosChess::figure_types_t::KING && fig_check.color_ == clr)return king_ind;
    }
    return -1;
}

static inline bool CheckIndexForSlidingPieceAttacks(const unsigned char *brd, GosChess::color_t color, int figure_index) {
    GosChess::figure_t test_fig;
    for (int direction_index = 0; direction_index < 8; direction_index++) {
        for (int offset = 0; offset < GosChess::precalculated_offsets[figure_index][direction_index]; offset++) {
            int target_index = figure_index + (offset + 1) * GosChess::direction_offsets[direction_index];
            test_fig.full_type = brd[target_index];;
            if (test_fig.full_type != 0) {
                if (test_fig.color_ != color) {
                    if (direction_index < 4) {
                        if (test_fig.type_ == GosChess::figure_types_t::QUEEN ||
                            test_fig.type_ == GosChess::figure_types_t::ROOK)
                            return true;
                    } else {
                        if (test_fig.type_ == GosChess::figure_types_t::QUEEN ||
                            test_fig.type_ == GosChess::figure_types_t::BISHOP)
                            return true;
                    }
                }
                break;
            }
        }
    }
    return false;
}

static inline bool CheckIndexForPawnAttacks(const unsigned char *brd, GosChess::color_t color, int figure_index) {
    GosChess::figure_t test_fig;
    int direction_start_index = color == GosChess::player_color ? 4 : 5;
    for (int direction_index = direction_start_index; direction_index < 8; direction_index += 2) {
        if (GosChess::precalculated_offsets[figure_index][direction_index] >= 1) {
            unsigned char target_fig = brd[figure_index + GosChess::direction_offsets[direction_index]];
            if (target_fig != 0) {
                test_fig.full_type = target_fig;
                if (test_fig.color_ != color && test_fig.type_ == GosChess::figure_types_t::PAWN)
                    return true;
            }

        }
    }
    return false;
}

static inline bool CheckIndexForKingAttacks(const unsigned char *brd, GosChess::color_t color, int figure_index) {
    GosChess::figure_t test_fig;
    for (int direction_index = 0; direction_index < 8; direction_index++) {
        if (GosChess::precalculated_offsets[figure_index][direction_index] >= 1) {
            unsigned char targ = brd[figure_index + GosChess::direction_offsets[direction_index]];
            if (targ != 0) {
                test_fig.full_type = targ;
                if (test_fig.color_ != color && test_fig.type_ == GosChess::figure_types_t::KING)return true;
            }
        }
    }
    return false;
}

static inline bool CheckIndexForKnightAttacks(const unsigned char *brd, GosChess::color_t color, int figure_index) {
    int additional_moves[] = {3, 3, 2, 2};
    GosChess::figure_t test_fig;
    for (int direction_index = 4; direction_index < 8; direction_index++) {
        if (GosChess::precalculated_offsets[figure_index][direction_index] >= 1) {
            int targ_index = figure_index + GosChess::direction_offsets[direction_index];
            if (direction_index % 2 == 0) {
                if (GosChess::precalculated_offsets[targ_index][0] >= 1) {
                    int curr_index = targ_index + GosChess::direction_offsets[0];
                    if (brd[curr_index] != 0) {
                        test_fig.full_type = brd[curr_index];
                        if (test_fig.color_ != color && test_fig.type_ == GosChess::figure_types_t::KNIGHT)return true;
                    }
                }
                if (GosChess::precalculated_offsets[targ_index][additional_moves[direction_index - 4]] >= 1) {
                    int curr_index = targ_index + GosChess::direction_offsets[additional_moves[direction_index - 4]];
                    if (brd[curr_index] != 0) {
                        test_fig.full_type = brd[curr_index];
                        if (test_fig.color_ != color && test_fig.type_ == GosChess::figure_types_t::KNIGHT)return true;
                    }
                }
            } else {
                if (GosChess::precalculated_offsets[targ_index][1] >= 1) {
                    int curr_index = targ_index + GosChess::direction_offsets[1];
                    if (brd[curr_index] != 0) {
                        test_fig.full_type = brd[curr_index];
                        if (test_fig.color_ != color && test_fig.type_ == GosChess::figure_types_t::KNIGHT)return true;
                    }
                }
                if (GosChess::precalculated_offsets[targ_index][additional_moves[direction_index - 4]] >= 1) {
                    int curr_index = targ_index + GosChess::direction_offsets[additional_moves[direction_index - 4]];
                    if (brd[curr_index] != 0) {
                        test_fig.full_type = brd[curr_index];
                        if (test_fig.color_ != color && test_fig.type_ == GosChess::figure_types_t::KNIGHT)return true;
                    }
                }
            }
        }
    }
    return false;
}

static inline bool CheckForKingCheck(const unsigned char *brd, GosChess::color_t clr) {
    int king_index = GetKingPosition(brd, clr);
    GosChess::figure_t arg_kng(brd[king_index]);
    GosChess::color_t king_color = static_cast<GosChess::color_t>(arg_kng.color_);
    return CheckIndexForPawnAttacks(brd, king_color, king_index) ||
           CheckIndexForSlidingPieceAttacks(brd, king_color, king_index) ||
           CheckIndexForKnightAttacks(brd, king_color, king_index) ||
           CheckIndexForKingAttacks(brd, king_color, king_index);
}

static inline void
GenerateSlidingMoves(const unsigned char *board, GosChess::figure_t piece, int index, GosChess::color_t color) {
    int direction_start_index = (piece.type_ == GosChess::figure_types_t::BISHOP) ? 4 : 0;
    int direction_end_index = (piece.type_ == GosChess::figure_types_t::ROOK) ? 4 : 8;

    for (int direction = direction_start_index; direction < direction_end_index; direction++) {
        for (int offset = 0; offset < GosChess::precalculated_offsets[index][direction]; offset++) {
            int target_index = index + (offset + 1) * GosChess::direction_offsets[direction];
            if (board[target_index] != 0 && GosChess::figure_t(board[target_index]).color_ == color) break;
            GosChess::game_available_moves[index].insert(GosChess::action_t(index, target_index));
            if (board[target_index] != 0) break;
        }
    }
}

static inline void
GeneratePawnKillMoves(const unsigned char *board, GosChess::figure_t piece, int index, GosChess::color_t color) {
    int start_index = piece.color_ == GosChess::player_color ? 4 : 5;
    if (GosChess::precalculated_offsets[index][start_index] >= 1) {
        unsigned char fig_rep = board[index + GosChess::direction_offsets[start_index]];
        if (fig_rep != 0) {
            GosChess::figure_t targ_fig(board[index + GosChess::direction_offsets[start_index]]);
            if (targ_fig.color_ != color) {
                GosChess::game_available_moves[index].insert(
                        GosChess::action_t(index, index + GosChess::direction_offsets[start_index]));
            }
        }
    }
    if (GosChess::precalculated_offsets[index][start_index + 2] >= 1) {
        unsigned char fig_rep = board[index + GosChess::direction_offsets[start_index + 2]];
        if (fig_rep != 0) {
            GosChess::figure_t targ_fig(board[index + GosChess::direction_offsets[start_index + 2]]);
            if (targ_fig.color_ != color) {
                GosChess::game_available_moves[index].insert(
                        GosChess::action_t(index, index + GosChess::direction_offsets[start_index + 2]));
            }
        }
    }
}

static inline void
GeneratePawnMoves(const unsigned char *board, GosChess::figure_t piece, int index, GosChess::color_t color) {
    if (piece.type_ != GosChess::figure_types_t::PAWN)return;
    int multiplier = piece.color_ == GosChess::enemy_color ? -1 : 1;
    if (index + 8 * multiplier < GosChess::board_t::BOARD_SIZE && index + 8 * multiplier >= 0) {
        if (board[index + 8 * multiplier] == 0)
            GosChess::game_available_moves[index].insert(GosChess::action_t(index, index + 8 * multiplier));
        if ((piece.color_ == GosChess::player_color && index / GosChess::board_t::ROW_NUM == 1) ||
            (piece.color_ == GosChess::enemy_color && index / GosChess::board_t::ROW_NUM == 6)) {
            if (board[index + 8 * multiplier] == 0 && board[index + 8 * 2 * multiplier] == 0)
                GosChess::game_available_moves[index].insert(GosChess::action_t(index, index + 8 * 2 * multiplier));
        }
    }
    GeneratePawnKillMoves(board, piece, index, color);
}

static inline void GenerateKingMoves(const unsigned char *board, GosChess::figure_t piece, int index) {
    if (piece.type_ != GosChess::figure_types_t::KING)return;
    for (int i = 0; i < 8; i++) {
        if (GosChess::precalculated_offsets[index][i] >= 1) {
            if (board[index + GosChess::direction_offsets[i]] == 0 ||
                GosChess::figure_t(board[index + GosChess::direction_offsets[i]]).color_ != piece.color_) {
                GosChess::game_available_moves[index].insert(
                        GosChess::action_t(index, index + GosChess::direction_offsets[i]));
            }
        }
    }
}

static inline void
GenerateKnightMoves(const unsigned char *board, GosChess::figure_t piece, int index, GosChess::color_t color) {
    static constexpr int8_t knight_moves_y[]{2, 2, 1, -1, 1, -1, -2, -2};
    static constexpr int8_t knight_moves_x[]{1, -1, 2, 2, -2, -2, 1, -1};
    if (piece.type_ != GosChess::figure_types_t::KNIGHT)return;
    for (int i = 0; i < 8; i++) {
        GosChess::board_node_t curr = GosChess::GetSquare(index);
        GosChess::board_node_t res = GosChess::board_node_t(knight_moves_y[i], knight_moves_x[i]) + curr;
        if ((res.x_ >= 0 && res.x_ < GosChess::board_t::ROW_LENGTH && res.y_ >= 0 && res.y_ < GosChess::board_t::ROW_NUM) &&
            (board[GosChess::GetNumFromNode(res)] == 0 ||
             GosChess::figure_t(board[GosChess::GetNumFromNode(res)]).color_ != color)) {
            GosChess::game_available_moves[index].insert(GosChess::action_t(index, GosChess::GetNumFromNode(res)));
        }
    }
}

GosChess::board_node_t GosChess::GetSquare(const int &num) {
    int x = num % GosChess::board_t::ROW_LENGTH;
    int y = num / GosChess::board_t::ROW_NUM;
    return {y, x};
}

void GosChess::GenerateOffsets() {
    int curr_y, curr_x;
    for (int i = 0; i < GosChess::board_t::BOARD_SIZE; i++) {
        curr_y = i / GosChess::board_t::ROW_NUM, curr_x = i % GosChess::board_t::ROW_LENGTH;
        std::int8_t north = GosChess::board_t::ROW_NUM - curr_y - 1, south = curr_y, west = curr_x,
                east = GosChess::board_t::ROW_LENGTH - curr_x - 1;
        GosChess::precalculated_offsets[i] = GosChess::offset_t(north, south, west, east);
    }
}

void GosChess::CalculateAvailableMoves(const unsigned char *game_board, GosChess::color_t clr) {
    GosChess::game_available_moves.clear();
    for (int board_index = 0; board_index < board_t::BOARD_SIZE; board_index++) {
        if (game_board[board_index] == 0)continue;
        figure_t curr_fig(game_board[board_index]);
        if (curr_fig.color_ == clr) {
            if (IsSlidingPiece(curr_fig)) {
                GenerateSlidingMoves(game_board, curr_fig, board_index, clr);
            } else if (IsPawn(curr_fig)) {
                GeneratePawnMoves(game_board, curr_fig, board_index, clr);
            } else if (IsKing(curr_fig)) {
                GenerateKingMoves(game_board, curr_fig, board_index);
            } else if (IsKnight(curr_fig)) {
                GenerateKnightMoves(game_board, curr_fig, board_index, clr);
            }
        }
    }
}

GosChess::board_node_t GosChess::GetNodeFromScreen(const float &y, const float &x) {
    int real_y = static_cast<int>(static_cast<float>(GosChess::window_height) - y - GosChess::board_offset_ver) /
                 static_cast<float>(GosChess::square_size);
    int real_x = static_cast<int>(x - GosChess::board_position.x) / static_cast<int>(GosChess::square_size);
    return GosChess::board_node_t(real_y, real_x);
}

int GosChess::GetNumFromNode(const GosChess::board_node_t &cell) {
    return cell.y_ * GosChess::board_t::ROW_NUM + cell.x_;
}

bool GosChess::CanMakeMove(GosChess::action_t mv, GosChess::actions_set_t &available_moves) {
    return available_moves.find(mv) != available_moves.end();
}

bool GosChess::MakeMove(GosChess::action_t mv, GosChess::board_t &board, GosChess::color_t color,
                        GosChess::actions_set_t &available_moves) {
    if (!CanMakeMove(mv, available_moves)) return false;
    board.saveState();
    unsigned char current = board.getFigure(mv.move_from).full_type;
    board.setFigure(mv.move_from, 0);
    board.setFigure(mv.move_to, current);
    if (CheckForKingCheck(board.getRawBoard(), color)) {
        GosChess::UndoMove(board);
        return false;
    }
    PromotePawns(board);
    return true;
}

bool GosChess::UndoMove(GosChess::board_t &board) {
    std::copy(board.game_rev_.top().begin(), board.game_rev_.top().end(), board.board_);
    board.game_rev_.pop();
}


void GosChess::ChangeActiveColour(GosChess::board_t &board) {
    GosChess::color_to_play = static_cast<GosChess::color_t>(!static_cast<int>(GosChess::color_to_play));
    GosChess::CalculateAvailableMoves(board.getRawBoard(), color_to_play);
}

bool GosChess::CheckMate(GosChess::board_t &board, GosChess::color_t color) {
    if (!GosChess::CheckForDraw(board, color)) return false;
    if (CheckForKingCheck(board.getRawBoard(), color)) {
        return true;
    }
}

bool GosChess::CheckForDraw(GosChess::board_t &board, GosChess::color_t color) {
    GosChess::figure_t test_fig;
    for (auto it = GosChess::game_available_moves.begin(); it != GosChess::game_available_moves.end(); it++) {
        test_fig.full_type = board.getRawBoard()[it->first];
        if (test_fig.color_ == color) {
            for (auto &move: GosChess::game_available_moves[it->first]) {
                if (GosChess::MakeMove(move, board, color, GosChess::game_available_moves[it->first])) {
                    GosChess::UndoMove(board);
                    return false;
                }
            }
        }
    }
    return true;
}

void GosChess::MakeMoveForce(GosChess::action_t mv, GosChess::board_t &board) {
    unsigned char current = board.getFigure(mv.move_from).full_type;
    board.setFigure(mv.move_from, 0);
    board.setFigure(mv.move_to, current);
    PromotePawns(board);
}

GosChess::action_t GosChess::InvertMove(GosChess::action_t move) {
    GosChess::board_node_t move_from = GosChess::GetSquare(move.move_from);
    GosChess::board_node_t move_to = GosChess::GetSquare(move.move_to);
    move_from.y_ = GosChess::board_t::ROW_NUM - move_from.y_ - 1;
    move_to.y_ = GosChess::board_t::ROW_NUM - move_to.y_ - 1;
    return GosChess::action_t(GosChess::GetNumFromNode(move_from), GosChess::GetNumFromNode(move_to));
}

bool GosChess::CheckIndexForAttackers(const unsigned char *board, const int &index) {
    GosChess::figure_t trg_fig(board[index]);
    GosChess::color_t figure_color = static_cast<GosChess::color_t>(trg_fig.color_);
    return CheckIndexForPawnAttacks(board, figure_color, index) ||
           CheckIndexForSlidingPieceAttacks(board, figure_color, index) ||
           CheckIndexForKnightAttacks(board, figure_color, index) ||
           CheckIndexForKingAttacks(board, figure_color, index);
}

void GosChess::PromotePawns(GosChess::board_t &board) {
    const unsigned char *raw_board = board.getRawBoard();
    figure_t test_fig;
    for (int i = 0; i < board_t::ROW_LENGTH; i++) {
        test_fig.full_type = raw_board[i];
        if (test_fig.type_ == GosChess::figure_types_t::PAWN && test_fig.color_ == enemy_color) {
            test_fig.type_ = GosChess::figure_types_t::QUEEN;
            board.setFigure(i, test_fig.full_type);
        }
    }
    for (int i = 0; i < board_t::ROW_LENGTH; i++) {
        int index = (board_t::ROW_NUM - 1) * board_t::ROW_LENGTH + i;
        test_fig.full_type = raw_board[index];
        if (test_fig.type_ == GosChess::figure_types_t::PAWN && test_fig.color_ == player_color) {
            test_fig.type_ = GosChess::figure_types_t::QUEEN;
            board.setFigure(index, test_fig.full_type);
        }
    }
}