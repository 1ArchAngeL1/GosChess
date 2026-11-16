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

    // The loop owns the frame: it polls input, hands ImGui its tick, lets the
    // mode read the player's intentions, lets it draw, and presents. The modes
    // used to each clear and display the window themselves, which is why a
    // panel drawn with ImGui could not be layered over a board drawn with SFML.
    void GameLoop(sf::RenderWindow &, void (*OnUserInit)(sf::RenderWindow &),
                  void (*OnUserDraw)(sf::RenderWindow &, float delta_seconds),
                  bool (*ModeTerminator)(), GosChess::GameModeListener *);


    void AIGameInit(sf::RenderWindow &);

    void AIGameDraw(sf::RenderWindow &, float delta_seconds);

    void OnlineGameInit(sf::RenderWindow &);

    void OnlineGameDraw(sf::RenderWindow &, float delta_seconds);

    void MenuInit(sf::RenderWindow &);

    void MenuDraw(sf::RenderWindow &, float delta_seconds);

    bool CheckOnlineModeFinished();

    bool CheckSinglePLayerFinished();

    bool CheckMenuModeFinished();

}


#endif //GOSCHESS_GAMELOOPS_HH
