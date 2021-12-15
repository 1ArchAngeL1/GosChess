
#ifndef PROJECT_NAME_GAMEOBJECTS_H
#define PROJECT_NAME_GAMEOBJECTS_H

#include <iostream>
#include <map>

namespace GosChess {
    enum Color {
        WHITE, BLACK
    };

    enum FigureTypes {
        ROOK = 1, BISHOP,
        PAWN, KNIGHT,
        KING, QUEEN
    };

    union Figure {
        explicit Figure(const unsigned char &color, const unsigned char &type);

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

        static const std::map<char, unsigned char> FEN_TO_FIG;

        static const std::map<unsigned char, char> FIG_TO_FEN;
    public:

        static constexpr int ROW_LENGTH = 8;

        static constexpr int ROW_NUM = 8;

        static constexpr int BOARD_SIZE = ROW_LENGTH * ROW_NUM;

        static unsigned char *DecodeFen(std::string fen_str);

        static Figure FigureFromChar(const unsigned char &figure_rep);

        explicit Board(std::string initial_state);

        Figure GetPosition(const int &grid_num);

        void SetPosition(const int &grid_num, unsigned char figure);

        std::string BoardStateToFen();
    };

}


#endif
