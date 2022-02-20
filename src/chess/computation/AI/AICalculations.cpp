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

static std::vector<GosChess::Move> MergeColorMoves(const unsigned char *board, GosChess::Color clr) {
    GosChess::Figure fig;
    std::vector<GosChess::Move> moves;
    for (auto &figure_moves: GosChess::available_moves) {
        for (auto move: figure_moves.second) {
            if (board[move.move_from] != 0) {
                fig.full_type = board[move.move_from];
                if (fig.color == clr) moves.push_back(move);
            }
        }
    }
    return moves;
}

static int Evaluate(const unsigned char *board) {
    return CountScore(board, GosChess::enemy_color) - CountScore(board, GosChess::player_color);
}


static void OrderMoves(const unsigned char *board, std::vector<GosChess::Move> &moves) {
    std::unordered_map<GosChess::Move, int, GosChess::MoveHash> move_scores;
    for (GosChess::Move &move: moves) {
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
    std::sort(moves.begin(), moves.end(),
              [&](const GosChess::Move &l, const GosChess::Move &r) { return move_scores[l] < move_scores[r]; });
}


static int Search(GosChess::Board &board, int depth, bool maximize, int alpha, int beta, GosChess::Move &result_move) {
    GosChess::Color color_to_calculate = maximize ? GosChess::enemy_color : GosChess::player_color;
    CalculateAvailableMoves(board.GetRawBoard(), color_to_calculate);
    if (depth == 0)return Evaluate(board.GetRawBoard());
    //if (maximize && GosChess::CheckMate(board, GosChess::enemy_color))return INT_MIN;
    std::vector<GosChess::Move> moves = MergeColorMoves(board.GetRawBoard(), color_to_calculate);
    if (maximize) {
        int maximum = INT_MIN;
        GosChess::Move maximum_move;
        for (auto &move: moves) {
            if (MakeMove(move, board)) {
                int curr_score = Search(board, depth - 1, false, alpha, beta, result_move);
                if (curr_score >= maximum) {
                    maximum = curr_score;
                    maximum_move = move;
                }
                alpha = std::max(alpha, maximum);
                GosChess::UndoMove(board);
                if (beta <= alpha)break;
            }
        }
        result_move = maximum_move;
        return maximum;
    } else {
        int minimum = INT_MAX;
        for (auto &move: moves) {
            if (MakeMove(move, board)) {
                int curr_score = Search(board, depth - 1, true, alpha, beta, result_move);
                minimum = std::min(minimum, curr_score);
                beta = std::min(beta, minimum);
                GosChess::UndoMove(board);
                if (beta <= alpha)break;
            }
        }
        return minimum;
    }

}

GosChess::Move GosChess::GetBestMove(GosChess::Board board) {
    GosChess::Move best_move;
    //int current_value = GosChess::CountScore(board.GetRawBoard(),GosChess::enemy_color);
    Search(board, 2, true, INT_MIN, INT_MAX, best_move);
    GosChess::CalculateAvailableMoves(board.GetRawBoard(), GosChess::enemy_color);
    return best_move;
}


