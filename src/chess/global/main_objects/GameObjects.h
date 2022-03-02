
#ifndef GOSCHESS_GAMEOBJECTS_H
#define GOSCHESS_GAMEOBJECTS_H

#include <iostream>
#include <map>
#include <array>
#include <stack>


namespace GosChess {

    enum Color {
        WHITE, BLACK
    };

    enum FigureTypes {
        ROOK = 1, BISHOP,
        PAWN, KNIGHT,
        KING, QUEEN
    };


    struct board_square {
        int x;
        int y;

        board_square() = default;

        board_square(int y, int x) : y(y), x(x) {}

        board_square operator+(const board_square &cell) {
            return board_square(this->y + cell.y, this->x + cell.x);
        }
    };


    union Figure {
        explicit Figure(const unsigned char &, const unsigned char &);

        explicit Figure(const unsigned char &full_type) : full_type(full_type) {}

        explicit Figure() : full_type(0) {}

        struct {
            unsigned char color: 1;
            unsigned char type: 3;
        };
        unsigned char full_type;
    };


    class Board {
    public:
        explicit Board(std::string);

        Board() = default;

        Board(const Board &board);

        ~Board();

        void setState(const std::string &state);

        void saveState();

        void setPosition(const int &, unsigned char);

        const unsigned char *getRawBoard() const;

        Figure at(const int &) const;

        std::string boardStateToFen() const;

        // static members from here

        static unsigned char *decodeFen(std::string fen_str);

        static const std::map<char, unsigned char> FEN_TO_FIG;

        static const std::map<unsigned char, char> FIG_TO_FEN;

        static constexpr int ROW_LENGTH = 8;

        static constexpr int ROW_NUM = 8;

        static constexpr int BOARD_SIZE = ROW_LENGTH * ROW_NUM;

        friend bool UndoMove(GosChess::Board &);

    private:
        unsigned char *board;

        std::stack<std::array<unsigned char, BOARD_SIZE>> game_rev;

    };

}

#endif
