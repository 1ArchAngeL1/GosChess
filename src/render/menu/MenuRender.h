//
// Created by Leo Gogichaishvili on 01.02.22.
//

#ifndef GOSCHESS_MENURENDER_H
#define GOSCHESS_MENURENDER_H

#include "../../exec/game_play/GamePlayFunctional.h"
#include "../../exec/game_play/GameLoops.h"
#include "../globals/RenderGlobals.h"
#include <imgui.h>
#include <imgui-SFML.h>

namespace GosChess {

    void RenderMainMenuBackground(sf::RenderWindow &, ImGuiContext *);

    void RenderBackgroundWallpaper(sf::RenderWindow&);

    void RenderMainMenuWidgets(sf::RenderWindow &, ImGuiContext *);

    void RenderOptionsBackground(sf::RenderWindow&,ImGuiContext*);

    void RenderOptionsWidgets(sf::RenderWindow&,ImGuiContext*);

    void RenderHostGameBackground(sf::RenderWindow&,ImGuiContext*);

    void RenderHostGameWidgets(sf::RenderWindow&,ImGuiContext*);

    void RenderGameResultBackground(sf::RenderWindow&,ImGuiContext*);

    void RenderGameResultWidgets(sf::RenderWindow&,ImGuiContext*);

    void RenderJoinGameBackground(sf::RenderWindow&,ImGuiContext*);

    void RenderJoinGameWidgets(sf::RenderWindow&, ImGuiContext*);

    void RenderMenu(void (*)(sf::RenderWindow &,ImGuiContext *), void (*)(sf::RenderWindow &,ImGuiContext *), sf::RenderWindow &);

    void MenuRenderConfig();

    extern int main_menu_width;

    extern int main_menu_height;

    extern bool menu_active_flag;

}


#endif //PROJECT_NAME_MENURENDER_H
