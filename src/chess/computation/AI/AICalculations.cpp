//
// Created by Leo Gogichaishvili on 14.02.22.
//

#include "AICalculations.h"
#include "../../global/GameGlobals.h"
#include "../InGameCalculations.h"

typedef GosChess::FigureTypes FIG;

static std::unordered_map<unsigned char, short> figure_values{{FIG::PAWN,   100},
                                                              {FIG::KNIGHT, 300},
                                                              {FIG::BISHOP, 300},
                                                              {FIG::ROOK,   500},
                                                              {FIG::QUEEN,  900}};


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

static int Search(GosChess::Board &board, int depth, bool Maximize) {
    GosChess::Color color_to_calculate = Maximize ? GosChess::player_color : GosChess::enemy_color;
    if (depth == 0)return Evaluate(board.GetRawBoard());
    if (GosChess::CheckMate(board, color_to_calculate))return INT_MIN;
    int maximum = Maximize ? INT_MIN : INT_MAX;
    CalculateAvailableMoves(board.GetRawBoard(), color_to_calculate);
    std::vector<GosChess::Move> moves = MergeColorMoves(color_to_calculate);
    for (auto &move: moves) {
        MakeMove(move, board);
        int curr_score = Search(board, depth - 1, !Maximize);
        if (!Maximize)curr_score *= -1;
        if (curr_score > maximum) maximum = curr_score;
        board.Undo();
    }
    return maximum;

}


GosChess::Move GosChess::GetBestMove(GosChess::Board board) {
    GosChess::Move best_move;
    int best_move_value = INT_MIN;
    //int current_value = GosChess::CountScore(board.GetRawBoard(),GosChess::enemy_color);
    std::vector<GosChess::Move> moves = MergeColorMoves(GosChess::enemy_color);
    int calculated_value;
    for (auto &move: moves) {
        if (GosChess::MakeMove(move, board)) {
            calculated_value = Search(board, 10, true);
            if (calculated_value > best_move_value) {
                best_move_value = calculated_value;
                best_move = move;
            }
            board.Undo();
        }

    }
    return best_move;
}


