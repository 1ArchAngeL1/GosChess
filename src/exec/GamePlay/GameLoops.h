//
// Created by Leo Gogichaishvili on 07.02.22.
//

#ifndef GOSCHESS_GAMELOOPS_H
#define GOSCHESS_GAMELOOPS_H

#include "GamePlayFunctional.h"


namespace GosChess {

    namespace RenderFlags {

        enum RenderMenuFLag {
            MAIN_MENU,
            HOSTING,
            JOINING,
            OPTION,
            NONE
        };

        extern RenderMenuFLag render_menu_flag;
    }

    enum LoopType {
        GAMEPLAY,
        MENU
    };

    void GameLoop(sf::RenderWindow &window, void (*OnUserInit)(sf::RenderWindow &, ...),
                  void (*OnUserUpdate)(sf::RenderWindow & ...),
                  GosChess::GameModeListener *listener, GosChess::LoopType, GosChess::Board *);

    void GameUpdate(sf::RenderWindow &, ...);


    void GameInit(sf::RenderWindow &, ...);

    void MenuUpdate(sf::RenderWindow &, ...);

    void MenuInit(sf::RenderWindow &, ...);

    extern sf::Clock delta_clock;

}


#endif //GOSCHESS_GAMELOOPS_H
