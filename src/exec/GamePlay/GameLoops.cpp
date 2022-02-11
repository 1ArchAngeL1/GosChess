//
// Created by Leo Gogichaishvili on 07.02.22.
//

#include <imgui-SFML.h>
#include <imgui.h>
#include "GameLoops.h"
#include "../../render/menu/MenuRender.h"

typedef void (*OnUserInit)(sf::RenderWindow &, ...);

typedef void (*OnUserUpdate)(sf::RenderWindow & ...);

void GosChess::GameLoop(sf::RenderWindow &window, OnUserInit init, OnUserUpdate update,
                        GosChess::GameModeListener *listener, GosChess::LoopType type, GosChess::Board *game_board) {

    type == GosChess::LoopType::MENU ? init(window) : init(window, game_board);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                GosChess::KillNetwork();
                window.close();
                exit(3);
            }
        }
        if (GosChess::connected && type == GosChess::LoopType::MENU) {
            return;
        }
        type == GosChess::LoopType::MENU ? update(window) : update(window, listener, game_board);
    }
}

void GosChess::GameInit(sf::RenderWindow &window, ...) {
    va_list args;
    va_start(args, window);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    GosChess::ChessDrawingConfig();
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
    GosChess::CalculateAvailableMoves(board->GetRawBoard());
}

void GosChess::GameUpdate(sf::RenderWindow &window, ...) {
    va_list args;
    va_start(args, window);
    GosChess::GameModeListener *listener = va_arg(args, GosChess::GameModeListener*);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);

    if (GosChess::IsGameFinished()) {
        GosChess::KillNetwork();
        window.close();
    }

    GosChess::CheckReceivedMove(GosChess::ReceiveMove(), *board);

    GosChess::InputHandle::Listen();
    if (GosChess::player_color == GosChess::color_to_play &&
        GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
        listener->Action(*board);
    }

    window.clear();
    GosChess::DrawCurrentBoardState(board->GetRawBoard(), window);
    window.display();
}

void GosChess::MenuInit(sf::RenderWindow &window, ...) {
    ImGui::SFML::Init(window);
    GosChess::MenuRenderConfig();
    ImGuiIO *imgui_io = &ImGui::GetIO();
    //imgui_io->Fonts->AddFontFromFileTTF("GosChess/resources/Lato2FFL/Lato-Black.ttf", 10);
    imgui_io->FontGlobalScale = 3.f;
    GosChess::delta_clock.restart();
}

void GosChess::MenuUpdate(sf::RenderWindow &window, ...) {
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
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderOptionsWidgets, window);
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

GosChess::RenderFlags::RenderMenuFLag GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::RenderMenuFLag::MAIN_MENU;

sf::Clock GosChess::delta_clock;