
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../chess/global/MainObjects/GameObjects.h"
#include "../chess/global/GameGlobals.h"
#include "../render/GameDraw.h"
#include "GamePlay/GamePlayFunctional.h"


#include "imgui.h"
#include "imgui-SFML.h"
#include "../render/menu/MenuRender.h"
#include "GamePlay/GameLoops.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>


int main() {
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "window");
    GosChess::GameLoop(window,GosChess::MenuInit,GosChess::MenuListen,GosChess::MenuUpdate);

}







