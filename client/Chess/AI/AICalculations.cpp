//
// Created by Leo Gogichaishvili on 14.02.22.
//

#include "AICalculations.hh"

#include <algorithm>
#include <array>
#include <climits>
#include <vector>

#include "GosChess/Rules.hh"

namespace {

    using namespace GosChess;

    constexpr int PIECE_VALUE[8] = {
            0,
            500,   // ROOK
            330,   // BISHOP
            100,   // PAWN
            320,   // KNIGHT
            0,     // KING - priceless, and never actually captured
            900,   // QUEEN
            0
    };

    // Scores near the extremes are reserved for mates, so that a mate found
    // sooner beats a mate found later and the engine actually finishes games
    // instead of shuffling in a won position.
    constexpr int MATE_SCORE = 100000;

    // Square preferences, written from White's point of view and mirrored for
    // Black. Material alone makes an engine that develops nothing and leaves
    // its knights in the corner; this is the cheapest fix that changes how the
    // moves actually look.
    constexpr std::array<int, 64> PAWN_SQUARES = {
            0,  0,  0,  0,  0,  0,  0,  0,
            5, 10, 10,-20,-20, 10, 10,  5,
            5, -5,-10,  0,  0,-10, -5,  5,
            0,  0,  0, 20, 20,  0,  0,  0,
            5,  5, 10, 25, 25, 10,  5,  5,
            10, 10, 20, 30, 30, 20, 10, 10,
            50, 50, 50, 50, 50, 50, 50, 50,
            0,  0,  0,  0,  0,  0,  0,  0
    };

    constexpr std::array<int, 64> KNIGHT_SQUARES = {
            -50,-40,-30,-30,-30,-30,-40,-50,
            -40,-20,  0,  5,  5,  0,-20,-40,
            -30,  5, 10, 15, 15, 10,  5,-30,
            -30,  0, 15, 20, 20, 15,  0,-30,
            -30,  5, 15, 20, 20, 15,  5,-30,
            -30,  0, 10, 15, 15, 10,  0,-30,
            -40,-20,  0,  0,  0,  0,-20,-40,
            -50,-40,-30,-30,-30,-30,-40,-50
    };

    constexpr std::array<int, 64> BISHOP_SQUARES = {
            -20,-10,-10,-10,-10,-10,-10,-20,
            -10,  5,  0,  0,  0,  0,  5,-10,
            -10, 10, 10, 10, 10, 10, 10,-10,
            -10,  0, 10, 10, 10, 10,  0,-10,
            -10,  5,  5, 10, 10,  5,  5,-10,
            -10,  0,  5, 10, 10,  5,  0,-10,
            -10,  0,  0,  0,  0,  0,  0,-10,
            -20,-10,-10,-10,-10,-10,-10,-20
    };

    constexpr std::array<int, 64> ROOK_SQUARES = {
            0,  0,  0,  5,  5,  0,  0,  0,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            -5,  0,  0,  0,  0,  0,  0, -5,
            5, 10, 10, 10, 10, 10, 10,  5,
            0,  0,  0,  0,  0,  0,  0,  0
    };

    constexpr std::array<int, 64> KING_SQUARES = {
            20, 30, 10,  0,  0, 10, 30, 20,
            20, 20,  0,  0,  0,  0, 20, 20,
            -10,-20,-20,-20,-20,-20,-20,-10,
            -20,-30,-30,-40,-40,-30,-30,-20,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30,
            -30,-40,-40,-50,-50,-40,-40,-30
    };

    // White reads the table as written; Black reads it upside down.
    int SquareBonus(const std::array<int, 64> &table, int square, color_t color) {
        return color == color_t::WHITE ? table[square] : table[square ^ 56];
    }

    int PositionalBonus(unsigned char type, int square, color_t color) {
        switch (type) {
            case figure_types_t::PAWN:   return SquareBonus(PAWN_SQUARES, square, color);
            case figure_types_t::KNIGHT: return SquareBonus(KNIGHT_SQUARES, square, color);
            case figure_types_t::BISHOP: return SquareBonus(BISHOP_SQUARES, square, color);
            case figure_types_t::ROOK:   return SquareBonus(ROOK_SQUARES, square, color);
            case figure_types_t::KING:   return SquareBonus(KING_SQUARES, square, color);
            default: return 0;
        }
    }

    // Positive means good for `ai_color`.
    int Evaluate(const board_t &board, color_t ai_color) {
        int score = 0;
        const unsigned char *squares = board.getRawBoard();
        for (int square = 0; square < board_t::BOARD_SIZE; square++) {
            if (squares[square] == 0) continue;
            const figure_t piece(squares[square]);
            const color_t color = static_cast<color_t>(piece.color_);
            const int value = PIECE_VALUE[piece.type_] +
                              PositionalBonus(piece.type_, square, color);
            score += color == ai_color ? value : -value;
        }
        return score;
    }

    // Captures first, best capture first, then promotions. Cheap ordering is
    // what makes alpha-beta actually prune.
    void OrderMoves(const board_t &board, std::vector<action_t> &moves) {
        const unsigned char *squares = board.getRawBoard();
        std::stable_sort(moves.begin(), moves.end(),
                         [&](const action_t &left, const action_t &right) {
                             auto score = [&](const action_t &move) {
                                 int value = 0;
                                 const unsigned char target = squares[move.move_to];
                                 if (target != 0) {
                                     const figure_t victim(target);
                                     const figure_t attacker(squares[move.move_from]);
                                     // Take the most valuable piece with the
                                     // least valuable one.
                                     value += 10 * PIECE_VALUE[victim.type_] -
                                              PIECE_VALUE[attacker.type_];
                                 }
                                 if (move.isPromotion()) value += PIECE_VALUE[move.promotion];
                                 return value;
                             };
                             return score(left) > score(right);
                         });
    }

    int Search(board_t &board, color_t to_move, color_t ai_color, int depth, int ply,
               int alpha, int beta, action_t *best_move) {
        std::vector<action_t> moves;
        GenerateMoveList(board, to_move, moves);
        OrderMoves(board, moves);

        const bool maximizing = to_move == ai_color;
        int best = maximizing ? INT_MIN : INT_MAX;
        action_t best_found;
        bool any_legal = false;

        for (const action_t &move: moves) {
            if (!TryMove(move, board, to_move)) continue;
            any_legal = true;
            const int score = depth <= 1
                              ? Evaluate(board, ai_color)
                              : Search(board, OppositeColor(to_move), ai_color, depth - 1, ply + 1,
                                       alpha, beta, nullptr);
            UndoMove(board);

            if (maximizing) {
                if (score > best) {
                    best = score;
                    best_found = move;
                }
                alpha = std::max(alpha, best);
            } else {
                if (score < best) {
                    best = score;
                    best_found = move;
                }
                beta = std::min(beta, best);
            }
            if (beta <= alpha) break;
        }

        if (!any_legal) {
            // Mate or stalemate. Scoring mate by distance means the engine
            // walks towards the shortest one instead of circling forever.
            if (!IsKingInCheck(board, to_move)) return 0;
            return maximizing ? -(MATE_SCORE - ply) : (MATE_SCORE - ply);
        }

        if (best_move != nullptr) *best_move = best_found;
        return best;
    }

}

GosChess::action_t GosChess::GetBestMove(GosChess::board_t board, GosChess::color_t ai_color) {
    const int depth = std::clamp(ai_level, 1, MAX_AI_LEVEL);
    action_t best_move;
    Search(board, ai_color, ai_color, depth, 0, INT_MIN + 1, INT_MAX - 1, &best_move);
    return best_move;
}

int GosChess::ai_level = 3;
