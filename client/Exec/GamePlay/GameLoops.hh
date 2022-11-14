//
// Created by Leo Gogichaishvili on 07.02.22.
//

#ifndef GOSCHESS_GAMELOOPS_HH
#define GOSCHESS_GAMELOOPS_HH

#include "GamePlayFunctional.hh"
#include "../../Render/menu/MenuRender.hh"
#include "../../Network/GameNetwork.hh"
#include "../../chess/Computation/Ai/AICalculations.hh"

namespace GosChess {

    enum RenderMenuFLag {
        MAIN_MENU,
        COMPUTER,
        HOSTING,
        JOINING,
        OPTION,
        GAME_RESULT,
        NONE
    };

    extern RenderMenuFLag render_menu_flag;


    void GameLoop(sf::RenderWindow &, void (*OnUserInit)(sf::RenderWindow &, ...),
                  void (*OnUserUpdate)(sf::RenderWindow &, sf::Clock *, ...), bool (*ModeTeminator)(),
                  GosChess::GameModeListener *, GosChess::board_t *);


    void AIGameInit(sf::RenderWindow &, ...);

    void AIGameUpdate(sf::RenderWindow &, sf::Clock *, ...);

    void OnlineGameInit(sf::RenderWindow &, ...);

    void OnlineGameUpdate(sf::RenderWindow &, sf::Clock *, ...);

    void MenuInit(sf::RenderWindow &, ...);

    void MenuUpdate(sf::RenderWindow &, sf::Clock *, ...);

    bool CheckOnlineModeFinished();

    bool CheckSinglePLayerFinished();

    bool CheckMenuModeFinished();

    extern short time_limit_minutes;

}


#endif //GOSCHESS_GAMELOOPS_HH
