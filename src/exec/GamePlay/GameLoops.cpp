//
// Created by Leo Gogichaishvili on 07.02.22.
//


#include "GameLoops.h"


typedef void (*OnUserInit)(sf::RenderWindow &, ...);

typedef void (*OnUserUpdate)(sf::RenderWindow &, sf::Clock *...);

typedef bool (*ModeTeminator)();

static GosChess::Time::Timer player_timer;

static GosChess::Time::Timer enemy_timer;

void GosChess::GameLoop(sf::RenderWindow &window, OnUserInit init, OnUserUpdate update, ModeTeminator stop,
                        GosChess::GameModeListener *listener, GosChess::Board *game_board) {
    init(window, game_board);
    sf::Clock delta_clock;
    delta_clock.restart();
    while (window.isOpen()) {
        if (stop()) return;
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) {
                GosChess::KillNetwork();
                window.close();
                exit(3);
            }
        }
        GosChess::InputHandle::Listen();
        listener->Action(*game_board);
        update(window, &delta_clock, game_board);
    }
}

void GosChess::GameInit(sf::RenderWindow &window, ...) {
    player_timer.Set(180.f);
    enemy_timer.Set(180.f);
    va_list args;
    va_start(args, window);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    GosChess::ChessDrawingConfig();
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
    GosChess::CalculateAvailableMoves(board->GetRawBoard());
}

void GosChess::GameUpdate(sf::RenderWindow &window, sf::Clock *delta_clock ...) {
    va_list args;
    va_start(args, window);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    if(GosChess::connection_role == GosChess::ConnectionType::HOST) {
        if (GosChess::color_to_play == GosChess::player_color) {
            player_timer.Subtract(delta_clock->restart().asSeconds());
        } else {
            enemy_timer.Subtract(delta_clock->restart().asSeconds());
        }
        GosChess::SendTime(GosChess::Time::TimerTransferObject(enemy_timer.GetAmount(),player_timer.GetAmount()));
    } else {
        std::optional<GosChess::Time::TimerTransferObject> res = GosChess::ReceiveTime();
        if(res.has_value()) {
            player_timer.Set(res->player_timer_amount);
            enemy_timer.Set(res->enemy_timer_amount);
        }
    }
    GosChess::CheckReceivedMove(GosChess::ReceiveMove(), *board);

    window.clear();
    GosChess::DrawCurrentBoardState(board->GetRawBoard(), window, player_timer.ToString(), enemy_timer.ToString());
    window.display();
}

void GosChess::MenuInit(sf::RenderWindow &window, ...) {
    ImGui::SFML::Init(window);
    GosChess::MenuRenderConfig();
    ImGuiIO *imgui_io = &ImGui::GetIO();
    imgui_io->FontGlobalScale = 3.f;
}

void GosChess::MenuUpdate(sf::RenderWindow &window, sf::Clock *delta_clock, ...) {
    ImGui::SFML::Update(window, delta_clock->restart());

    switch (GosChess::render_menu_flag) {
        case GosChess::RenderMenuFLag::MAIN_MENU:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderMainMenuWidgets, window);
            break;
        case GosChess::RenderMenuFLag::JOINING:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderJoinGameWidgets, window);
            break;
        case GosChess::RenderMenuFLag::HOSTING:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderHostGameWidgets, window);
            break;
        case GosChess::RenderMenuFLag::OPTION:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderOptionsWidgets, window);
            break;
        case GosChess::RenderMenuFLag::NONE:
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

GosChess::RenderMenuFLag GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
