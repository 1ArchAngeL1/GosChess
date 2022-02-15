//
// Created by Leo Gogichaishvili on 07.02.22.
//

#ifndef GOSCHESS_GAMELOOPS_H
#define GOSCHESS_GAMELOOPS_H

#include "GamePlayFunctional.h"
#include "../../render/menu/MenuRender.h"
#include "../../network/GameNetwork.h"
#include "../../chess/computation/Ai/AICalculations.h"

namespace GosChess {

    enum RenderMenuFLag {
        MAIN_MENU,
        HOSTING,
        JOINING,
        OPTION,
        NONE
    };

    extern RenderMenuFLag render_menu_flag;


    void GameLoop(sf::RenderWindow &, void (*OnUserInit)(sf::RenderWindow &, ...),
                  void (*OnUserUpdate)(sf::RenderWindow &, sf::Clock *, ...), bool (*ModeTeminator)(),
                  GameModeListener *, GosChess::Board *);


    void AIGameInit(sf::RenderWindow &, ...);

    void AIGameUpdate(sf::RenderWindow &, sf::Clock *...);

    void OnlineGameInit(sf::RenderWindow &, ...);

    void OnlineGameUpdate(sf::RenderWindow &, sf::Clock *...);

    void MenuInit(sf::RenderWindow &, ...);

    void MenuUpdate(sf::RenderWindow &, sf::Clock *, ...);

    bool CheckGameModeFinished();

    bool CheckMenuModeFinished();

    extern short time_limit_minutes;

}


#endif //GOSCHESS_GAMELOOPS_H
