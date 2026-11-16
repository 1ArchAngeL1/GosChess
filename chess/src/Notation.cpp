#include "GosChess/Notation.hh"

#include "GosChess/Rules.hh"

#include <cctype>

namespace {

    using namespace GosChess;

    // Other pieces of the same kind and colour that could also legally move to
    // the same square. Notation only needs enough of the origin square to tell
    // them apart.
    struct ambiguity_t {
        bool any = false;
        bool shares_file = false;
        bool shares_rank = false;
    };

    ambiguity_t FindAmbiguity(board_t &board, color_t color, action_t move, unsigned char type) {
        ambiguity_t result;
        action_map_t legal;
        CalculateLegalMoves(board, color, legal);
        for (const auto &[square, moves]: legal) {
            if (square == move.move_from) continue;
            const figure_t other = board.getFigure(square);
            if (other.full_type == 0 || other.type_ != type || other.color_ != color) continue;
            for (const action_t &candidate: moves) {
                if (candidate.move_to != move.move_to) continue;
                result.any = true;
                if (FileOf(square) == FileOf(move.move_from)) result.shares_file = true;
                if (RankOf(square) == RankOf(move.move_from)) result.shares_rank = true;
            }
        }
        return result;
    }

    // "+" or "#", worked out by playing the move and asking the other side
    // whether it has anything left.
    std::string CheckSuffix(board_t &board, color_t color, action_t move) {
        if (!TryMove(move, board, color)) return {};
        const color_t opponent = OppositeColor(color);
        const bool in_check = IsKingInCheck(board, opponent);
        const bool stuck = HasNoLegalMoves(board, opponent, {});
        UndoMove(board);
        if (!in_check) return {};
        return stuck ? "#" : "+";
    }

}

std::string GosChess::ToSan(GosChess::board_t &board, GosChess::color_t color,
                            GosChess::action_t move) {
    if (!IsSquare(move.move_from) || !IsSquare(move.move_to)) return ToCoordinateNotation(move);
    const figure_t mover = board.getFigure(move.move_from);
    if (mover.full_type == 0) return ToCoordinateNotation(move);

    const castling_rook_move_t castle = CastlingRookMove(board, move);
    if (castle.from >= 0) {
        const bool king_side = FileOf(move.move_to) > FileOf(move.move_from);
        return (king_side ? "O-O" : "O-O-O") + CheckSuffix(board, color, move);
    }

    const bool en_passant = IsEnPassantCapture(board, move);
    const bool capture = board.getRawBoard()[move.move_to] != 0 || en_passant;

    std::string text;
    if (mover.type_ == figure_types_t::PAWN) {
        // A pawn is named by the file it came from, but only when it captures.
        if (capture) {
            text += static_cast<char>('a' + FileOf(move.move_from));
            text += 'x';
        }
        text += SquareName(move.move_to);
        if (move.isPromotion()) {
            text += '=';
            text += NotationLetter(move.promotion);
        }
    } else {
        text += NotationLetter(mover.type_);
        const ambiguity_t ambiguity = FindAmbiguity(board, color, move, mover.type_);
        if (ambiguity.any) {
            // The file alone usually does it; the rank is needed when the two
            // pieces stand on the same file, and both when they share neither.
            if (!ambiguity.shares_file) {
                text += static_cast<char>('a' + FileOf(move.move_from));
            } else if (!ambiguity.shares_rank) {
                text += static_cast<char>('1' + RankOf(move.move_from));
            } else {
                text += SquareName(move.move_from);
            }
        }
        if (capture) text += 'x';
        text += SquareName(move.move_to);
    }

    return text + CheckSuffix(board, color, move);
}

std::string GosChess::ToCoordinateNotation(GosChess::action_t move) {
    std::string text = SquareName(move.move_from) + SquareName(move.move_to);
    if (move.isPromotion()) {
        const char letter = NotationLetter(move.promotion);
        if (letter != '\0') text += static_cast<char>(std::tolower(letter));
    }
    return text;
}
