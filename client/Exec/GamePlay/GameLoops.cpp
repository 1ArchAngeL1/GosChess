//
// Created by Leo Gogichaishvili on 07.02.22.
//


#include "GameLoops.hh"


typedef void (*OnUserInit)(sf::RenderWindow &);

typedef void (*OnUserUpdate)(sf::RenderWindow &, sf::Clock *);

typedef bool (*ModeTerminator)();


void GosChess::GameLoop(sf::RenderWindow &window, OnUserInit init, OnUserUpdate update, ModeTerminator stop,
                        GosChess::GameModeListener *listener) {
    init(window);
    sf::Clock delta_clock;
    delta_clock.restart();
    while (window.isOpen()) {
        if (stop()) break;
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) {
                GosChess::LeaveServer();
                exit(3);
            }
        }
        GosChess::InputHandle::Listen();

        listener->action();

        update(window, &delta_clock);
    }
}

void GosChess::AIGameInit(sf::RenderWindow &window) {
    GosChess::SetPlayerColor(GosChess::color_t::WHITE);
    GosChess::ResetClientGame();
    GosChess::BoardRenderConfig();
    GosChess::LoadChessFigureSprites();
}

void GosChess::AIGameUpdate(sf::RenderWindow &window, sf::Clock *delta_clock) {
    if (!GosChess::game.finished() && GosChess::game.colorToPlay() == GosChess::enemy_color) {
        GosChess::game.applyMove(GosChess::GetBestMove(GosChess::game.board(), GosChess::enemy_color));
        GosChess::SettleLocalGame();
    }
    window.clear();
    GosChess::DrawCurrentBoardState(GosChess::game.board().getRawBoard(), window,
                                    GosChess::PlayerClockText(), GosChess::OpponentClockText());
    window.display();
}

// Note what is *not* here: no position is set up and no colour is drawn. Both
// arrived from the server before this loop was entered, which is the point -
// the client no longer has an opening position of its own to disagree with.
void GosChess::OnlineGameInit(sf::RenderWindow &window) {
    GosChess::BoardRenderConfig();
    GosChess::LoadChessFigureSprites();
}

void GosChess::OnlineGameUpdate(sf::RenderWindow &window, sf::Clock *delta_clock) {
    const float delta_seconds = delta_clock->restart().asSeconds();
    GosChess::ProcessServerMessages();
    // Display only: the authoritative value arrived with the last position and
    // will arrive again with the next one.
    GosChess::TickClockDisplay(delta_seconds);
    window.clear();
    GosChess::DrawCurrentBoardState(GosChess::game.board().getRawBoard(), window,
                                    GosChess::PlayerClockText(), GosChess::OpponentClockText());
    window.display();
}

void GosChess::MenuInit(sf::RenderWindow &window) {
    ImGui::SFML::Init(window);
    ImGuiIO *imgui_io = &ImGui::GetIO();
    imgui_io->FontGlobalScale = 3.f;
}

void GosChess::MenuUpdate(sf::RenderWindow &window, sf::Clock *delta_clock) {
    ImGui::SFML::Update(window, delta_clock->restart());

    switch (GosChess::render_menu_flag) {
        case GosChess::RenderMenuFLag::MAIN_MENU:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderMainMenuWidgets, window);
            break;
        case GosChess::RenderMenuFLag::ONLINE:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderOnlineWidgets, window);
            break;
        case GosChess::RenderMenuFLag::OPTION:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderOptionsWidgets, window);
            break;
        case GosChess::RenderMenuFLag::GAME_RESULT:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderGameResultWidgets, window);
            break;
        case GosChess::RenderMenuFLag::COMPUTER:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderAiGameWidgets, window);
            break;
        case GosChess::RenderMenuFLag::NONE:
        default:
            GosChess::RenderMenu(GosChess::RenderMainMenuBackground, GosChess::RenderMainMenuWidgets, window);
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
    return GosChess::game_status_flag == GosChess::game_status_t::FINISHED;
}

bool GosChess::CheckSinglePLayerFinished() {
    return GosChess::game_status_flag == GosChess::game_status_t::FINISHED;
}


GosChess::RenderMenuFLag GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
