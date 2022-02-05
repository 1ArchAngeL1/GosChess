
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../chess/global/MainObjects/GameObjects.h"
#include "../chess/global/GameGlobals.h"
#include "../render/GameDraw.h"
#include "GamePlay/GamePlayFunctional.h"



#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(640, 480), "ImGui + SFML = <3");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    ImGuiIO* imgui_io = &ImGui::GetIO();
    imgui_io->FontGlobalScale = 2.f;
    imgui_io->IniFilename = NULL;


    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::ShowDemoWindow();

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::Button("j");
        ImGui::End();
        window.clear();
        window.draw(shape);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
}

//int main() {
//    char type;
//    std::cin >> type;
//
//    if (type == 's') {
//        GosChess::HostGame();
//        GosChess::InitHost();
//    } else if (type == 'c') {
//        std::string ip_adress = "127.0.0.1";
//        GosChess::remote_ip = sf::IpAddress(ip_adress);
//        GosChess::JoinGame();
//        GosChess::InitClient();
//    } else {
//        return 0;
//    }
//
//    std::string fen_string = GosChess::GetInitialFenBoard();
//    GosChess::Board board(fen_string);
//    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "GosChess");
//    GosChess::DrawingConfig();
//    GosChess::LoadChessFigureSprites();
//    GosChess::GenerateOffsets();
//    GosChess::GameMode* performer = new GosChess::MultiPlayer(window);
//    GosChess::CalculateAvailableMoves(board.GetRawBoard());
//    while (window.isOpen()) {
//        if(GosChess::IsGameFinished()) {
//            GosChess::client_listener.close();
//            delete performer;
//            window.close();
//        }
//
//        sf::Event event;
//
//        while (window.pollEvent(event)) {
//            if (event.type == sf::Event::Closed) {
//                GosChess::client_listener.close();
//                window.close();
//            }
//        }
//
//        GosChess::CheckReceivedMove(GosChess::ReceiveMove(), board);
//        GosChess::InputHandle::Listen();
//        if (GosChess::player_color == GosChess::color_to_play &&
//            GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
//            performer->MouseClicked(board);
//        }
//        if(GosChess::IsGameFinished()) {
//            GosChess::client_listener.close();
//            delete performer;
//            window.close();
//        }
//        window.clear(GosChess::background_color);
//        GosChess::DrawCurrentBoardState(board.GetRawBoard(), window);
//        window.display();
//    }
//
//    return 0;
//}







