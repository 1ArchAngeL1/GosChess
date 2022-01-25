
#ifndef PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
#define PROJECT_NAME_GAMEPLAYFUNCTIONAL_H

#include "../global/GameObjects.h"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "../global/GameGlobals.h"


namespace GosChess {

    struct Offset {
        Offset(std::int8_t north, std::int8_t south, std::int8_t west, std::int8_t east) :
                north(north), south(south), west(west), east(east),
                north_east(std::min(north, east)),
                north_west(std::min(north, west)),
                south_east(std::min(south, east)),
                south_west(std::min(south, west)) {}

        Offset() {}

        int operator[](const int &index) {
            if (index >= 0 && index < 8) {
                return ((std::int8_t *) this)[index];
            }
        }

        int8_t north;
        int8_t south;
        int8_t west;
        int8_t east;
        int8_t north_east;
        int8_t south_east;
        int8_t north_west;
        int8_t south_west;
    };


    struct Move {
        int8_t move_from;
        int8_t move_to;

        Move(int8_t move_from, std::int8_t move_to) : move_from(move_from), move_to(move_to) {};

        bool operator==(const GosChess::Move &rhm) const {
            return this->move_from == rhm.move_from && this->move_to == rhm.move_to;
        }
    };


    class MoveHash {
    public:
        std::size_t operator()(GosChess::Move const &_mv) const {
            return (std::size_t) _mv.move_to;
        }
    };


    GosChess::Cell GetNode(const int &num);

    bool MakeMove(GosChess::Move mv, GosChess::Board &brd);

    bool CanMakeMove(GosChess::Move mv);

    bool CheckMate(GosChess::Board &brd, GosChess::Color clr);

    int GetNumFromNode(const GosChess::Cell &_cell);

    GosChess::Cell GetNodeFromScreen(const float &_y, const float &_x);

    void GenerateOffsets();

    void CalculateAvailableMoves(const unsigned char *game_board);

    extern const std::int8_t *const direction_offsets;

    extern Offset *precalculated_offsets;

    extern std::unordered_map<std::int8_t, std::unordered_set<GosChess::Move, GosChess::MoveHash>> available_moves;

    extern const unsigned char *sliding_pieces;

    void ChangeActiveColour();

}


#endif
