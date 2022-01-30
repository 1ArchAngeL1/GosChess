
#ifndef PROJECT_NAME_GAMEOBJECTS_H
#define PROJECT_NAME_GAMEOBJECTS_H

#include <iostream>
#include <map>
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


    struct Cell {
        int x;
        int y;

        Cell() {}

        Cell(int y, int x) : y(y), x(x) {}

        Cell operator+(const Cell &cell) {
            return Cell(this->y + cell.y, this->x + cell.x);
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
    private:
        unsigned char *board;

        std::stack<unsigned char *> game_rev;
    public:
        explicit Board(std::string);

        const unsigned char *GetRawBoard() const;

        Figure GetPosition(const int &) const;

        void SetPosition(const int &, unsigned char);

        std::string BoardStateToFen();

        void SaveState();

        void Undo();

        static unsigned char *DecodeFen(std::string);

        static const std::map<char, unsigned char> FEN_TO_FIG;

        static const std::map<unsigned char, char> FIG_TO_FEN;

        static constexpr int ROW_LENGTH = 8;

        static constexpr int ROW_NUM = 8;

        static constexpr int BOARD_SIZE = ROW_LENGTH * ROW_NUM;


    };

}

#endif
