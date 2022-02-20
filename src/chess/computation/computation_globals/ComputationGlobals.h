//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef GOSCHESS_COMPUTATIONGLOBALS_H
#define GOSCHESS_COMPUTATIONGLOBALS_H


#include "../../global/MainObjects/GameObjects.h"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "../../../render/globals/RenderGlobals.h"


namespace GosChess {

    struct Offset {
        Offset(int8_t north, int8_t south, int8_t west, int8_t east) :
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

        Move(int8_t move_from, int8_t move_to) : move_from(move_from), move_to(move_to) {};

        Move() = default;

        bool operator==(const GosChess::Move &rhm) const {
            return this->move_from == rhm.move_from && this->move_to == rhm.move_to;
        }
    };


    class MoveHash {
    public:
        std::size_t operator()(GosChess::Move const &move) const {
            return static_cast<size_t>(move.move_from * move.move_to + move.move_from);
        }
    };

    extern const std::int8_t direction_offsets[];

    extern GosChess::Offset precalculated_offsets[];

    extern std::unordered_map<std::int8_t, std::unordered_set<GosChess::Move, GosChess::MoveHash>> available_moves;

}


#endif //PROJECT_NAME_COMPUTATIONGLOBALS_H