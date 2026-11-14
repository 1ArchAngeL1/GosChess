#include "GosChess/Rules.hh"

#include <algorithm>

namespace {

    using namespace GosChess;

    constexpr unsigned char sliding_pieces[]{figure_types_t::BISHOP,
                                             figure_types_t::ROOK,
                                             figure_types_t::QUEEN};

    // The ray indices a pawn of the given colour captures along: White takes
    // towards higher ranks (NE/NW), Black towards lower ones (SE/SW). This is
    // the one place the board's absolute orientation is spelled out, and the
    // reason the rules no longer need to know who is looking at the board.
    constexpr int PawnAttackRayStart(color_t color) {
        return color == color_t::WHITE ? 4 : 5;
    }

    constexpr int PawnPushDirection(color_t color) {
        return color == color_t::WHITE ? 1 : -1;
    }

    constexpr int PawnStartRank(color_t color) {
        return color == color_t::WHITE ? 1 : board_t::ROW_NUM - 2;
    }

    constexpr int PromotionRank(color_t color) {
        return color == color_t::WHITE ? board_t::ROW_NUM - 1 : 0;
    }

    bool offsets_ready = false;

    void EnsureOffsets() {
        if (offsets_ready) return;
        GenerateOffsets();
        offsets_ready = true;
    }

    bool IsPawn(const figure_t &fig) { return fig.type_ == figure_types_t::PAWN; }

    bool IsKing(const figure_t &fig) { return fig.type_ == figure_types_t::KING; }

    bool IsKnight(const figure_t &fig) { return fig.type_ == figure_types_t::KNIGHT; }

    bool IsSlidingPiece(const figure_t &fig) {
        for (unsigned char sliding_piece: sliding_pieces) {
            if (fig.type_ == sliding_piece) return true;
        }
        return false;
    }

    int GetKingPosition(const unsigned char *board, color_t clr) {
        figure_t fig_check;
        for (int king_ind = 0; king_ind < board_t::BOARD_SIZE; king_ind++) {
            fig_check.full_type = board[king_ind];
            if (fig_check.type_ == figure_types_t::KING && fig_check.color_ == clr) return king_ind;
        }
        return -1;
    }

    bool CheckIndexForSlidingPieceAttacks(const unsigned char *brd, color_t color, int figure_index) {
        figure_t test_fig;
        for (int direction_index = 0; direction_index < 8; direction_index++) {
            for (int offset = 0; offset < precalculated_offsets[figure_index][direction_index]; offset++) {
                int target_index = figure_index + (offset + 1) * direction_offsets[direction_index];
                test_fig.full_type = brd[target_index];
                if (test_fig.full_type != 0) {
                    if (test_fig.color_ != color) {
                        if (direction_index < 4) {
                            if (test_fig.type_ == figure_types_t::QUEEN ||
                                test_fig.type_ == figure_types_t::ROOK)
                                return true;
                        } else {
                            if (test_fig.type_ == figure_types_t::QUEEN ||
                                test_fig.type_ == figure_types_t::BISHOP)
                                return true;
                        }
                    }
                    break;
                }
            }
        }
        return false;
    }

    // An enemy pawn attacks `figure_index` from the squares this piece's own
    // colour would capture towards, which is why the ray start is derived from
    // `color` rather than from the attacker.
    bool CheckIndexForPawnAttacks(const unsigned char *brd, color_t color, int figure_index) {
        figure_t test_fig;
        int direction_start_index = PawnAttackRayStart(color);
        for (int direction_index = direction_start_index; direction_index < 8; direction_index += 2) {
            if (precalculated_offsets[figure_index][direction_index] >= 1) {
                unsigned char target_fig = brd[figure_index + direction_offsets[direction_index]];
                if (target_fig != 0) {
                    test_fig.full_type = target_fig;
                    if (test_fig.color_ != color && test_fig.type_ == figure_types_t::PAWN)
                        return true;
                }
            }
        }
        return false;
    }

    bool CheckIndexForKingAttacks(const unsigned char *brd, color_t color, int figure_index) {
        figure_t test_fig;
        for (int direction_index = 0; direction_index < 8; direction_index++) {
            if (precalculated_offsets[figure_index][direction_index] >= 1) {
                unsigned char targ = brd[figure_index + direction_offsets[direction_index]];
                if (targ != 0) {
                    test_fig.full_type = targ;
                    if (test_fig.color_ != color && test_fig.type_ == figure_types_t::KING) return true;
                }
            }
        }
        return false;
    }

    bool CheckIndexForKnightAttacks(const unsigned char *brd, color_t color, int figure_index) {
        figure_t test_fig;
        const board_node_t from = GetSquare(figure_index);
        static constexpr int8_t knight_moves_y[]{2, 2, 1, -1, 1, -1, -2, -2};
        static constexpr int8_t knight_moves_x[]{1, -1, 2, 2, -2, -2, 1, -1};
        for (int i = 0; i < 8; i++) {
            const board_node_t target = board_node_t(knight_moves_y[i], knight_moves_x[i]) + from;
            if (!OnBoard(target)) continue;
            const unsigned char raw = brd[GetNumFromNode(target)];
            if (raw == 0) continue;
            test_fig.full_type = raw;
            if (test_fig.color_ != color && test_fig.type_ == figure_types_t::KNIGHT) return true;
        }
        return false;
    }

    bool AttackedBySomething(const unsigned char *board, color_t color, int index) {
        return CheckIndexForPawnAttacks(board, color, index) ||
               CheckIndexForSlidingPieceAttacks(board, color, index) ||
               CheckIndexForKnightAttacks(board, color, index) ||
               CheckIndexForKingAttacks(board, color, index);
    }

    void GenerateSlidingMoves(const unsigned char *board, figure_t piece, int index, color_t color,
                              action_map_t &out) {
        int direction_start_index = (piece.type_ == figure_types_t::BISHOP) ? 4 : 0;
        int direction_end_index = (piece.type_ == figure_types_t::ROOK) ? 4 : 8;

        for (int direction = direction_start_index; direction < direction_end_index; direction++) {
            for (int offset = 0; offset < precalculated_offsets[index][direction]; offset++) {
                int target_index = index + (offset + 1) * direction_offsets[direction];
                if (board[target_index] != 0 && figure_t(board[target_index]).color_ == color) break;
                out[index].insert(action_t(index, target_index));
                if (board[target_index] != 0) break;
            }
        }
    }

    void GeneratePawnKillMoves(const unsigned char *board, figure_t piece, int index, color_t color,
                               action_map_t &out) {
        const int start_index = PawnAttackRayStart(static_cast<color_t>(piece.color_));
        for (int direction_index: {start_index, start_index + 2}) {
            if (precalculated_offsets[index][direction_index] < 1) continue;
            const int target_index = index + direction_offsets[direction_index];
            if (board[target_index] == 0) continue;
            if (figure_t(board[target_index]).color_ != color) {
                out[index].insert(action_t(index, target_index));
            }
        }
    }

    void GeneratePawnMoves(const unsigned char *board, figure_t piece, int index, color_t color,
                           action_map_t &out) {
        if (piece.type_ != figure_types_t::PAWN) return;
        const int multiplier = PawnPushDirection(static_cast<color_t>(piece.color_));
        const int one_forward = index + board_t::ROW_LENGTH * multiplier;
        if (one_forward < board_t::BOARD_SIZE && one_forward >= 0) {
            if (board[one_forward] == 0) {
                out[index].insert(action_t(index, one_forward));
                const int two_forward = index + 2 * board_t::ROW_LENGTH * multiplier;
                if (index / board_t::ROW_NUM == PawnStartRank(static_cast<color_t>(piece.color_)) &&
                    board[two_forward] == 0) {
                    out[index].insert(action_t(index, two_forward));
                }
            }
        }
        GeneratePawnKillMoves(board, piece, index, color, out);
    }

    void GenerateKingMoves(const unsigned char *board, figure_t piece, int index, action_map_t &out) {
        if (piece.type_ != figure_types_t::KING) return;
        for (int i = 0; i < 8; i++) {
            if (precalculated_offsets[index][i] >= 1) {
                const int target_index = index + direction_offsets[i];
                if (board[target_index] == 0 ||
                    figure_t(board[target_index]).color_ != piece.color_) {
                    out[index].insert(action_t(index, target_index));
                }
            }
        }
    }

    void GenerateKnightMoves(const unsigned char *board, figure_t piece, int index, color_t color,
                             action_map_t &out) {
        static constexpr int8_t knight_moves_y[]{2, 2, 1, -1, 1, -1, -2, -2};
        static constexpr int8_t knight_moves_x[]{1, -1, 2, 2, -2, -2, 1, -1};
        if (piece.type_ != figure_types_t::KNIGHT) return;
        for (int i = 0; i < 8; i++) {
            const board_node_t curr = GetSquare(index);
            const board_node_t res = board_node_t(knight_moves_y[i], knight_moves_x[i]) + curr;
            if (!OnBoard(res)) continue;
            const int target_index = GetNumFromNode(res);
            if (board[target_index] == 0 || figure_t(board[target_index]).color_ != color) {
                out[index].insert(action_t(index, target_index));
            }
        }
    }

}

void GosChess::CalculateAvailableMoves(const unsigned char *game_board, GosChess::color_t clr,
                                       GosChess::action_map_t &out) {
    EnsureOffsets();
    out.clear();
    for (int board_index = 0; board_index < board_t::BOARD_SIZE; board_index++) {
        if (game_board[board_index] == 0) continue;
        figure_t curr_fig(game_board[board_index]);
        if (curr_fig.color_ != clr) continue;
        if (IsSlidingPiece(curr_fig)) {
            GenerateSlidingMoves(game_board, curr_fig, board_index, clr, out);
        } else if (IsPawn(curr_fig)) {
            GeneratePawnMoves(game_board, curr_fig, board_index, clr, out);
        } else if (IsKing(curr_fig)) {
            GenerateKingMoves(game_board, curr_fig, board_index, out);
        } else if (IsKnight(curr_fig)) {
            GenerateKnightMoves(game_board, curr_fig, board_index, clr, out);
        }
    }
}

bool GosChess::CanMakeMove(GosChess::action_t mv, const GosChess::actions_set_t &available_moves) {
    return available_moves.find(mv) != available_moves.end();
}

bool GosChess::MakeMove(GosChess::action_t mv, GosChess::board_t &board, GosChess::color_t color,
                        const GosChess::actions_set_t &available_moves) {
    EnsureOffsets();
    if (!CanMakeMove(mv, available_moves)) return false;
    board.saveState();
    unsigned char current = board.getFigure(mv.move_from).full_type;
    board.setFigure(mv.move_from, 0);
    board.setFigure(mv.move_to, current);
    if (IsKingInCheck(board.getRawBoard(), color)) {
        GosChess::UndoMove(board);
        return false;
    }
    PromotePawns(board);
    return true;
}

void GosChess::MakeMoveForce(GosChess::action_t mv, GosChess::board_t &board) {
    unsigned char current = board.getFigure(mv.move_from).full_type;
    board.setFigure(mv.move_from, 0);
    board.setFigure(mv.move_to, current);
    PromotePawns(board);
}

bool GosChess::UndoMove(GosChess::board_t &board) {
    return board.restoreState();
}

bool GosChess::IsKingInCheck(const unsigned char *board, GosChess::color_t clr) {
    EnsureOffsets();
    const int king_index = GetKingPosition(board, clr);
    if (king_index < 0) return false;
    return AttackedBySomething(board, clr, king_index);
}

bool GosChess::HasNoLegalMoves(GosChess::board_t &board, GosChess::color_t color,
                               const GosChess::action_map_t &available_moves) {
    EnsureOffsets();
    figure_t test_fig;
    for (const auto &[from_square, moves]: available_moves) {
        test_fig.full_type = board.getRawBoard()[from_square];
        if (test_fig.color_ != color) continue;
        for (const auto &move: moves) {
            if (GosChess::MakeMove(move, board, color, moves)) {
                GosChess::UndoMove(board);
                return false;
            }
        }
    }
    return true;
}

bool GosChess::IsCheckmate(GosChess::board_t &board, GosChess::color_t color,
                           const GosChess::action_map_t &available_moves) {
    return HasNoLegalMoves(board, color, available_moves) && IsKingInCheck(board.getRawBoard(), color);
}

bool GosChess::IsStalemate(GosChess::board_t &board, GosChess::color_t color,
                           const GosChess::action_map_t &available_moves) {
    return HasNoLegalMoves(board, color, available_moves) && !IsKingInCheck(board.getRawBoard(), color);
}

bool GosChess::IsSquareAttacked(const unsigned char *board, const int &index) {
    EnsureOffsets();
    figure_t trg_fig(board[index]);
    return AttackedBySomething(board, static_cast<color_t>(trg_fig.color_), index);
}

void GosChess::PromotePawns(GosChess::board_t &board) {
    const unsigned char *raw_board = board.getRawBoard();
    figure_t test_fig;
    for (color_t color: {color_t::WHITE, color_t::BLACK}) {
        const int rank = PromotionRank(color);
        for (int file = 0; file < board_t::ROW_LENGTH; file++) {
            const int index = rank * board_t::ROW_LENGTH + file;
            test_fig.full_type = raw_board[index];
            if (test_fig.full_type == 0) continue;
            if (test_fig.type_ == figure_types_t::PAWN && test_fig.color_ == color) {
                test_fig.type_ = figure_types_t::QUEEN;
                board.setFigure(index, test_fig.full_type);
            }
        }
    }
}
