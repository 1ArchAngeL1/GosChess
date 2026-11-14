//
// Created by Leo Gogichaishvili on 07.02.22.
//

#ifndef GOSCHESS_GAMELOOPS_HH
#define GOSCHESS_GAMELOOPS_HH

#include "GamePlayFunctional.hh"
#include "../../Render/Menu/MenuRender.hh"
#include "../../Network/ServerConnection.hh"
#include "../../Chess/AI/AICalculations.hh"

namespace GosChess {

    enum RenderMenuFLag {
        MAIN_MENU,
        COMPUTER,
        ONLINE,
        OPTION,
        GAME_RESULT,
        NONE
    };

    extern RenderMenuFLag render_menu_flag;


    // The board used to be threaded through here as a varargs argument, with
    // each callback pulling it back out by hand. There is now one GameState
    // the callbacks read directly (ClientGame.hh), so the loop no longer has
    // anything mode specific to carry and the varargs are gone with it.
    void GameLoop(sf::RenderWindow &, void (*OnUserInit)(sf::RenderWindow &),
                  void (*OnUserUpdate)(sf::RenderWindow &, sf::Clock *), bool (*ModeTerminator)(),
                  GosChess::GameModeListener *);


    void AIGameInit(sf::RenderWindow &);

    void AIGameUpdate(sf::RenderWindow &, sf::Clock *);

    void OnlineGameInit(sf::RenderWindow &);

    void OnlineGameUpdate(sf::RenderWindow &, sf::Clock *);

    void MenuInit(sf::RenderWindow &);

    void MenuUpdate(sf::RenderWindow &, sf::Clock *);

    bool CheckOnlineModeFinished();

    bool CheckSinglePLayerFinished();

    bool CheckMenuModeFinished();

}


#endif //GOSCHESS_GAMELOOPS_HH
