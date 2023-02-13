//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef GOSCHESS_COMPUTATIONGLOBALS_HH
#define GOSCHESS_COMPUTATIONGLOBALS_HH

#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "../../global/MainOjbects/GameObjects.hh"
#include "../../../Render/Globals/RenderGlobals.hh"


namespace GosChess {

    struct offset_t {
        offset_t(int8_t north, int8_t south, int8_t west, int8_t east) :
                n(north), s(south), w(west), e(east),
                ne(std::min(north, east)),
                nw(std::min(north, west)),
                se(std::min(south, east)),
                sw(std::min(south, west)) {}

        offset_t() = default;

        int operator[](const int &index) {
            if (index < 0 || index >= 8) {
                throw std::invalid_argument("given index is out of range");
            }
            return ((std::int8_t *) this)[index];
        }

        int8_t n{};
        int8_t s{};
        int8_t w{};
        int8_t e{};
        int8_t ne{};
        int8_t se{};
        int8_t nw{};
        int8_t sw{};
    };


    struct action_t {
        int8_t move_from;
        int8_t move_to;

        action_t(int8_t move_from, int8_t move_to) : move_from(move_from), move_to(move_to) {};

        action_t() = default;

        bool operator==(const GosChess::action_t &rhm) const {
            return this->move_from == rhm.move_from && this->move_to == rhm.move_to;
        }
    };


    class action_hash_t {
    public:
        std::size_t operator()(const GosChess::action_t &move) const {
            return static_cast<size_t>(move.move_from * GosChess::board_t::ROW_LENGTH + move.move_from);
        }
    };

    extern const std::int8_t direction_offsets[];

    extern GosChess::offset_t precalculated_offsets[];

    typedef std::unordered_set<GosChess::action_t, GosChess::action_hash_t> actions_set_t;

    typedef std::unordered_map<std::int8_t, actions_set_t> action_map_t;

    extern action_map_t game_available_moves;

}


#endif //PROJECT_NAME_COMPUTATIONGLOBALS_H