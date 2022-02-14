
#include <iostream>
#include "../chess/global/MainObjects/GameObjects.h"
#include "../render/GamePlayRender.h"
#include "GamePlay/GamePlayFunctional.h"
#include "GamePlay/GameLoops.h"

#include <SFML/Graphics/RenderWindow.hpp>

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

int main() {

//    sf::RenderWindow window(sf::VideoMode(640, 480), "ImGui + SFML = <3");
//    window.setFramerateLimit(60);
//    ImGui::SFML::Init(window);
//    ImGuiIO *imgui_io = &ImGui::GetIO();
//    imgui_io->FontGlobalScale = 3.f;
//
//    sf::Clock deltaClock;
//    while (window.isOpen()) {
//        sf::Event event;
//        while (window.pollEvent(event)) {
//            ImGui::SFML::ProcessEvent(window, event);
//
//            if (event.type == sf::Event::Closed) {
//                window.close();
//            }
//        }
//
//        ImGui::SFML::Update(window, deltaClock.restart());
//
//        ImGui::ShowDemoWindow();
//        window.clear();
//        ImGui::SFML::Render(window);
//        window.display();
//    }
//
//    ImGui::SFML::Shutdown();


    GosChess::MenuNetworkMode();
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height, 24), "window");
    GosChess::MainMenuListener menu_listener(window);
    GosChess::GameLoop(window, GosChess::MenuInit, GosChess::MenuUpdate, GosChess::CheckMenuModeFinished,
                       &menu_listener,
                       nullptr);
    GosChess::GamePlayNetworkMode();
    GosChess::MultiPlayerListener game_listener(window);
    GosChess::Board board(GosChess::GetInitialFenBoard());
    GosChess::GameLoop(window, GosChess::OnlineGameInit, GosChess::OnlineGameUpdate, GosChess::CheckGameModeFinished,
                       &game_listener,
                       &board);
}







