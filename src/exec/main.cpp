
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../chess/global/MainObjects/GameObjects.h"
#include "../chess/global/GameGlobals.h"
#include "../render/GameDraw.h"
#include "GamePlay/GamePlayFunctional.h"
#include "../network/GameNetwok.h"




int main() {
    char type;

    std::cin >> type;

    if (type == 's') {
        GosChess::HostGame();
        GosChess::InitHost();
        std::cout << "xosti bliad" << std::endl;
    } else if (type == 'c') {
        std::cout << "klienti" << std::endl;
        std::string ip_adress = "127.0.0.1";
        GosChess::remote_ip = sf::IpAddress(ip_adress);
        GosChess::JoinGame();
        GosChess::InitClient();
    } else {
        return 0;
    }

    std::string fen_string = GosChess::GetInitialFenBoard();
    GosChess::Board board(fen_string);
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "GosChess");
    GosChess::DrawingConfig();
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
    GosChess::GameMode* performer = new GosChess::MultiPlayer(window);
    GosChess::CalculateAvailableMoves(board.GetRawBoard());
    while (window.isOpen()) {
        if(GosChess::IsGameFinished()) {
            GosChess::client_listener.close();
            window.close();
        }

        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                GosChess::client_listener.close();
                window.close();
            }
        }

        GosChess::CheckReceivedMove(GosChess::ReceiveMove(), board);
        GosChess::InputHandle::Listen();
        if (GosChess::player_color == GosChess::color_to_play &&
            GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
            performer->MouseClicked(board);
        }

        window.clear(GosChess::background_color);
        GosChess::DrawCurrentBoardState(board.GetRawBoard(), window);
        window.display();
    }

    return 0;
}







