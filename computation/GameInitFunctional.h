
#ifndef PROJECT_NAME_GAMEINITFUNCTIONAL_H
#define PROJECT_NAME_GAMEINITFUNCTIONAL_H

#include "GameObjects.h"

namespace GosChess {

    struct offsets {
        offsets(std::int8_t north, std::int8_t south,
                std::int8_t west, std::int8_t east) : north(north), south(south), west(west), east(east) {
            north_east = std::min(north, east);
            north_west = std::min(north, west);
            south_east = std::min(south, east);
            south_west = std::min(south, west);
        }

        offsets() {}

        std::int8_t north;
        std::int8_t south;
        std::int8_t west;
        std::int8_t east;
        std::int8_t north_east;
        std::int8_t south_east;
        std::int8_t north_west;
        std::int8_t south_west;
    };

    void GenerateOffsets();

    extern offsets *precalculated_offsets;

}

#endif
