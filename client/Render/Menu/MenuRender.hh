//
// Created by Leo Gogichaishvili on 01.02.22.
//

#ifndef GOSCHESS_MENURENDER_HH
#define GOSCHESS_MENURENDER_HH

#include "../../Exec/GamePlay/GamePlayFunctional.hh"
#include "../../Exec/GamePlay/GameLoops.hh"
#include "../Globals/RenderGlobals.hh"
#include <imgui.h>
#include <imgui-SFML.h>

namespace GosChess {

    void RenderMainMenuBackground(sf::RenderWindow &, ImGuiContext *);

    void RenderBackgroundWallpaper(sf::RenderWindow &);

    void RenderMainMenuWidgets(sf::RenderWindow &, ImGuiContext *);

    void RenderOptionsBackground(sf::RenderWindow &, ImGuiContext *);

    void RenderOptionsWidgets(sf::RenderWindow &, ImGuiContext *);

    void RenderGameResultBackground(sf::RenderWindow &, ImGuiContext *);

    void RenderGameResultWidgets(sf::RenderWindow &, ImGuiContext *);

    // One page replaces the old Host and Join pair: there is no host any more,
    // only a server, and both players do the same thing to reach it.
    void RenderOnlineWidgets(sf::RenderWindow &, ImGuiContext *);

    void RenderAiGameBackground(sf::RenderWindow &, ImGuiContext *);

    void RenderAiGameWidgets(sf::RenderWindow &, ImGuiContext *);

    void RenderMenu(void (*)(sf::RenderWindow &, ImGuiContext *), void (*)(sf::RenderWindow &, ImGuiContext *),
                    sf::RenderWindow &);

    void MenuRenderConfig();

    extern int main_menu_width;

    extern int main_menu_height;

}


#endif //PROJECT_NAME_MENURENDER_H
