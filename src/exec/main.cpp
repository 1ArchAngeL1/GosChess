
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
    std::optional<GosChess::Cell> src_cell;
    std::optional<GosChess::Cell> trg_cell;

    while (window.isOpen()) {
        GosChess::InputHandle::Listen();
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                GosChess::client_listener.close();
                window.close();
            }

        }

        GosChess::CheckReceivedMove(GosChess::ReceiveMove(), board);

        if (GosChess::player_color == GosChess::color_to_play &&
            GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
            if (!GosChess::highlited) {
                src_cell = GosChess::ChooseSrcFigure(board, window);
            } else {
                trg_cell = GosChess::ChooseTrgFigure(board, window);
            }
            if (src_cell.has_value() && trg_cell.has_value()) {
                int8_t from = static_cast<int8_t>(GosChess::GetNumFromNode(src_cell.value()));
                int8_t to = static_cast<int8_t>(GosChess::GetNumFromNode(trg_cell.value()));
                if (GosChess::Play(board, GosChess::Move(GosChess::Move(from, to)))) {
                    GosChess::ChangeActiveColour();
                    GosChess::SendMove(GosChess::Move(from, to));
                }
                src_cell = std::nullopt;
                trg_cell = std::nullopt;
                GosChess::CalculateAvailableMoves(board.GetRawBoard());
                if (GosChess::CheckMate(board, GosChess::color_to_play)) {
                    break;
                }

            }
        }

        window.clear(GosChess::background_color);
        GosChess::DrawCurrentBoardState(board.GetRawBoard(), window);
        window.display();
    }

    return 0;
}







