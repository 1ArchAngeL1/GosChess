//
// Created by Leo Gogichaishvili on 07.02.22.
//


#include "GameLoops.h"


typedef void (*OnUserInit)(sf::RenderWindow &, ...);

typedef void (*OnUserUpdate)(sf::RenderWindow &, sf::Clock *, ...);

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
    int tmp = std::rand() % 2;
    GosChess::player_color = static_cast<GosChess::Color>(tmp);
    GosChess::enemy_color = static_cast<GosChess::Color>(!tmp);
    board->SetState(GosChess::GetInitialFenBoard());
    GosChess::GenerateOffsets();
    GosChess::BoardRenderConfig();
    GosChess::LoadChessFigureSprites();
    GosChess::CalculateAvailableMoves(board->GetRawBoard(), color_to_play);
}

void GosChess::AIGameUpdate(sf::RenderWindow &window, sf::Clock *delta_clock, ...) {
    va_list args;
    va_start(args, delta_clock);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    if (GosChess::color_to_play == GosChess::enemy_color) {
        if (GosChess::CheckMate(*board, GosChess::enemy_color)) {
            GosChess::game_status_flag = GosChess::GameStatus::FINISHED;
            GosChess::game_result = GosChess::GameResult::WON;
            return;
        }
        GosChess::Move move = GosChess::GetBestMove(*board);
        if (GosChess::MakeMove(move, *board, GosChess::color_to_play, GosChess::game_available_moves[move.move_from])) {
            GosChess::ChangeActiveColour(*board);
        }
    }
    window.clear();
    GosChess::DrawCurrentBoardState(board->GetRawBoard(), window, "00:00 ", "00:00");
    window.display();
}

void GosChess::OnlineGameInit(sf::RenderWindow &window, ...) {
    player_timer.Set(60 * time_limit_minutes);
    enemy_timer.Set(60 * time_limit_minutes);
    va_list args;
    va_start(args, window);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    board->SetState(GosChess::GetInitialFenBoard());
    GosChess::BoardRenderConfig();
    GosChess::CalculateAvailableMoves(board->GetRawBoard(), color_to_play);
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
}

static void ProcessData(GosChess::Board *board) {
    if (GosChess::game_status_flag == GosChess::GameStatus::FINISHED)return;
    std::optional<GosChess::DataTransfer<std::any>> info = GosChess::Receive();
    if (info.has_value()) {
        GosChess::DataTransfer<std::any> inside = info.value();
        switch (inside.protocol) {
            case GosChess::TransferType::MOVE:
                GosChess::CheckReceivedMove(std::any_cast<GosChess::Move>(inside.body), *board);
                break;
            case GosChess::TransferType::TIMER:
                if (GosChess::connection_role == GosChess::ConnectionType::CLIENT &&
                    GosChess::time_limit_minutes != 0) {
                    GosChess::CheckReceivedTime(player_timer, enemy_timer,
                                                std::any_cast<GosChess::Time::TimerTransferObject>(inside.body));
                    if (player_timer.GetAmount() <= 0.f) {
                        GosChess::game_status_flag = GosChess::GameStatus::FINISHED;
                        GosChess::game_result = GosChess::GameResult::LOST;
                    }
                }
                break;
            case GosChess::TransferType::RESULT:
                GosChess::game_result = std::any_cast<GosChess::GameResultTransfer>(inside.body).result;
                GosChess::SetGameFlagFinished();
                break;
        }
    }
}

void GosChess::OnlineGameUpdate(sf::RenderWindow &window, sf::Clock *delta_clock ...) {
    static float chrono = 0;
    va_list args;
    va_start(args, delta_clock);
    GosChess::Board *board = va_arg(args, GosChess::Board*);
    va_end(args);
    if (GosChess::connection_role == GosChess::ConnectionType::HOST && GosChess::time_limit_minutes != 0) {
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
    window.clear();
    GosChess::DrawCurrentBoardState(board->GetRawBoard(), window,
                                    time_limit_minutes ? player_timer.ToString() : "99:99",
                                    time_limit_minutes ? enemy_timer.ToString() : "99:99");
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
        case GosChess::RenderMenuFLag::GAME_RESULT:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderGameResultWidgets, window);
            break;
        case GosChess::RenderMenuFLag::COMPUTER:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderAiGameWidgets, window);
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

bool GosChess::CheckOnlineModeFinished() {
    bool finished = GosChess::game_status_flag == GosChess::GameStatus::FINISHED;
    if (finished) {
        GosChess::KillNetwork();
        return true;
    }
    return false;
}

bool GosChess::CheckSinglePLayerFinished() {
    return GosChess::game_status_flag == GosChess::GameStatus::FINISHED;
}


short GosChess::time_limit_minutes = 0;

GosChess::RenderMenuFLag GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
