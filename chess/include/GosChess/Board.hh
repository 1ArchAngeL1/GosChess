#ifndef GOSCHESS_BOARD_HH
#define GOSCHESS_BOARD_HH

#include <array>
#include <map>
#include <stack>
#include <string>

// Board representation and the FEN-like serialisation used on the wire.
//
// Squares are numbered 0..63 in *absolute* orientation: index = rank * 8 + file,
// with rank 0 being White's back rank and file 0 the a-file. Index 0 is a1,
// index 63 is h8. The orientation never depends on who is looking at the board
// - mirroring for the player sitting on the black side is a rendering concern
// and lives in the client's view layer.

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
        /* object construction functions */
        explicit board_t(const std::string &);

        board_t();

        board_t(const board_t &board);

        board_t(board_t &&board) noexcept;

        board_t &operator=(const board_t &board);

        board_t &operator=(board_t &&board) noexcept;

        ~board_t();

        /* member functions */
        void setState(const std::string &state);

        // Pushes a snapshot of the current position onto the undo stack.
        void saveState();

        // Pops the most recent snapshot back onto the board. Returns false when
        // the undo stack is empty, leaving the position untouched.
        bool restoreState();

        void setFigure(const int &, const unsigned char &);

        [[nodiscard]] const unsigned char *getRawBoard() const;

        [[nodiscard]] figure_t getFigure(const int &) const;

        [[nodiscard]] std::string boardStateToFen() const;

        /* static members */
    public:
        static constexpr int ROW_LENGTH = 8;

        static constexpr int ROW_NUM = 8;

        static constexpr int BOARD_SIZE = ROW_LENGTH * ROW_NUM;

    private:
        /* member data  */
        unsigned char *board_;

        std::stack<std::array<unsigned char, BOARD_SIZE>> game_rev_;

    };

    // Ranks are listed a1-first, which is the reverse of standard FEN. The
    // string is fed to decodeFen() index 0 upwards, so the first rank written
    // is the one stored at indices 0..7.
    inline constexpr const char *INITIAL_FEN = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr/";

    // Piece letter <-> raw figure byte.
    //
    // These are functions over a switch rather than the std::maps they
    // replaced, because a map is filled during dynamic initialisation: a
    // GameState that is itself a global - the client has one - would decode
    // its opening position before the maps existed, and the order of the two
    // is not something a translation unit gets to decide.

    // The piece type a FEN letter stands for, or 0 if it is not a piece.
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

    unsigned char *decodeFen(const std::string &);

    // index <-> (rank, file) conversions. Pure arithmetic on absolute indices.
    board_node_t GetSquare(const int &);

    int GetNumFromNode(const board_node_t &);

    constexpr bool OnBoard(const board_node_t &node) {
        return node.x_ >= 0 && node.x_ < board_t::ROW_LENGTH &&
               node.y_ >= 0 && node.y_ < board_t::ROW_NUM;
    }

}

#endif
