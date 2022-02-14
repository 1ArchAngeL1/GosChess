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


GosChess::Move GosChess::BasicChessAi::GetBestMove(unsigned char *) {
    return GosChess::Move();
}


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

int GosChess::MiniMax(GosChess::Board &board, int depth, bool Maximize) {
    GosChess::Color clr = Maximize ? GosChess::color_to_play : static_cast<GosChess::Color>(!GosChess::color_to_play);
    if (depth <= 0 || GosChess::CheckMate(board, clr)) {
        return GosChess::Evaluate(board.GetRawBoard());
    }
    GosChess::CalculateAvailableMoves(board.GetRawBoard());
    std::vector<GosChess::Move> moves = GosChess::MergeColorMoves(color_to_play);
    if (clr == GosChess::color_to_play) {
        int max = INT_MIN;
        for (auto & move : moves) {
            if(!GosChess::MakeMove(move, board))continue;
            int curr_eval = MiniMax(board, depth - 1, !Maximize);
            max = std::max(max, curr_eval);
            board.Undo();
        }
        return max;
    } else {
        int min = INT_MAX;
        for (auto & move : moves) {
            GosChess::MakeMove(move, board);
            int curr_eval = MiniMax(board, depth - 1, !Maximize);
            min = std::min(min, curr_eval);
            board.Undo();
        }
        return min;
    }
}
