//
// Created by Leo Gogichaishvili on 07.02.22.
//


#include "GameLoops.h"


typedef void (*OnUserInit)(sf::RenderWindow &, ...);

typedef void (*OnUserUpdate)(sf::RenderWindow &, sf::Clock *...);

typedef bool (*ModeTerminator)();

static GosChess::Time::Timer player_timer;

static GosChess::Time::Timer enemy_timer;

void GosChess::GameLoop(sf::RenderWindow &window, OnUserInit init, OnUserUpdate update, ModeTerminator stop,
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


void GosChess::AIGameInit(sf::RenderWindow &window, ...) {
    va_list args;
    va_start(args, window);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
    GosChess::CalculateAvailableMoves(board->GetRawBoard(), color_to_play);
    int tmp = std::rand() % 2;
    GosChess::player_color = static_cast<GosChess::Color>(tmp);
    GosChess::enemy_color = static_cast<GosChess::Color>(!tmp);
}

void AIGameUpdate(sf::RenderWindow &window, sf::Clock *delta_clock...) {
    va_list args;
    va_start(args, delta_clock);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    GosChess::IChessAi *ai_player = va_arg(args, GosChess::IChessAi*);
    va_end(args);
    if (GosChess::color_to_play == GosChess::enemy_color) {
        GosChess::Move computer_move = ai_player->GetBestMove(*board);
    }
}

void GosChess::OnlineGameInit(sf::RenderWindow &window, ...) {
    player_timer.Set(60 * time_limit_minutes);
    enemy_timer.Set(60 * time_limit_minutes);
    va_list args;
    va_start(args, window);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    board->SetState(GosChess::GetInitialFenBoard());
    va_end(args);
    GosChess::BoardRenderConfig();
    GosChess::CalculateAvailableMoves(board->GetRawBoard(), color_to_play);
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();

}

static void ProcessData(GosChess::Board *board) {
    std::optional<GosChess::DataTransfer<std::any>> info = GosChess::Receive();
    if (info.has_value()) {
        GosChess::DataTransfer<std::any> inside = info.value();
        if (inside.protocol == GosChess::TransferType::MOVE) {
            GosChess::CheckReceivedMove(std::any_cast<GosChess::Move>(inside.body), *board);
        } else if (GosChess::connection_role == GosChess::ConnectionType::CLIENT &&
                   inside.protocol == GosChess::TransferType::TIMER) {
            GosChess::CheckReceivedTime(player_timer, enemy_timer,
                                        std::any_cast<GosChess::Time::TimerTransferObject>(inside.body));
        }
    }
}

void GosChess::OnlineGameUpdate(sf::RenderWindow &window, sf::Clock *delta_clock ...) {
    va_list args;
    va_start(args, delta_clock);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    static float chrono = 0;
    if (GosChess::connection_role == GosChess::ConnectionType::HOST) {
        float dt = delta_clock->restart().asSeconds();
        chrono += dt;
        if (GosChess::color_to_play == GosChess::player_color) {
            player_timer.Subtract(dt);
        } else {
            enemy_timer.Subtract(dt);
        }
        if (chrono >= 0.3) {
            GosChess::SendTime(GosChess::Time::TimerTransferObject(enemy_timer.GetAmount(), player_timer.GetAmount()));
            chrono = 0;
        }
    }
    ProcessData(board);
    if (GosChess::connection.getRemoteAddress() == sf::IpAddress::None)
        GosChess::game_status_flag = GosChess::GameStatus::FINISHED;
    window.clear();
    GosChess::DrawCurrentBoardState(board->GetRawBoard(), window, player_timer.ToString(), enemy_timer.ToString());
    window.display();
}

void GosChess::MenuInit(sf::RenderWindow &window, ...) {
    ImGui::SFML::Init(window);
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

bool GosChess::CheckMenuModeFinished() {
    return !GosChess::menu_active_flag;
}

bool GosChess::CheckGameModeFinished() {
    bool finished = GosChess::game_status_flag == GosChess::GameStatus::FINISHED;
    if (finished) {
        GosChess::KillNetwork();
        return true;
    }
    return false;
}


short GosChess::time_limit_minutes = 0;

GosChess::RenderMenuFLag GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
