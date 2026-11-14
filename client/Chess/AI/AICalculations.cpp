//
// Created by Leo Gogichaishvili on 14.02.22.
//

#include "AICalculations.hh"

#include <algorithm>
#include <climits>
#include <unordered_map>
#include <vector>

#include "GosChess/Rules.hh"

typedef GosChess::figure_types_t FIG;

static std::unordered_map<unsigned char, short> figure_values{{FIG::PAWN,   10},
                                                              {FIG::KNIGHT, 30},
                                                              {FIG::BISHOP, 30},
                                                              {FIG::ROOK,   50},
                                                              {FIG::QUEEN,  90}};

static int CountScore(const unsigned char *board, GosChess::color_t target_clr) {
    int score = 0;
    GosChess::figure_t fig;
    for (int i = 0; i < GosChess::board_t::BOARD_SIZE; i++) {
        if (board[i] == 0)continue;
        fig.full_type = board[i];
        if (fig.color_ == target_clr) {
            score += figure_values[fig.type_];
        }
    }
    return score;
}

// Every entry in the map already belongs to the side that was generated for,
// so they can simply be poured into one set.
static GosChess::actions_set_t MergeMoves(const GosChess::action_map_t &moves_by_square) {
    GosChess::actions_set_t moves;
    for (const auto &[square, from_square]: moves_by_square) {
        moves.insert(from_square.begin(), from_square.end());
    }
    return moves;
}

static int PromotionRankFor(GosChess::color_t color) {
    return color == GosChess::color_t::WHITE ? GosChess::board_t::ROW_NUM - 1 : 0;
}

static std::vector<GosChess::action_t> OrderMoves(const unsigned char *board,
                                                  const GosChess::actions_set_t &moves,
                                                  GosChess::color_t mover) {
    std::unordered_map<GosChess::action_t, int, GosChess::action_hash_t> move_scores;
    std::vector<GosChess::action_t> result_moves(moves.begin(), moves.end());
    for (GosChess::action_t move: moves) {
        GosChess::figure_t move_from(board[move.move_from]);
        GosChess::figure_t move_to(board[move.move_to]);
        if (move_to.full_type == 0) {
            move_scores[move] = GosChess::IsSquareAttacked(board, move.move_to)
                                ? -figure_values[move_from.type_] : 0;
        } else {
            move_scores[move] = figure_values[move_from.type_] - figure_values[move_to.type_];
        }
        if (move_from.type_ == FIG::PAWN &&
            move.move_to / GosChess::board_t::ROW_LENGTH == PromotionRankFor(mover)) {
            move_scores[move] += figure_values[FIG::QUEEN];
        }
    }
    std::sort(result_moves.begin(), result_moves.end(),
              [&](const GosChess::action_t &l, const GosChess::action_t &r) { return move_scores[l] > move_scores[r]; });
    return result_moves;
}

static int Evaluate(const unsigned char *board, GosChess::color_t ai_color) {
    return CountScore(board, ai_color) - CountScore(board, GosChess::OppositeColor(ai_color));
}

static int Search(GosChess::board_t &board, int depth, bool maximize, int alpha, int beta, bool initial,
                  GosChess::action_t &best_move, GosChess::color_t ai_color) {
    const GosChess::color_t color_to_calculate = maximize ? ai_color : GosChess::OppositeColor(ai_color);
    if (depth == 0)return Evaluate(board.getRawBoard(), ai_color);

    GosChess::action_map_t moves_by_square;
    GosChess::CalculateAvailableMoves(board.getRawBoard(), color_to_calculate, moves_by_square);

    // A terminal position has to be scored for whichever side is stuck in it,
    // not only for the engine's own side as it used to be.
    if (GosChess::HasNoLegalMoves(board, color_to_calculate, moves_by_square)) {
        if (!GosChess::IsKingInCheck(board.getRawBoard(), color_to_calculate)) return 0;
        return maximize ? INT_MIN + 1 : INT_MAX - 1;
    }

    GosChess::actions_set_t moves = MergeMoves(moves_by_square);
    int evaluated_best = maximize ? INT_MIN : INT_MAX;
    GosChess::action_t curr_best_move;
    for (auto &move: OrderMoves(board.getRawBoard(), moves, color_to_calculate)) {
        if (!GosChess::MakeMove(move, board, color_to_calculate, moves)) continue;
        int evaluate = Search(board, depth - 1, !maximize, alpha, beta, false, best_move, ai_color);
        GosChess::UndoMove(board);
        if (maximize) {
            if (evaluate > evaluated_best) {
                evaluated_best = evaluate;
                if (initial)curr_best_move = move;
            }
            alpha = std::max(alpha, evaluated_best);
        } else {
            evaluated_best = std::min(evaluated_best, evaluate);
            beta = std::min(beta, evaluated_best);
        }
        if (beta <= alpha) {
            break;
        }
    }

    if (initial) {
        best_move = curr_best_move;
    }
    return evaluated_best;
}

GosChess::action_t GosChess::GetBestMove(GosChess::board_t board, GosChess::color_t ai_color) {
    GosChess::action_t best_move;
    Search(board, ai_level, true, INT_MIN, INT_MAX, true, best_move, ai_color);
    return best_move;
}

int GosChess::ai_level = 2;
