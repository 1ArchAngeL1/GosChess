//
// Created by Leo Gogichaishvili on 07.02.22.
//

#include <imgui-SFML.h>
#include <imgui.h>
#include "GameLoops.h"
#include "../../render/menu/MenuRender.h"

typedef void (*OnUserInit)(sf::RenderWindow &);

typedef void (*OnUserUpdate)(sf::RenderWindow &, GosChess::GameModeListener *listener, ...);

void GosChess::GameLoop(sf::RenderWindow &window, OnUserInit init_func,
                        OnUserUpdate update_func,
                        GosChess::GameModeListener *listener) {
    init_func(window);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        update_func(window, listener);

    }
}


void GosChess::GameInit(sf::RenderWindow &window) {
    std::string fen_string = GosChess::GetInitialFenBoard();
    GosChess::Board board(fen_string);
    GosChess::ChessDrawingConfig();
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
    GosChess::CalculateAvailableMoves(board.GetRawBoard());
}

void GosChess::GameUpdate(sf::RenderWindow &window, GosChess::GameModeListener *listener, ...) {
    va_list args;
    va_start(args, listener);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    if (GosChess::IsGameFinished()) {
        GosChess::client_listener.close();
        delete listener;
        window.close();
    }
    GosChess::CheckReceivedMove(GosChess::ReceiveMove(), *board);
    GosChess::InputHandle::Listen();
    if (GosChess::player_color == GosChess::color_to_play &&
        GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
        listener->MouseClicked(*board);
    }

    window.clear();
    GosChess::DrawCurrentBoardState(board->GetRawBoard(), window);
    window.display();
}

void GosChess::MenuUpdate(sf::RenderWindow &window, GosChess::GameModeListener *listener, ...) {
    ImGui::SFML::Update(window, GosChess::delta_clock.restart());

    switch (GosChess::RenderFlags::render_menu_flag) {
        case GosChess::RenderFlags::RenderMenuFLag::MAIN_MENU:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderMainMenuWidgets, window);
            break;
        case GosChess::RenderFlags::RenderMenuFLag::JOINING:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderJoinGameWidgets, window);
            break;
        case GosChess::RenderFlags::RenderMenuFLag::HOSTING:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderHostGameWidgets, window);
            break;
        case GosChess::RenderFlags::RenderMenuFLag::OPTION:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderHostGameWidgets, window);
            break;
        case GosChess::RenderFlags::RenderMenuFLag::NONE:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderJoinGameWidgets, window);
            break;
        default:
            break;
    }

    window.clear();
    GosChess::RenderBackgroundWallpaper(window);
    ImGui::SFML::Render(window);
    window.display();

}

void GosChess::MenuInit(sf::RenderWindow &window) {
    ImGui::SFML::Init(window);
    GosChess::MenuRenderConfig();
    ImGuiIO *imgui_io = &ImGui::GetIO();
    imgui_io->FontGlobalScale = 3.f;
    GosChess::delta_clock.restart();
}

GosChess::RenderFlags::RenderMenuFLag GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::RenderMenuFLag::MAIN_MENU;

sf::Clock GosChess::delta_clock;