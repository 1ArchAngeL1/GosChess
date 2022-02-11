//
// Created by Leo Gogichaishvili on 07.02.22.
//

#ifndef GOSCHESS_GAMELOOPS_H
#define GOSCHESS_GAMELOOPS_H

#include "GamePlayFunctional.h"
#include "../../render/menu/MenuRender.h"
#include <imgui-SFML.h>
#include <imgui.h>
#include "../../network/GameNetwork.h"


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

    void GameLoop(sf::RenderWindow &, void (*OnUserInit)(sf::RenderWindow &, ...),
                  void (*OnUserUpdate)(sf::RenderWindow &...), bool (*ModeTeminator)(),
                  GameModeListener *, GosChess::LoopType, GosChess::Board *);

    void GameUpdate(sf::RenderWindow &, ...);


    void GameInit(sf::RenderWindow &, ...);

    void MenuUpdate(sf::RenderWindow &, ...);

    void MenuInit(sf::RenderWindow &, ...);

    extern sf::Clock delta_clock;

}


#endif //GOSCHESS_GAMELOOPS_H
