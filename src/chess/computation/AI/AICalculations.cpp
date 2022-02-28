//
// Created by Leo Gogichaishvili on 14.02.22.
//

#include "AICalculations.h"
#include "../../global/GameGlobals.h"
#include "../InGameCalculations.h"

typedef GosChess::FigureTypes FIG;

static std::unordered_map<unsigned char, short> figure_values{{FIG::PAWN,   10},
                                                              {FIG::KNIGHT, 30},
                                                              {FIG::BISHOP, 30},
                                                              {FIG::ROOK,   50},
                                                              {FIG::QUEEN,  90}};

static int CountScore(const unsigned char *board, GosChess::Color target_clr) {
    int score = 0;
    GosChess::Figure fig;
    for (int i = 0; i < GosChess::Board::BOARD_SIZE; i++) {
        if (board[i] == 0)continue;
        fig.full_type = board[i];
        if (fig.color == target_clr) {
            score += figure_values[fig.type];
        }
    }
    return score;
}

static GosChess::MoveBucket MergeColorMoves(const unsigned char *board, GosChess::Color color) {
    GosChess::Figure fig;
    GosChess::MoveBucket moves;
    for (auto &figure_moves: GosChess::game_available_moves) {
        fig.full_type = board[figure_moves.first];
        if (fig.color == color) {
            moves.insert(figure_moves.second.begin(), figure_moves.second.end());
        }
    }
    return moves;
}

static std::vector<GosChess::Move> OrderMoves(const unsigned char *board, GosChess::MoveBucket &moves) {
    std::unordered_map<GosChess::Move, int, GosChess::MoveHash> move_scores;
    std::vector<GosChess::Move> result_moves(moves.begin(), moves.end());
    for (GosChess::Move move: moves) {
        GosChess::Figure move_from(board[move.move_from]);
        GosChess::Figure move_to(board[move.move_to]);
        if (move_to.full_type == 0) {
            if (GosChess::CheckIndexForAttackers(board, move.move_to)) {
                move_scores[move] = -figure_values[move_from.type];
                continue;
            }
        } else {
            int evaluate = figure_values[move_from.type] - figure_values[move_to.type];
            move_scores[move] = evaluate;
            continue;
        }
        move_scores[move] = INT_MIN;
    }
    std::sort(result_moves.begin(), result_moves.end(),
              [&](const GosChess::Move &l, const GosChess::Move &r) { return move_scores[l] > move_scores[r]; });
    return result_moves;
}

static int Evaluate(const unsigned char *board, const GosChess::Color &color) {
    int value = CountScore(board, GosChess::enemy_color) - CountScore(board, GosChess::player_color);
    return value;
}

static int
Search(GosChess::Board &board, int depth, bool maximize, int alpha, int beta, bool initial, GosChess::Move &best_move) {
    GosChess::Color color_to_calculate = maximize ? GosChess::enemy_color : GosChess::player_color;
    if (depth == 0)return Evaluate(board.GetRawBoard(), color_to_calculate);
    GosChess::CalculateAvailableMoves(board.GetRawBoard(), color_to_calculate);
    if (maximize && GosChess::CheckMate(board, GosChess::enemy_color))return INT_MIN;
    GosChess::MoveBucket moves = MergeColorMoves(board.GetRawBoard(), color_to_calculate);
    int evaluated_best = maximize ? INT_MIN : INT_MAX;
    GosChess::Move curr_best_move;
    for (auto &move: OrderMoves(board.GetRawBoard(), moves)) {
        if (MakeMove(move, board, color_to_calculate, moves)) {
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

GosChess::Move GosChess::GetBestMove(GosChess::Board board) {
    GosChess::Move best_move;
    sf::Clock clock;
    Search(board, ai_level, true, INT_MIN, INT_MAX, true, best_move);
    std::cout << clock.restart().asSeconds() << std::endl;
    GosChess::CalculateAvailableMoves(board.GetRawBoard(), GosChess::enemy_color);
    return best_move;
}

int GosChess::ai_level = 2;


