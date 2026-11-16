#include "GosChess/Rules.hh"

#include <algorithm>
#include <cstdlib>

namespace {

    using namespace GosChess;

    constexpr unsigned char sliding_pieces[]{figure_types_t::BISHOP,
                                             figure_types_t::ROOK,
                                             figure_types_t::QUEEN};

    constexpr int8_t knight_moves_y[]{2, 2, 1, -1, 1, -1, -2, -2};
    constexpr int8_t knight_moves_x[]{1, -1, 2, 2, -2, -2, 1, -1};

    constexpr unsigned char promotion_pieces[]{figure_types_t::QUEEN,
                                               figure_types_t::ROOK,
                                               figure_types_t::BISHOP,
                                               figure_types_t::KNIGHT};

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

    constexpr int HomeRank(color_t color) {
        return color == color_t::WHITE ? 0 : board_t::ROW_NUM - 1;
    }

    constexpr int KingHomeSquare(color_t color) {
        return HomeRank(color) * board_t::ROW_LENGTH + 4;
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

    // An enemy pawn attacks `figure_index` from the squares this colour would
    // capture towards, which is why the ray start comes from the defender.
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

    typedef std::vector<action_t> move_list_t;

    void AddMove(move_list_t &out, int from, int to) {
        out.emplace_back(static_cast<std::int8_t>(from), static_cast<std::int8_t>(to));
    }

    // A pawn arriving on the far rank is four different moves, not one.
    void AddPawnMove(move_list_t &out, int from, int to, color_t color) {
        if (RankOf(to) != PromotionRank(color)) {
            AddMove(out, from, to);
            return;
        }
        for (unsigned char piece: promotion_pieces) {
            out.emplace_back(static_cast<std::int8_t>(from), static_cast<std::int8_t>(to), piece);
        }
    }

    void GenerateSlidingMoves(const unsigned char *board, figure_t piece, int index, color_t color,
                              move_list_t &out) {
        int direction_start_index = (piece.type_ == figure_types_t::BISHOP) ? 4 : 0;
        int direction_end_index = (piece.type_ == figure_types_t::ROOK) ? 4 : 8;

        for (int direction = direction_start_index; direction < direction_end_index; direction++) {
            for (int offset = 0; offset < precalculated_offsets[index][direction]; offset++) {
                int target_index = index + (offset + 1) * direction_offsets[direction];
                if (board[target_index] != 0 && figure_t(board[target_index]).color_ == color) break;
                AddMove(out, index, target_index);
                if (board[target_index] != 0) break;
            }
        }
    }

    void GeneratePawnCaptures(const board_t &board, figure_t piece, int index, color_t color,
                              move_list_t &out) {
        const unsigned char *squares = board.getRawBoard();
        const int start_index = PawnAttackRayStart(static_cast<color_t>(piece.color_));
        for (int direction_index: {start_index, start_index + 2}) {
            if (precalculated_offsets[index][direction_index] < 1) continue;
            const int target_index = index + direction_offsets[direction_index];
            if (squares[target_index] != 0) {
                if (figure_t(squares[target_index]).color_ != color) {
                    AddPawnMove(out, index, target_index, color);
                }
                continue;
            }
            // En passant: the only capture that lands on an empty square.
            if (target_index == board.enPassantTarget()) {
                AddMove(out, index, target_index);
            }
        }
    }

    void GeneratePawnMoves(const board_t &board, figure_t piece, int index, color_t color,
                           move_list_t &out) {
        if (piece.type_ != figure_types_t::PAWN) return;
        const unsigned char *squares = board.getRawBoard();
        const int multiplier = PawnPushDirection(static_cast<color_t>(piece.color_));
        const int one_forward = index + board_t::ROW_LENGTH * multiplier;
        if (IsSquare(one_forward) && squares[one_forward] == 0) {
            AddPawnMove(out, index, one_forward, color);
            const int two_forward = index + 2 * board_t::ROW_LENGTH * multiplier;
            if (RankOf(index) == PawnStartRank(static_cast<color_t>(piece.color_)) &&
                IsSquare(two_forward) && squares[two_forward] == 0) {
                AddMove(out, index, two_forward);
            }
        }
        GeneratePawnCaptures(board, piece, index, color, out);
    }

    bool IsRookOf(const unsigned char *squares, int square, color_t color) {
        if (squares[square] == 0) return false;
        const figure_t piece(squares[square]);
        return piece.type_ == figure_types_t::ROOK && piece.color_ == color;
    }

    // Castling is generated only when it is wholly legal, because "the king
    // may not pass through an attacked square" is a rule about squares the
    // king never ends up on, and so cannot be caught by playing the move and
    // looking at the result.
    void GenerateCastlingMoves(const board_t &board, color_t color, int king_square,
                               move_list_t &out) {
        const std::uint8_t rights = board.castlingRights();
        if (rights == NO_CASTLING) return;
        if (king_square != KingHomeSquare(color)) return;

        const unsigned char *squares = board.getRawBoard();
        if (AttackedBySomething(squares, color, king_square)) return;

        // The transit squares are tested on a board with the king lifted off
        // its home square, so that it cannot shield a square it is about to
        // walk onto. For a line attacker on the home rank this makes no
        // difference - anything whose ray reaches a transit square through the
        // king's square attacks the king too, and castling out of check is
        // already refused above - but testing a square the king is standing on
        // is the kind of shortcut that stops being safe the moment a piece
        // moves differently.
        std::array<unsigned char, board_t::BOARD_SIZE> without_king = board.position().squares;
        without_king[king_square] = 0;

        const int home_rank = HomeRank(color) * board_t::ROW_LENGTH;

        if (rights & KingSideRight(color)) {
            const int f_file = home_rank + 5;
            const int g_file = home_rank + 6;
            const int h_file = home_rank + 7;
            if (squares[f_file] == 0 && squares[g_file] == 0 && IsRookOf(squares, h_file, color) &&
                !AttackedBySomething(without_king.data(), color, f_file) &&
                !AttackedBySomething(without_king.data(), color, g_file)) {
                AddMove(out, king_square, g_file);
            }
        }

        if (rights & QueenSideRight(color)) {
            const int d_file = home_rank + 3;
            const int c_file = home_rank + 2;
            const int b_file = home_rank + 1;
            const int a_file = home_rank + 0;
            // b1 has to be empty but need not be safe - the king never stands on it.
            if (squares[d_file] == 0 && squares[c_file] == 0 && squares[b_file] == 0 &&
                IsRookOf(squares, a_file, color) &&
                !AttackedBySomething(without_king.data(), color, d_file) &&
                !AttackedBySomething(without_king.data(), color, c_file)) {
                AddMove(out, king_square, c_file);
            }
        }
    }

    void GenerateKingMoves(const board_t &board, figure_t piece, int index, color_t color,
                           move_list_t &out) {
        if (piece.type_ != figure_types_t::KING) return;
        const unsigned char *squares = board.getRawBoard();
        for (int i = 0; i < 8; i++) {
            if (precalculated_offsets[index][i] >= 1) {
                const int target_index = index + direction_offsets[i];
                if (squares[target_index] == 0 ||
                    figure_t(squares[target_index]).color_ != piece.color_) {
                    AddMove(out, index, target_index);
                }
            }
        }
        GenerateCastlingMoves(board, color, index, out);
    }

    void GenerateKnightMoves(const unsigned char *board, figure_t piece, int index, color_t color,
                             move_list_t &out) {
        if (piece.type_ != figure_types_t::KNIGHT) return;
        for (int i = 0; i < 8; i++) {
            const board_node_t curr = GetSquare(index);
            const board_node_t res = board_node_t(knight_moves_y[i], knight_moves_x[i]) + curr;
            if (!OnBoard(res)) continue;
            const int target_index = GetNumFromNode(res);
            if (board[target_index] == 0 || figure_t(board[target_index]).color_ != color) {
                AddMove(out, index, target_index);
            }
        }
    }

    // Which castling rights are given up by a piece leaving, or being taken
    // on, one of the four corner squares.
    std::uint8_t RightsLostBySquare(int square) {
        switch (square) {
            case 0:  return WHITE_QUEEN_SIDE;
            case 7:  return WHITE_KING_SIDE;
            case 56: return BLACK_QUEEN_SIDE;
            case 63: return BLACK_KING_SIDE;
            default: return NO_CASTLING;
        }
    }

    // Plays the move on the board, assuming it is one of `color`'s legal
    // moves. Everything special about chess moves happens here: the rook that
    // travels with a castling king, the pawn captured on a square the capturer
    // never visits, the piece a pawn turns into, and the three pieces of state
    // that outlive the move itself.
    void ApplyMove(action_t move, board_t &board, color_t color) {
        const position_t before = board.position();
        const figure_t mover(before.squares[move.move_from]);
        const bool pawn_move = mover.type_ == figure_types_t::PAWN;
        const bool king_move = mover.type_ == figure_types_t::KING;
        const bool captured_something = before.squares[move.move_to] != 0;
        const bool en_passant = pawn_move && move.move_to == before.en_passant_target &&
                                FileOf(move.move_from) != FileOf(move.move_to);

        unsigned char placed = mover.full_type;
        if (pawn_move && move.isPromotion()) {
            figure_t promoted;
            promoted.color_ = color;
            promoted.type_ = move.promotion;
            placed = promoted.full_type;
        }
        board.setFigure(move.move_from, 0);
        board.setFigure(move.move_to, placed);

        if (en_passant) {
            board.setFigure(move.move_to - board_t::ROW_LENGTH * PawnPushDirection(color), 0);
        }

        if (king_move && std::abs(FileOf(move.move_to) - FileOf(move.move_from)) == 2) {
            const int home_rank = RankOf(move.move_from) * board_t::ROW_LENGTH;
            const bool king_side = FileOf(move.move_to) > FileOf(move.move_from);
            const int rook_from = home_rank + (king_side ? 7 : 0);
            const int rook_to = home_rank + (king_side ? 5 : 3);
            const unsigned char rook = before.squares[rook_from];
            board.setFigure(rook_from, 0);
            board.setFigure(rook_to, rook);
        }

        // Only a double pawn push leaves an en passant square behind, and only
        // for the one move that follows it.
        board.setEnPassantTarget(-1);
        if (pawn_move && std::abs(RankOf(move.move_to) - RankOf(move.move_from)) == 2) {
            board.setEnPassantTarget(static_cast<std::int8_t>((move.move_from + move.move_to) / 2));
        }

        std::uint8_t lost = NO_CASTLING;
        if (king_move) lost |= KingSideRight(color) | QueenSideRight(color);
        lost |= RightsLostBySquare(move.move_from);
        lost |= RightsLostBySquare(move.move_to);
        board.clearCastlingRights(lost);

        if (pawn_move || captured_something || en_passant) board.setHalfmoveClock(0);
        else board.setHalfmoveClock(static_cast<std::uint16_t>(before.halfmove_clock + 1));
    }

}

void GosChess::GenerateMoveList(const GosChess::board_t &board, GosChess::color_t clr,
                                std::vector<GosChess::action_t> &out) {
    EnsureOffsets();
    out.clear();
    const unsigned char *squares = board.getRawBoard();
    for (int board_index = 0; board_index < board_t::BOARD_SIZE; board_index++) {
        if (squares[board_index] == 0) continue;
        figure_t curr_fig(squares[board_index]);
        if (curr_fig.color_ != clr) continue;
        if (IsSlidingPiece(curr_fig)) {
            GenerateSlidingMoves(squares, curr_fig, board_index, clr, out);
        } else if (IsPawn(curr_fig)) {
            GeneratePawnMoves(board, curr_fig, board_index, clr, out);
        } else if (IsKing(curr_fig)) {
            GenerateKingMoves(board, curr_fig, board_index, clr, out);
        } else if (IsKnight(curr_fig)) {
            GenerateKnightMoves(squares, curr_fig, board_index, clr, out);
        }
    }
}

void GosChess::CalculateAvailableMoves(const GosChess::board_t &board, GosChess::color_t clr,
                                       GosChess::action_map_t &out) {
    std::vector<action_t> moves;
    GenerateMoveList(board, clr, moves);
    out.clear();
    for (const action_t &move: moves) out[move.move_from].insert(move);
}

void GosChess::CalculateLegalMoves(GosChess::board_t &board, GosChess::color_t color,
                                   GosChess::action_map_t &out) {
    std::vector<action_t> moves;
    GenerateMoveList(board, color, moves);
    out.clear();
    for (const action_t &move: moves) {
        if (!TryMove(move, board, color)) continue;
        UndoMove(board);
        out[move.move_from].insert(move);
    }
}

bool GosChess::CanMakeMove(GosChess::action_t mv, const GosChess::actions_set_t &available_moves) {
    return available_moves.find(mv) != available_moves.end();
}

bool GosChess::MakeMove(GosChess::action_t mv, GosChess::board_t &board, GosChess::color_t color,
                        const GosChess::actions_set_t &available_moves) {
    if (!CanMakeMove(mv, available_moves)) return false;
    return TryMove(mv, board, color);
}

bool GosChess::TryMove(GosChess::action_t mv, GosChess::board_t &board, GosChess::color_t color) {
    EnsureOffsets();
    board.saveState();
    ApplyMove(mv, board, color);
    if (IsKingInCheck(board, color)) {
        GosChess::UndoMove(board);
        return false;
    }
    return true;
}

bool GosChess::UndoMove(GosChess::board_t &board) {
    return board.restoreState();
}

bool GosChess::IsKingInCheck(const GosChess::board_t &board, GosChess::color_t clr) {
    EnsureOffsets();
    const int king_index = GetKingPosition(board.getRawBoard(), clr);
    if (king_index < 0) return false;
    return AttackedBySomething(board.getRawBoard(), clr, king_index);
}

int GosChess::KingSquare(const GosChess::board_t &board, GosChess::color_t color) {
    return GetKingPosition(board.getRawBoard(), color);
}

bool GosChess::IsSquareAttacked(const unsigned char *board, int square, GosChess::color_t defender) {
    EnsureOffsets();
    if (!IsSquare(square)) return false;
    return AttackedBySomething(board, defender, square);
}

bool GosChess::HasNoLegalMoves(GosChess::board_t &board, GosChess::color_t color,
                               const GosChess::action_map_t &available_moves) {
    // The caller's map is ignored in favour of generating afresh: it may have
    // been built for a different position, and getting this wrong turns a live
    // game into a mate.
    (void) available_moves;
    EnsureOffsets();
    std::vector<action_t> moves;
    GenerateMoveList(board, color, moves);
    for (const action_t &move: moves) {
        if (!TryMove(move, board, color)) continue;
        UndoMove(board);
        return false;
    }
    return true;
}

bool GosChess::IsCheckmate(GosChess::board_t &board, GosChess::color_t color,
                           const GosChess::action_map_t &available_moves) {
    return HasNoLegalMoves(board, color, available_moves) && IsKingInCheck(board, color);
}

bool GosChess::IsStalemate(GosChess::board_t &board, GosChess::color_t color,
                           const GosChess::action_map_t &available_moves) {
    return HasNoLegalMoves(board, color, available_moves) && !IsKingInCheck(board, color);
}

bool GosChess::IsInsufficientMaterial(const GosChess::board_t &board) {
    int minors[2] = {0, 0};
    const unsigned char *squares = board.getRawBoard();
    for (int square = 0; square < board_t::BOARD_SIZE; square++) {
        if (squares[square] == 0) continue;
        const figure_t piece(squares[square]);
        switch (piece.type_) {
            case figure_types_t::PAWN:
            case figure_types_t::ROOK:
            case figure_types_t::QUEEN:
                // Any of these can still force a mate, so the game is alive.
                return false;
            case figure_types_t::BISHOP:
            case figure_types_t::KNIGHT:
                minors[piece.color_]++;
                break;
            default:
                break;
        }
    }
    // A lone minor piece cannot mate, and neither can one against one.
    return minors[color_t::WHITE] <= 1 && minors[color_t::BLACK] <= 1;
}

GosChess::castling_rook_move_t GosChess::CastlingRookMove(const GosChess::board_t &board,
                                                          GosChess::action_t move) {
    castling_rook_move_t rook_move;
    if (!IsSquare(move.move_from) || !IsSquare(move.move_to)) return rook_move;
    const figure_t mover = board.getFigure(move.move_from);
    if (mover.full_type == 0 || mover.type_ != figure_types_t::KING) return rook_move;
    if (std::abs(FileOf(move.move_to) - FileOf(move.move_from)) != 2) return rook_move;
    const int home_rank = RankOf(move.move_from) * board_t::ROW_LENGTH;
    const bool king_side = FileOf(move.move_to) > FileOf(move.move_from);
    rook_move.from = home_rank + (king_side ? 7 : 0);
    rook_move.to = home_rank + (king_side ? 5 : 3);
    return rook_move;
}

bool GosChess::IsEnPassantCapture(const GosChess::board_t &board, GosChess::action_t move) {
    if (!IsSquare(move.move_from) || !IsSquare(move.move_to)) return false;
    const figure_t mover = board.getFigure(move.move_from);
    if (mover.full_type == 0 || mover.type_ != figure_types_t::PAWN) return false;
    return move.move_to == board.enPassantTarget() &&
           FileOf(move.move_from) != FileOf(move.move_to);
}

std::uint64_t GosChess::Perft(GosChess::board_t &board, GosChess::color_t color, int depth) {
    if (depth <= 0) return 1;
    std::vector<action_t> moves;
    GenerateMoveList(board, color, moves);
    std::uint64_t nodes = 0;
    for (const action_t &move: moves) {
        if (!TryMove(move, board, color)) continue;
        nodes += depth == 1 ? 1 : Perft(board, OppositeColor(color), depth - 1);
        UndoMove(board);
    }
    return nodes;
}
