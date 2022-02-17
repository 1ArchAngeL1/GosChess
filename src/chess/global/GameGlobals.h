
#ifndef GOSCHESS_GAMEGLOBALS_H
#define GOSCHESS_GAMEGLOBALS_H


#include "MainObjects/GameObjects.h"


namespace GosChess {


    enum GameStatus {
        INGAME,
        FINISHED
    };

    extern Color color_to_play;

    extern Color enemy_color;

    extern Color player_color;

}



#endif
