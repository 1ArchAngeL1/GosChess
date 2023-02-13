
#ifndef GOSCHESS_GAMEOBJECTS_HH
#define GOSCHESS_GAMEOBJECTS_HH

#include <iostream>
#include <map>
#include <array>
#include <stack>


namespace GosChess {

    enum color_t {
        WHITE, BLACK
    };

    enum figure_types_t {
        ROOK = 1, BISHOP,
        PAWN, KNIGHT,
        KING, QUEEN
    };


    struct board_node_t {
        int x_;
        int y_;

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

        ~board_t();

        /* member functions */
        void setState(const std::string &state);

        void saveState();

        void setFigure(const int &, const unsigned char &);

        //[[nodiscard]]
        const unsigned char *getRawBoard() const;

        //[[nodiscard]]
        figure_t getFigure(const int &) const;

        //[[nodiscard]]
        std::string boardStateToFen() const;

        /* static members */
    public:
        static constexpr int ROW_LENGTH = 8;

        static constexpr int ROW_NUM = 8;

        static constexpr int BOARD_SIZE = ROW_LENGTH * ROW_NUM;

    public:
        /* friend functions */
        friend bool UndoMove(GosChess::board_t &);

    private:
        /* member data  */
        unsigned char *board_;

        std::stack<std::array<unsigned char, BOARD_SIZE>> game_rev_;

    };

    extern const std::map<unsigned char, char> FIG_TO_FEN;

    extern const std::map<char, unsigned char> FEN_TO_FIG;

    unsigned char *decodeFen(const std::string &);

}

#endif
