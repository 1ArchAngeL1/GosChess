#ifndef GOSCHESS_BOARD_HH
#define GOSCHESS_BOARD_HH

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Board representation and FEN.
//
// Squares are numbered 0..63 in *absolute* orientation: index = rank * 8 + file,
// with rank 0 being White's back rank and file 0 the a-file. Index 0 is a1,
// index 63 is h8. The orientation never depends on who is looking at the board
// - mirroring for the player sitting on the black side is a rendering concern
// and lives in the client's view layer.
//
// A chess position is more than where the pieces stand: castling rights, the
// en passant square and the halfmove clock are all part of it, and two
// otherwise identical boards with different rights are different positions.
// position_t holds the lot, which is why it is what gets snapshotted, hashed
// for repetition, and written to FEN.

namespace GosChess {

    enum color_t {
        WHITE, BLACK
    };

    enum figure_types_t {
        ROOK = 1, BISHOP,
        PAWN, KNIGHT,
        KING, QUEEN
    };

    constexpr color_t OppositeColor(color_t color) {
        return color == color_t::WHITE ? color_t::BLACK : color_t::WHITE;
    }

    // Castling rights, as a bitmask.
    enum castling_right_t : std::uint8_t {
        NO_CASTLING = 0,
        WHITE_KING_SIDE = 1u << 0u,
        WHITE_QUEEN_SIDE = 1u << 1u,
        BLACK_KING_SIDE = 1u << 2u,
        BLACK_QUEEN_SIDE = 1u << 3u,
        ALL_CASTLING = 0x0Fu
    };

    constexpr std::uint8_t KingSideRight(color_t color) {
        return color == color_t::WHITE ? WHITE_KING_SIDE : BLACK_KING_SIDE;
    }

    constexpr std::uint8_t QueenSideRight(color_t color) {
        return color == color_t::WHITE ? WHITE_QUEEN_SIDE : BLACK_QUEEN_SIDE;
    }

    struct board_node_t {
        int y_;
        int x_;

        board_node_t() = default;

        board_node_t(const int &y, const int &x) : y_(y), x_(x) {}

        board_node_t operator+(const board_node_t &node) const {
            return {this->y_ + node.y_, this->x_ + node.x_};
        }
    };


    union figure_t {
        explicit figure_t(const unsigned char &, const unsigned char &);

        explicit figure_t(const unsigned char &full_type) : full_type(full_type) {}

        explicit figure_t() : full_type(0) {}

        struct {
            unsigned char color_: 1;
            unsigned char type_: 3;
        };
        unsigned char full_type;
    };


    class board_t {
    public:
        static constexpr int ROW_LENGTH = 8;

        static constexpr int ROW_NUM = 8;

        static constexpr int BOARD_SIZE = ROW_LENGTH * ROW_NUM;

        // Everything that makes one position different from another.
        struct position_t {
            std::array<unsigned char, BOARD_SIZE> squares{};
            std::uint8_t castling_rights = NO_CASTLING;
            // The square a pawn would land on when capturing en passant, or -1
            // when the last move did not create the opportunity.
            std::int8_t en_passant_target = -1;
            // Plies since the last capture or pawn move, for the fifty move rule.
            std::uint16_t halfmove_clock = 0;
        };

        board_t();

        explicit board_t(const position_t &position);

        void setPosition(const position_t &position);

        [[nodiscard]] const position_t &position() const { return position_; }

        // Pushes a snapshot of the current position onto the undo stack.
        void saveState();

        // Pops the most recent snapshot back. Returns false when the undo stack
        // is empty, leaving the position untouched.
        bool restoreState();

        void setFigure(const int &square, const unsigned char &figure);

        [[nodiscard]] const unsigned char *getRawBoard() const { return position_.squares.data(); }

        [[nodiscard]] figure_t getFigure(const int &square) const {
            return figure_t(position_.squares[square]);
        }

        [[nodiscard]] std::uint8_t castlingRights() const { return position_.castling_rights; }

        [[nodiscard]] std::int8_t enPassantTarget() const { return position_.en_passant_target; }

        [[nodiscard]] std::uint16_t halfmoveClock() const { return position_.halfmove_clock; }

        void setCastlingRights(std::uint8_t rights) { position_.castling_rights = rights; }

        void clearCastlingRights(std::uint8_t rights) {
            position_.castling_rights &= static_cast<std::uint8_t>(~rights);
        }

        void setEnPassantTarget(std::int8_t square) { position_.en_passant_target = square; }

        void setHalfmoveClock(std::uint16_t plies) { position_.halfmove_clock = plies; }

        // Identifies the position for repetition purposes: the pieces, the
        // rights, the en passant square and whose turn it is. Deliberately not
        // the clocks, since a repetition does not care how long it took.
        [[nodiscard]] std::uint64_t key(color_t color_to_play) const;

    private:
        position_t position_;
        std::vector<position_t> undo_stack_;
    };

    typedef board_t::position_t position_t;

    // --- FEN ---------------------------------------------------------------
    //
    // Standard FEN, rank 8 first, with all six fields. The project used to
    // carry a homemade variant that listed ranks the other way round and had
    // nowhere to put castling rights or an en passant square - which stopped
    // mattering the moment those became part of a position, and also made it
    // impossible to check the move generator against published test positions.

    struct fen_t {
        position_t position;
        color_t color_to_play = color_t::WHITE;
        std::uint16_t fullmove_number = 1;
    };

    inline constexpr const char *INITIAL_FEN =
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Returns nothing when the text is not a position this can read, rather
    // than throwing: a FEN can arrive over a socket.
    std::optional<fen_t> ParseFen(const std::string &fen);

    std::string MakeFen(const position_t &position, color_t color_to_play,
                        std::uint16_t fullmove_number);

    // index <-> (rank, file) conversions. Pure arithmetic on absolute indices.
    board_node_t GetSquare(const int &square);

    int GetNumFromNode(const board_node_t &cell);

    constexpr bool OnBoard(const board_node_t &node) {
        return node.x_ >= 0 && node.x_ < board_t::ROW_LENGTH &&
               node.y_ >= 0 && node.y_ < board_t::ROW_NUM;
    }

    constexpr bool IsSquare(int square) {
        return square >= 0 && square < board_t::BOARD_SIZE;
    }

    constexpr int RankOf(int square) { return square / board_t::ROW_LENGTH; }

    constexpr int FileOf(int square) { return square % board_t::ROW_LENGTH; }

    // "e4" style names, for notation and for anything a human has to read.
    std::string SquareName(int square);

    // Returns -1 when the text is not a square name.
    int SquareFromName(const std::string &name);

    // --- piece letters -----------------------------------------------------
    //
    // Functions over a switch rather than the std::maps these replaced,
    // because a map is filled during dynamic initialisation: a GameState that
    // is itself a global - the client has one - would decode its opening
    // position before the maps existed, and the order of the two is not
    // something a translation unit gets to decide.

    constexpr unsigned char FigureTypeFromFenChar(char symbol) {
        switch (symbol) {
            case 'p': case 'P': return figure_types_t::PAWN;
            case 'q': case 'Q': return figure_types_t::QUEEN;
            case 'k': case 'K': return figure_types_t::KING;
            case 'b': case 'B': return figure_types_t::BISHOP;
            case 'r': case 'R': return figure_types_t::ROOK;
            case 'n': case 'N': return figure_types_t::KNIGHT;
            default: return 0;
        }
    }

    // The FEN letter for a raw figure byte - upper case for White, lower for
    // Black - or '\0' if the byte holds no piece.
    inline char FenCharFromFigure(unsigned char full_type) {
        const figure_t figure(full_type);
        char symbol;
        switch (figure.type_) {
            case figure_types_t::PAWN:   symbol = 'P'; break;
            case figure_types_t::QUEEN:  symbol = 'Q'; break;
            case figure_types_t::KING:   symbol = 'K'; break;
            case figure_types_t::BISHOP: symbol = 'B'; break;
            case figure_types_t::ROOK:   symbol = 'R'; break;
            case figure_types_t::KNIGHT: symbol = 'N'; break;
            default: return '\0';
        }
        return figure.color_ == color_t::BLACK ? static_cast<char>(symbol - 'A' + 'a') : symbol;
    }

    // The upper case letter used for a piece in algebraic notation. Pawns have
    // no letter, so this returns '\0' for them.
    constexpr char NotationLetter(unsigned char type) {
        switch (type) {
            case figure_types_t::QUEEN:  return 'Q';
            case figure_types_t::ROOK:   return 'R';
            case figure_types_t::BISHOP: return 'B';
            case figure_types_t::KNIGHT: return 'N';
            case figure_types_t::KING:   return 'K';
            default: return '\0';
        }
    }

}

#endif
