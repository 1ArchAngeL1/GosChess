//
// Created by Leo Gogichaishvili on 14.02.22.
//

#include "AICalculations.h"
#include "../../global/GameGlobals.h"
#include "../InGameCalculations.h"
#include "../../../exec/GamePlay/GamePlayFunctional.h"

typedef GosChess::FigureTypes FIG;

static std::unordered_map<unsigned char, short> figure_values{{FIG::PAWN,   10},
                                                              {FIG::KNIGHT, 30},
                                                              {FIG::BISHOP, 30},
                                                              {FIG::ROOK,   50},
                                                              {FIG::QUEEN,  90}};


static int CountScore(const unsigned char *board, GosChess::Color target_clr) {
    int score = 0;
    for (int i = 0; i < GosChess::Board::BOARD_SIZE; i++) {
        if (board[i] == 0)continue;
        GosChess::Figure fig(board[i]);
        if (fig.color == target_clr) {
            score += figure_values[fig.type];
        }
    }
    return score;
}

static std::vector<GosChess::Move> MergeColorMoves(GosChess::Color) {
    std::vector<GosChess::Move> moves;
    for (auto &figure_moves: GosChess::available_moves) {
        for (auto move: GosChess::available_moves[figure_moves.first]) {
            moves.push_back(move);
        }
    }
    return moves;
}

static int Evaluate(const unsigned char *board) {
    return CountScore(board, GosChess::enemy_color) - CountScore(board, GosChess::player_color);
}

static int Search(GosChess::Board &board, int depth, bool Maximize, int alpha, int beta) {
    GosChess::Color color_to_calculate = Maximize ? GosChess::player_color : GosChess::enemy_color;
    CalculateAvailableMoves(board.GetRawBoard(), color_to_calculate);
    if (depth == 0)return Evaluate(board.GetRawBoard());
    if (GosChess::CheckMate(board, color_to_calculate))return INT_MIN;
    std::vector<GosChess::Move> moves = MergeColorMoves(color_to_calculate);
    if (Maximize) {
        int maximum = INT_MIN;
        for (auto &move: moves) {
            if (MakeMove(move, board)) {
                int curr_score = Search(board, depth - 1, false, alpha, beta);
                maximum = std::max(curr_score, maximum);
                alpha = std::max(alpha, maximum);
                board.Undo();
                if (beta <= alpha)break;
            }
        }
        return maximum;
    } else {
        int minimum = INT_MAX;
        for (auto &move: moves) {
            if (MakeMove(move, board)) {
                int curr_score = Search(board, depth - 1, true, alpha, beta);
                minimum = std::min(curr_score, minimum);
                beta = std::min(beta, minimum);
                board.Undo();
                if (beta <= alpha)break;
            }
        }
        return minimum;
    }

}


GosChess::Move GosChess::GetBestMove(GosChess::Board board) {
    GosChess::Move best_move;
    int best_move_value = INT_MIN;
    //int current_value = GosChess::CountScore(board.GetRawBoard(),GosChess::enemy_color);
    std::vector<GosChess::Move> moves = MergeColorMoves(GosChess::enemy_color);
    int calculated_value;
    for (auto &move: moves) {
        if (GosChess::MakeMove(move, board)) {
            calculated_value = Evaluate(board.GetRawBoard()) + Search(board, 4, false, INT_MAX, INT_MIN);
            if (calculated_value > best_move_value) {
                best_move_value = calculated_value;
                best_move = move;
            }
            board.Undo();
            GosChess::CalculateAvailableMoves(board.GetRawBoard(), GosChess::enemy_color);
        }
    }
    return best_move;
}


