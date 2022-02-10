//
// Created by Leo Gogichaishvili on 07.02.22.
//

#include <imgui-SFML.h>
#include <imgui.h>
#include "GameLoops.h"
#include "../../render/menu/MenuRender.h"

typedef void (*OnUserInit)(sf::RenderWindow &, ...);

typedef void (*OnUserUpdate)(sf::RenderWindow &, GosChess::GameModeListener *listener, ...);

void GosChess::GameLoop(sf::RenderWindow &window, OnUserInit init,
                        OnUserUpdate update,
                        GosChess::GameModeListener *listener, GosChess::LoopType type, ...) {
    GosChess::Board *board;
    if (type == GosChess::LoopType::GAMEPLAY) {
        va_list args;
        va_start(args, type);
        board = va_arg(args, GosChess::Board*);
        va_end(args);
    }
    init(window, board);
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);

            if (event.type == sf::Event::Closed) {
                GosChess::connection.disconnect();
                window.close();
                exit(3);
            }

        }
        if (GosChess::connected && type == GosChess::LoopType::MENU) {
            return;
        }
        update(window, listener, board);
    }
}

void GosChess::GameInit(sf::RenderWindow &window, ...) {
    va_list args;
    va_start(args, window);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    GosChess::ChessDrawingConfig();
    std::cout << "configi moxdai" << std::endl;
    GosChess::LoadChessFigureSprites();
    std::cout << "figurebi chaiseta moxdai" << std::endl;
    GosChess::GenerateOffsets();
    std::cout << "opesetebica moxidai" << std::endl;
    GosChess::CalculateAvailableMoves(board->GetRawBoard());
    std::cout << "vgulaobt karoche ra" << std::endl;
}

void GosChess::GameUpdate(sf::RenderWindow &window, GosChess::GameModeListener *listener, ...) {
    va_list args;
    va_start(args, listener);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    std::cout << board->BoardStateToFen() << std::endl;
    if (GosChess::IsGameFinished()) {
        GosChess::client_listener.close();
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

void GosChess::MenuInit(sf::RenderWindow &window, ...) {
    ImGui::SFML::Init(window);
    GosChess::MenuRenderConfig();
    ImGuiIO *imgui_io = &ImGui::GetIO();
    imgui_io->FontGlobalScale = 3.f;
    GosChess::delta_clock.restart();
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

GosChess::RenderFlags::RenderMenuFLag GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::RenderMenuFLag::MAIN_MENU;

sf::Clock GosChess::delta_clock;