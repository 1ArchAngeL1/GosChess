
#include <iostream>
#include "../chess/global/MainObjects/GameObjects.h"
#include "../render/GameDraw.h"
#include "GamePlay/GamePlayFunctional.h"
#include "GamePlay/GameLoops.h"
#include <SFML/Graphics/RenderWindow.hpp>


int main() {

    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "window");
    GosChess::MainMenuListener listener(window);
    GosChess::GameLoop(window, GosChess::MenuInit, GosChess::MenuUpdate, &listener);

}







