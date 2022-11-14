
#ifndef GOSCHESS_GAMEGLOBALS_HH
#define GOSCHESS_GAMEGLOBALS_HH


#include "MainOjbects/GameObjects.hh"


namespace GosChess {


    enum game_status_t {
        INGAME,
        FINISHED
    };

    extern GosChess::color_t color_to_play;

    extern GosChess::color_t enemy_color;

    extern GosChess::color_t player_color;

}



#endif
