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

    void GameLoop(sf::RenderWindow &window, void (*OnUserInit)(sf::RenderWindow &),
                  void (*OnUserListen)(sf::RenderWindow &, GosChess::GameModeListener *),
                  void (*OnUserUpdate)(sf::RenderWindow &, GosChess::GameModeListener *listener),
                  GosChess::GameModeListener *listener);

    void GameUpdate(sf::RenderWindow &, GosChess::GameModeListener *);

    void GameListen(sf::RenderWindow &, GosChess::GameModeListener *);

    void GameInit(sf::RenderWindow &);

    void MenuUpdate(sf::RenderWindow &, GosChess::GameModeListener *);

    void MenuListen(sf::RenderWindow &, GosChess::GameModeListener *);

    void MenuInit(sf::RenderWindow &);

    extern sf::Clock delta_clock;

}


#endif //GOSCHESS_GAMELOOPS_H
