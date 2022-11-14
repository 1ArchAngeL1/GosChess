//
// Created by Leo Gogichaishvili on 14.02.22.
//

#include "AICalculations.hh"
#include "../../global/GameGlobals.hh"
#include "../InGameCalculations.hh"

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
        if (fig.color == target_clr) {
            score += figure_values[fig.type];
        }
    }
    return score;
}

static GosChess::actions_set_t MergeColorMoves(const unsigned char *board, GosChess::color_t color) {
    GosChess::figure_t fig;
    GosChess::actions_set_t moves;
    for (auto &figure_moves: GosChess::game_available_moves) {
        fig.full_type = board[figure_moves.first];
        if (fig.color == color) {
            moves.insert(figure_moves.second.begin(), figure_moves.second.end());
        }
    }
    return moves;
}

static std::vector<GosChess::action_t> OrderMoves(const unsigned char *board, GosChess::actions_set_t &moves) {
    std::unordered_map<GosChess::action_t, int, GosChess::action_hash_t> move_scores;
    std::vector<GosChess::action_t> result_moves(moves.begin(), moves.end());
    for (GosChess::action_t move: moves) {
        GosChess::figure_t move_from(board[move.move_from]);
        GosChess::figure_t move_to(board[move.move_to]);
        move_scores[move] = INT_MIN;
        if (move_to.full_type == 0) {
            if (GosChess::CheckIndexForAttackers(board, move.move_to))
                move_scores[move] = -figure_values[move_from.type];
            else move_scores[move] = 0;
        } else {
            int evaluate = figure_values[move_from.type] - figure_values[move_to.type];
            move_scores[move] = evaluate;
        }
        if (move.move_to / GosChess::board_t::ROW_LENGTH == GosChess::board_t::ROW_NUM - 1) {
            if (move_from.type == GosChess::figure_types_t::PAWN && move_from.color == GosChess::player_color) {
                move_scores[move] += figure_values[FIG::QUEEN];
            }
        } else if (move.move_to < GosChess::board_t::ROW_LENGTH) {
            if (move_from.type == GosChess::figure_types_t::PAWN && move_from.color == GosChess::enemy_color) {
                move_scores[move] += figure_values[FIG::QUEEN];
            }
        }
    }
    std::sort(result_moves.begin(), result_moves.end(),
              [&](const GosChess::action_t &l, const GosChess::action_t &r) { return move_scores[l] > move_scores[r]; });
    return result_moves;
}

static int Evaluate(const unsigned char *board, const GosChess::color_t &color) {
    int value = CountScore(board, GosChess::enemy_color) - CountScore(board, GosChess::player_color);
    return value;
}

static int
Search(GosChess::board_t &board, int depth, bool maximize, int alpha, int beta, bool initial, GosChess::action_t &best_move) {
    GosChess::color_t color_to_calculate = maximize ? GosChess::enemy_color : GosChess::player_color;
    if (depth == 0)return Evaluate(board.getRawBoard(), color_to_calculate);
    GosChess::CalculateAvailableMoves(board.getRawBoard(), color_to_calculate);
    if (maximize &&
        (GosChess::CheckForDraw(board, GosChess::enemy_color) || GosChess::CheckMate(board, GosChess::enemy_color))) {
        return INT_MIN + 1;
    }
    GosChess::actions_set_t moves = MergeColorMoves(board.getRawBoard(), color_to_calculate);
    int evaluated_best = maximize ? INT_MIN : INT_MAX;
    GosChess::action_t curr_best_move;
    int count = 0;
    for (auto &move: OrderMoves(board.getRawBoard(), moves)) {
        if (MakeMove(move, board, color_to_calculate, moves)) {
            count++;
            int evaluate = Search(board, depth - 1, !maximize, alpha, beta, false, best_move);
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
    }

    if (initial) {
        best_move = curr_best_move;
    }
    return evaluated_best;
}

GosChess::action_t GosChess::GetBestMove(GosChess::board_t board) {
    GosChess::action_t best_move;
    Search(board, ai_level, true, INT_MIN, INT_MAX, true, best_move);
    GosChess::CalculateAvailableMoves(board.getRawBoard(), GosChess::enemy_color);
    return best_move;
}

int GosChess::ai_level = 2;


