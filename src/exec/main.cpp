
#include <iostream>
#include "../chess/global/MainObjects/GameObjects.h"
#include "../render/GameDraw.h"
#include "GamePlay/GamePlayFunctional.h"
#include "GamePlay/GameLoops.h"
#include "../network/GameNetwork.h"
#include <SFML/Graphics/RenderWindow.hpp>


int main() {
//    GosChess::initNewtork();
//    char type;
//
//    std::cin >> type;
//    if(type == 's') {
//        GosChess::HostInit();
//    } else {
//        GosChess::remote_ip = sf::IpAddress("127.0.0.1");
//        GosChess::JoinInit();
//    }
//
//    while(!GosChess::connected);
//    if(type == 's') {
//        GosChess::InitialSend();
//        std::cout << GosChess::player_color << std::endl;
//        std::this_thread::sleep_for(std::chrono::duration<int>(2));
//    }else {
//        GosChess::InitialReceive();
//        std::optional<GosChess::Move> mv = std::nullopt;
//        while(mv == std::nullopt) {
//           mv = GosChess::ReceiveMove();
//        }
//        std::cout << (int)mv.value().move_from << " " << (int)mv.value().move_to << std::endl;
//    }
    GosChess::MenuNetworkMode();
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "window");
    GosChess::MainMenuListener menu_listener(window);
    GosChess::GameLoop(window, GosChess::MenuInit, GosChess::MenuUpdate, GosChess::CheckMenuModeFinished,
                       &menu_listener, GosChess::LoopType::MENU,
                       nullptr);
    if (GosChess::connection_role == GosChess::ConnectionType::HOST) {
        GosChess::InitialSend();
    } else if (GosChess::connection_role == GosChess::ConnectionType::CLIENT) {
        GosChess::InitialReceive();
    }
    window.clear();
    GosChess::GamePlayNetworkMode();
    GosChess::MultiPlayerListener game_listener(window);
    GosChess::Board board(GosChess::GetInitialFenBoard());
    GosChess::GameLoop(window, GosChess::GameInit, GosChess::GameUpdate,GosChess::CheckGameModeFinished, &game_listener, GosChess::LoopType::GAMEPLAY,
                       &board);
}







