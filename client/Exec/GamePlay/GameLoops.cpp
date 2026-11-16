//
// Created by Leo Gogichaishvili on 07.02.22.
//


#include "GameLoops.hh"

#include "../../Audio/GameAudio.hh"
#include "../../Render/GamePanel.hh"
#include "../../Render/UiFonts.hh"


namespace {

    // Set when the player asks to leave a finished game. The loop no longer
    // ends the moment the game does, so that the final position and the result
    // stay on screen until the player has read them.
    bool leave_requested = false;

    // Turns whatever the game state recorded into a noise, and clears it. Kept
    // here rather than in ClientGame so that module stays free of the audio
    // library and can be linked into tests on its own.
    void PlayPendingSound() {
        switch (GosChess::pending_sound) {
            case GosChess::SoundCue::MOVE:      GosChess::PlayMoveSound(); break;
            case GosChess::SoundCue::CAPTURE:   GosChess::PlayCaptureSound(); break;
            case GosChess::SoundCue::CHECK:     GosChess::PlayCheckSound(); break;
            case GosChess::SoundCue::GAME_OVER: GosChess::PlayGameOverSound(); break;
            case GosChess::SoundCue::NONE:      return;
        }
        GosChess::pending_sound = GosChess::SoundCue::NONE;
    }

    // The board, the header and the panel, shared by both game modes.
    void DrawGameScreen(sf::RenderWindow &window) {
        PlayPendingSound();
        const GosChess::board_overlay_t overlay = GosChess::BuildOverlay(window);
        GosChess::DrawBoard(window, GosChess::game.board().getRawBoard(), overlay);

        const std::string side = GosChess::player_color == GosChess::color_t::WHITE
                                 ? "You are White" : "You are Black";
        const std::string opponent = GosChess::game_mode == GosChess::GameMode::SINGLE_PLAYER
                                     ? "Against the computer" : "Against another player";
        GosChess::DrawHeader(window, opponent, side + "   -   move " +
                                               std::to_string(GosChess::game.fullmoveNumber()));

        const GosChess::panel_result_t panel = GosChess::DrawGamePanel(GosChess::StatusLine());
        if (panel.promotion.has_value()) GosChess::ChoosePromotion(panel.promotion.value());
        if (panel.leave) leave_requested = true;
        if (panel.resign && !GosChess::game_over) {
            if (GosChess::game_mode == GosChess::GameMode::MULTI_PLAYER) {
                // The server decides what a resignation means, as it does with
                // everything else; it will send the result back.
                GosChess::SendResign();
            } else {
                GosChess::game_result = GosChess::GameResult::LOST;
                GosChess::game_over_detail = "by resignation";
                GosChess::SetGameFlagFinished();
            }
        }
    }

}


void GosChess::GameLoop(sf::RenderWindow &window, void (*init)(sf::RenderWindow &),
                        void (*draw)(sf::RenderWindow &, float), bool (*stop)(),
                        GosChess::GameModeListener *listener) {
    leave_requested = false;
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
                window.close();
                return;
            }
            // The window is resizable, so the board and the panel are laid out
            // again rather than being left at whatever size they started at.
            if (event.type == sf::Event::Resized) {
                GosChess::window_width = event.size.width;
                GosChess::window_height = event.size.height;
                window.setView(sf::View(sf::FloatRect(0.f, 0.f,
                                                      static_cast<float>(event.size.width),
                                                      static_cast<float>(event.size.height))));
                GosChess::RecalculateLayout();
            }
        }
        GosChess::InputHandle::Listen();

        const sf::Time delta = delta_clock.restart();
        // Before the listener runs, because the listener asks ImGui whether a
        // click belongs to the panel rather than the board.
        ImGui::SFML::Update(window, delta);

        listener->action();

        window.clear(GosChess::background_color);
        draw(window, delta.asSeconds());
        ImGui::SFML::Render(window);
        window.display();
    }
}

void GosChess::AIGameInit(sf::RenderWindow &window) {
    SetPlayerColor(color_t::WHITE);
    ResetClientGame();
    game_over_detail.clear();
    LoadBoardAssets();
}

void GosChess::AIGameDraw(sf::RenderWindow &window, float delta_seconds) {
    TickMoveAnimation(delta_seconds);
    if (!game.finished() && game.colorToPlay() == enemy_color) {
        game.applyMove(GetBestMove(game.board(), enemy_color));
        last_move_from = game.lastMove().move_from;
        last_move_to = game.lastMove().move_to;
        SettleLocalGame();
    }
    DrawGameScreen(window);
}

// Note what is *not* here: no position is set up and no colour is drawn. Both
// arrived from the server before this loop was entered, which is the point -
// the client no longer has an opening position of its own to disagree with.
void GosChess::OnlineGameInit(sf::RenderWindow &window) {
    game_over_detail.clear();
    LoadBoardAssets();
}

void GosChess::OnlineGameDraw(sf::RenderWindow &window, float delta_seconds) {
    ProcessServerMessages();
    TickMoveAnimation(delta_seconds);
    // Display only: the authoritative value arrived with the last position and
    // will arrive again with the next one.
    TickClockDisplay(delta_seconds);
    DrawGameScreen(window);
}

void GosChess::MenuInit(sf::RenderWindow &window) {
    // ImGui itself is started once in main. Starting it here created a fresh
    // context every time the menu was re-entered.
}

void GosChess::MenuDraw(sf::RenderWindow &window, float delta_seconds) {
    RenderBackgroundWallpaper(window);

    if (MenuFont() != nullptr) ImGui::PushFont(MenuFont());
    switch (render_menu_flag) {
        case RenderMenuFLag::MAIN_MENU:
            RenderMenu(RenderMainMenuBackground, RenderMainMenuWidgets, window);
            break;
        case RenderMenuFLag::ONLINE:
            RenderMenu(RenderMainMenuBackground, RenderOnlineWidgets, window);
            break;
        case RenderMenuFLag::OPTION:
            RenderMenu(RenderMainMenuBackground, RenderOptionsWidgets, window);
            break;
        case RenderMenuFLag::COMPUTER:
            RenderMenu(RenderMainMenuBackground, RenderAiGameWidgets, window);
            break;
        case RenderMenuFLag::GAME_RESULT:
        case RenderMenuFLag::NONE:
        default:
            RenderMenu(RenderMainMenuBackground, RenderMainMenuWidgets, window);
            break;
    }
    if (MenuFont() != nullptr) ImGui::PopFont();
}

bool GosChess::CheckMenuModeFinished() {
    return !menu_active_flag;
}

// A finished game stays on screen until the player leaves it, so the result and
// the final position can actually be looked at.
bool GosChess::CheckOnlineModeFinished() {
    return leave_requested;
}

bool GosChess::CheckSinglePLayerFinished() {
    return leave_requested;
}


GosChess::RenderMenuFLag GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
