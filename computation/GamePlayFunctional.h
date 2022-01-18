
#ifndef PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
#define PROJECT_NAME_GAMEPLAYFUNCTIONAL_H

#include "../global/MainGameObjects/GameObjects.h"
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include "../global/GameGlobals.h"


namespace GosChess {


    struct Offset {
        Offset(std::int8_t north, std::int8_t south,
               std::int8_t west, std::int8_t east) : north(north), south(south), west(west), east(east) {
            north_east = std::min(north, east);
            north_west = std::min(north, west);
            south_east = std::min(south, east);
            south_west = std::min(south, west);
        }

        Offset() {}

        int operator[](const int &index) {
            if (index >= 0 && index < 8) {
                return ((std::int8_t *) this)[index];
            }
        }

        std::int8_t north;
        std::int8_t south;
        std::int8_t west;
        std::int8_t east;
        std::int8_t north_east;
        std::int8_t south_east;
        std::int8_t north_west;
        std::int8_t south_west;
    };


    struct Move {
        std::int8_t move_from;
        std::int8_t move_to;

        Move(std::int8_t move_from, std::int8_t move_to) : move_from(move_from), move_to(move_to) {};

        bool operator==(const Move &move) {
            return move_from == move.move_from && move_to == move.move_to;
        }

    };


    GosChess::Cell GetNode(const int & num);

    void GenerateOffsets();

    void CalculateAvailableMoves(const unsigned char *game_board);

    extern const std::int8_t const *direction_offsets;

    extern Offset *PrecalculatedOffsets;

    extern std::unordered_map<std::int8_t, std::vector<GosChess::Move>> available_moves;

    extern const unsigned char * SlidingPieces;

}



#endif
