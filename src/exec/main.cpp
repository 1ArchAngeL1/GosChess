
#include <iostream>
#include "../chess/global/MainObjects/GameObjects.h"
#include "../render/GameDraw.h"
#include "GamePlay/GamePlayFunctional.h"
#include "GamePlay/GameLoops.h"
#include <SFML/Graphics/RenderWindow.hpp>



int main() {
    GosChess::InitNewtork();
    char type;

    std::cin >> type;
    if(type == 's') {
        GosChess::HostInit();
    } else {
        GosChess::remote_ip = sf::IpAddress("127.0.0.1");
        GosChess::JoinInit();
    }

    while(!GosChess::connected);
    if(type == 's') {
        GosChess::InitialSend();
        std::cout << GosChess::player_color << std::endl;

        std::this_thread::sleep_for(std::chrono::duration<int>(2));
        GosChess::SendMove(GosChess::Move(1,2));
    }else {
        GosChess::InitialReceive();
        std::cout << GosChess::player_color << std::endl;
        std::optional<GosChess::Move> mv = std::nullopt;
        while(mv == std::nullopt) {
           mv = GosChess::ReceiveMove();
        }
        std::cout << (int)mv.value().move_from << " " << (int)mv.value().move_to << std::endl;
    }


    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "window");
    GosChess::MainMenuListener listener(window);
    GosChess::GameLoop(window,GosChess::MenuInit,GosChess::MenuUpdate,&listener);

}







