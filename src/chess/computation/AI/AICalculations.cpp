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


int GosChess::CountScore(const unsigned char *board, GosChess::Color target_clr) {
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

std::vector<GosChess::Move> GosChess::MergeColorMoves(GosChess::Color) {
    std::vector<GosChess::Move> moves;
    for (auto &i: GosChess::available_moves) {
        for (auto j: GosChess::available_moves[i.first]) {
            moves.push_back(j);
        }
    }
    return moves;
}

int GosChess::Evaluate(const unsigned char *board) {
    int eval = CountScore(board, GosChess::WHITE) - CountScore(board, GosChess::BLACK);
    return GosChess::color_to_play == GosChess::WHITE ? eval : -eval;
}


GosChess::Move GosChess::BasicChessAi::GetBestMove(GosChess::Board board) {
    GosChess::Move best_move;
    int best_move_value = INT_MIN;
    //int current_value = GosChess::CountScore(board.GetRawBoard(),GosChess::enemy_color);
    std::vector<GosChess::Move> moves = GosChess::MergeColorMoves(GosChess::enemy_color);
    int calculated_value;
    for (auto &move: moves) {
        if (GosChess::MakeMove(move, board)) {
            calculated_value = GosChess::Search(board, 3, GosChess::enemy_color);
            if (calculated_value < best_move_value) {
                best_move_value = calculated_value;
                best_move = move;
            }
            board.Undo();
        }

    }
    return best_move;
}


int GosChess::Search(GosChess::Board &board, int depth, GosChess::Color clr) {
    if (depth == 0) return GosChess::Evaluate(board.GetRawBoard());
    if (clr == GosChess::enemy_color) {
        if (GosChess::CheckMate(board, GosChess::color_to_play)) return INT_MIN;
    }
    if (GosChess::available_moves.size() == 0) return Evaluate(board.GetRawBoard());

    GosChess::CalculateAvailableMoves(board.GetRawBoard(), clr);
    std::vector<GosChess::Move> moves = GosChess::MergeColorMoves(clr);
    int best = INT_MIN;
    for (auto &move: moves) {
        GosChess::MakeMove(move, board);
        int curr_eval = -Search(board, depth - 1, static_cast<GosChess::Color>(!clr));
        board.Undo();
        best = std::max(best, curr_eval);

    }
    return best;

}

