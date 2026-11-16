#include "GosChess/Board.hh"

#include <cctype>
#include <sstream>

namespace {

    // FNV-1a over the bytes that define a position. A 64 bit hash is used for
    // repetition detection rather than storing whole positions: the only cost
    // of a collision is a draw claimed one move early in a game that would
    // need two unrelated positions to hash alike, which at this width does not
    // happen in practice.
    constexpr std::uint64_t FNV_OFFSET = 1469598103934665603ull;
    constexpr std::uint64_t FNV_PRIME = 1099511628211ull;

    inline std::uint64_t Mix(std::uint64_t hash, unsigned char byte) {
        return (hash ^ byte) * FNV_PRIME;
    }

}

namespace GosChess {

    board_t::board_t() = default;

    board_t::board_t(const position_t &position) : position_(position) {}

    void board_t::setPosition(const position_t &position) {
        position_ = position;
        undo_stack_.clear();
    }

    void board_t::saveState() {
        undo_stack_.push_back(position_);
    }

    bool board_t::restoreState() {
        if (undo_stack_.empty()) return false;
        position_ = undo_stack_.back();
        undo_stack_.pop_back();
        return true;
    }

    void board_t::setFigure(const int &square, const unsigned char &figure) {
        position_.squares[square] = figure;
    }

    std::uint64_t board_t::key(color_t color_to_play) const {
        std::uint64_t hash = FNV_OFFSET;
        for (unsigned char square: position_.squares) hash = Mix(hash, square);
        hash = Mix(hash, position_.castling_rights);
        hash = Mix(hash, static_cast<unsigned char>(position_.en_passant_target + 1));
        hash = Mix(hash, static_cast<unsigned char>(color_to_play));
        return hash;
    }

    figure_t::figure_t(const unsigned char &clr, const unsigned char &tp) {
        full_type ^= full_type;
        color_ = clr;
        type_ = tp;
    }

    board_node_t GetSquare(const int &num) {
        return {num / board_t::ROW_LENGTH, num % board_t::ROW_LENGTH};
    }

    int GetNumFromNode(const board_node_t &cell) {
        return cell.y_ * board_t::ROW_LENGTH + cell.x_;
    }

    std::string SquareName(int square) {
        if (!IsSquare(square)) return "-";
        std::string name;
        name += static_cast<char>('a' + FileOf(square));
        name += static_cast<char>('1' + RankOf(square));
        return name;
    }

    int SquareFromName(const std::string &name) {
        if (name.size() < 2) return -1;
        const int file = name[0] - 'a';
        const int rank = name[1] - '1';
        if (file < 0 || file >= board_t::ROW_LENGTH || rank < 0 || rank >= board_t::ROW_NUM) return -1;
        return rank * board_t::ROW_LENGTH + file;
    }

    std::optional<fen_t> ParseFen(const std::string &fen) {
        std::istringstream stream(fen);
        std::string placement;
        std::string side;
        std::string rights;
        std::string en_passant;
        if (!(stream >> placement >> side)) return std::nullopt;

        fen_t parsed;

        // Rank 8 is written first, so it fills the highest indices.
        int rank = board_t::ROW_NUM - 1;
        int file = 0;
        for (const char symbol: placement) {
            if (symbol == '/') {
                if (file != board_t::ROW_LENGTH) return std::nullopt;
                rank--;
                file = 0;
                if (rank < 0) return std::nullopt;
                continue;
            }
            if (symbol >= '1' && symbol <= '8') {
                file += symbol - '0';
                if (file > board_t::ROW_LENGTH) return std::nullopt;
                continue;
            }
            const unsigned char type = FigureTypeFromFenChar(symbol);
            if (type == 0) return std::nullopt;
            if (file >= board_t::ROW_LENGTH) return std::nullopt;
            figure_t figure;
            figure.color_ = std::isupper(static_cast<unsigned char>(symbol)) ? color_t::WHITE
                                                                            : color_t::BLACK;
            figure.type_ = type;
            parsed.position.squares[rank * board_t::ROW_LENGTH + file] = figure.full_type;
            file++;
        }
        if (rank != 0 || file != board_t::ROW_LENGTH) return std::nullopt;

        if (side != "w" && side != "b") return std::nullopt;
        parsed.color_to_play = side == "w" ? color_t::WHITE : color_t::BLACK;

        // The remaining fields are optional: a position quoted in a book often
        // stops after the en passant square.
        if (stream >> rights) {
            for (const char symbol: rights) {
                switch (symbol) {
                    case 'K': parsed.position.castling_rights |= WHITE_KING_SIDE; break;
                    case 'Q': parsed.position.castling_rights |= WHITE_QUEEN_SIDE; break;
                    case 'k': parsed.position.castling_rights |= BLACK_KING_SIDE; break;
                    case 'q': parsed.position.castling_rights |= BLACK_QUEEN_SIDE; break;
                    case '-': break;
                    default: return std::nullopt;
                }
            }
        }
        if (stream >> en_passant && en_passant != "-") {
            const int square = SquareFromName(en_passant);
            if (square < 0) return std::nullopt;
            parsed.position.en_passant_target = static_cast<std::int8_t>(square);
        }
        int halfmove = 0;
        if (stream >> halfmove && halfmove >= 0) {
            parsed.position.halfmove_clock = static_cast<std::uint16_t>(halfmove);
        }
        int fullmove = 1;
        if (stream >> fullmove && fullmove >= 1) {
            parsed.fullmove_number = static_cast<std::uint16_t>(fullmove);
        }
        return parsed;
    }

    std::string MakeFen(const position_t &position, color_t color_to_play,
                        std::uint16_t fullmove_number) {
        std::string fen;
        for (int rank = board_t::ROW_NUM - 1; rank >= 0; rank--) {
            int empty = 0;
            for (int file = 0; file < board_t::ROW_LENGTH; file++) {
                const unsigned char raw = position.squares[rank * board_t::ROW_LENGTH + file];
                if (raw == 0) {
                    empty++;
                    continue;
                }
                if (empty != 0) {
                    fen += static_cast<char>('0' + empty);
                    empty = 0;
                }
                fen += FenCharFromFigure(raw);
            }
            if (empty != 0) fen += static_cast<char>('0' + empty);
            if (rank != 0) fen += '/';
        }

        fen += color_to_play == color_t::WHITE ? " w " : " b ";

        if (position.castling_rights == NO_CASTLING) {
            fen += '-';
        } else {
            if (position.castling_rights & WHITE_KING_SIDE) fen += 'K';
            if (position.castling_rights & WHITE_QUEEN_SIDE) fen += 'Q';
            if (position.castling_rights & BLACK_KING_SIDE) fen += 'k';
            if (position.castling_rights & BLACK_QUEEN_SIDE) fen += 'q';
        }

        fen += ' ';
        fen += IsSquare(position.en_passant_target) ? SquareName(position.en_passant_target) : "-";
        fen += ' ' + std::to_string(position.halfmove_clock);
        fen += ' ' + std::to_string(fullmove_number);
        return fen;
    }

}
