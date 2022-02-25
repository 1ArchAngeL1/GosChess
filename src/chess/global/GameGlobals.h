
#ifndef GOSCHESS_GAMEGLOBALS_H
#define GOSCHESS_GAMEGLOBALS_H


#include "main_objects//GameObjects.h"


namespace GosChess {


    enum GameStatus {
        INGAME,
        FINISHED
    };

    extern GosChess::Color color_to_play;

    extern GosChess::Color enemy_color;

    extern GosChess::Color player_color;

}



#endif
