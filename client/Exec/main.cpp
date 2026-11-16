
#include "../Audio/GameAudio.hh"
#include "../Render/BoardRender.hh"
#include "../Render/GamePanel.hh"
#include "../Render/UiFonts.hh"
#include "../Render/UiTheme.hh"
#include "GamePlay/GamePlayFunctional.hh"
#include "GamePlay/GameLoops.hh"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include <SFML/Graphics/RenderWindow.hpp>


namespace {

    // Asking the server for a game normally means clicking through the menu,
    // which makes bringing two clients up against each other tedious and
    // impossible to script. These switches do exactly what the menu button
    // does, so a pair of clients can be launched into one game in two
    // commands.
    bool auto_find_game = false;

    std::uint16_t auto_minutes = 0;

    // Where to write a single frame and exit. The board is drawn, not described
    // by a layout file, so the only way to check that a change to it looks
    // right is to look at one - and reading our own framebuffer needs no screen
    // recording permission and no human at the keyboard.
    std::string screenshot_path;

    bool screenshot_as_black = false;

    bool screenshot_menu = false;

    bool screenshot_promotion = false;

    bool screenshot_resigned = false;

    bool screenshot_animating = false;

    std::string dump_sounds_dir;

    enum class Startup {
        RUN,
        EXIT_OK,
        EXIT_ERROR
    };

    void PrintUsage(const char *program) {
        std::cout << "usage: " << program << " [--server [address]] [--port n] [--minutes n]\n"
                  << "  --server [address]  ask this server for a game at once (default "
                  << GosChess::server_address << ")\n"
                  << "  --port n            server port (default " << GosChess::server_port << ")\n"
                  << "  --minutes n         time control in minutes; 0 means no clock\n"
                  << "  --screenshot path   draw one frame of a sample game to a png and exit\n"
                  << "  --as-black          draw that frame from the black side of the board\n"
                  << "  --menu              draw the menu rather than a game\n"
                  << "  --promotion         draw a game waiting on a promotion choice\n"
                  << "  --resigned          draw a game that ended in a resignation\n"
                  << "  --animating         draw a piece caught part way through its move\n"
                  << "  no arguments        start at the menu as usual\n";
    }

    Startup ParseArguments(int argc, char **argv) {
        for (int i = 1; i < argc; i++) {
            const std::string argument = argv[i];
            if (argument == "--help" || argument == "-h") {
                PrintUsage(argv[0]);
                return Startup::EXIT_OK;
            } else if (argument == "--server") {
                auto_find_game = true;
                // The address is optional so that "--server" alone means one
                // running on this machine, the common case while testing.
                if (i + 1 < argc && argv[i + 1][0] != '-') {
                    std::strncpy(GosChess::server_address, argv[++i],
                                 sizeof(GosChess::server_address) - 1);
                    GosChess::server_address[sizeof(GosChess::server_address) - 1] = '\0';
                }
            } else if (argument == "--port" && i + 1 < argc) {
                GosChess::server_port = std::atoi(argv[++i]);
            } else if (argument == "--minutes" && i + 1 < argc) {
                auto_minutes = static_cast<std::uint16_t>(std::atoi(argv[++i]));
            } else if (argument == "--screenshot" && i + 1 < argc) {
                screenshot_path = argv[++i];
            } else if (argument == "--as-black") {
                screenshot_as_black = true;
            } else if (argument == "--menu") {
                screenshot_menu = true;
            } else if (argument == "--promotion") {
                screenshot_promotion = true;
            } else if (argument == "--resigned") {
                screenshot_resigned = true;
            } else if (argument == "--animating") {
                screenshot_animating = true;
            } else if (argument == "--dump-sounds" && i + 1 < argc) {
                dump_sounds_dir = argv[++i];
            } else {
                std::cerr << "unknown argument: " << argument << "\n";
                PrintUsage(argv[0]);
                return Startup::EXIT_ERROR;
            }
        }
        return Startup::RUN;
    }

    // Runs once, on the first trip through the menu, so that returning to the
    // menu after a game does not silently queue for another one.
    void ApplyAutoFindGame() {
        if (!auto_find_game) return;
        auto_find_game = false;
        std::cout << "asking " << GosChess::server_address << ":" << GosChess::server_port
                  << " for a game" << std::endl;
        GosChess::JoinServer(GosChess::server_address,
                             static_cast<unsigned short>(GosChess::server_port), auto_minutes);
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::ONLINE;
    }

    // A short opening, chosen so that the frame has something to show: a move
    // record several rows long, a castled king, and pieces off their starting
    // squares. It includes castling so the record shows an O-O.
    void PlaySampleOpening() {
        const char *moves[] = {"e2e4", "e7e5", "g1f3", "b8c6", "f1b5", "g8f6", "e1g1", "f8e7"};
        for (const char *move: moves) {
            const int from = GosChess::SquareFromName(std::string(move).substr(0, 2));
            const int to = GosChess::SquareFromName(std::string(move).substr(2, 2));
            if (!GosChess::game.applyMove(GosChess::action_t(static_cast<std::int8_t>(from),
                                                             static_cast<std::int8_t>(to)))) {
                std::cerr << "sample opening rejected at " << move << std::endl;
                return;
            }
            GosChess::last_move_from = static_cast<std::int8_t>(from);
            GosChess::last_move_to = static_cast<std::int8_t>(to);
        }
    }

    void DrawOneFrame(sf::RenderWindow &window) {
        sf::Event event;
        while (window.pollEvent(event)) {}
        ImGui::SFML::Update(window, sf::milliseconds(16));
        window.clear(GosChess::background_color);
        if (screenshot_menu) {
            GosChess::MenuDraw(window, 0.016f);
        } else {
            const GosChess::board_overlay_t overlay = GosChess::BuildOverlay(window);
            GosChess::DrawBoard(window, GosChess::game.board().getRawBoard(), overlay);
            const std::string side = GosChess::player_color == GosChess::color_t::WHITE
                                     ? "You are White   -   move " : "You are Black   -   move ";
            const std::string against =
                    GosChess::game_mode == GosChess::GameMode::SINGLE_PLAYER
                    ? "Against the computer" : "Against another player";
            GosChess::DrawHeader(window, against,
                                 side + std::to_string(GosChess::game.fullmoveNumber()));
            GosChess::DrawGamePanel(GosChess::StatusLine());
        }
        ImGui::SFML::Render(window);
        window.display();
    }

    bool SaveFrame(sf::RenderWindow &window, const std::string &path) {
        sf::Texture frame;
        if (!frame.create(window.getSize().x, window.getSize().y)) {
            std::cerr << "could not allocate a texture for the screenshot" << std::endl;
            return false;
        }
        frame.update(window);
        if (!frame.copyToImage().saveToFile(path)) {
            std::cerr << "could not write " << path << std::endl;
            return false;
        }
        std::cout << "wrote " << path << std::endl;
        return true;
    }

    // Captures a real online game rather than a simulated one: connects, waits
    // for the server to pair it and for a few moves to be played, and draws
    // what the player would be looking at. This is the only check that covers
    // the whole chain at once - protocol, server, client state and board.
    int WriteOnlineScreenshot(sf::RenderWindow &window, const std::string &path,
                              std::uint16_t minutes) {
        GosChess::JoinServer(GosChess::server_address,
                             static_cast<unsigned short>(GosChess::server_port), minutes);
        GosChess::game_mode = GosChess::GameMode::MULTI_PLAYER;

        // Bounded, so a server that never answers fails the run instead of
        // hanging it.
        for (int frame = 0; frame < 1200; frame++) {
            GosChess::ProcessServerMessages();
            GosChess::TickClockDisplay(0.016f);
            DrawOneFrame(window);
            // confirmed_ply, not game.ply(): adopting a server position
            // replaces the local game wholesale, which resets its own counter.
            if (GosChess::confirmed_ply >= 3) break;
        }
        if (GosChess::GetLinkState() == GosChess::LinkState::FAILED) {
            std::cerr << "online screenshot failed: " << GosChess::GetLinkError() << std::endl;
            return 1;
        }
        for (int frame = 0; frame < 3; frame++) DrawOneFrame(window);
        return SaveFrame(window, path) ? 0 : 1;
    }

    int SaveFrameAfterWarmup(sf::RenderWindow &window, const std::string &path) {
        // A few frames, because ImGui sizes some widgets from what it measured
        // on the previous one.
        for (int frame = 0; frame < 4; frame++) DrawOneFrame(window);
        return SaveFrame(window, path) ? 0 : 1;
    }

    int WriteScreenshot(sf::RenderWindow &window, const std::string &path) {
        GosChess::SetPlayerColor(screenshot_as_black ? GosChess::color_t::BLACK
                                                      : GosChess::color_t::WHITE);
        GosChess::ResetClientGame();
        GosChess::game_mode = GosChess::GameMode::SINGLE_PLAYER;
        if (screenshot_promotion) {
            // A pawn one square from the far rank, with the move already made
            // and waiting on the choice of piece.
            GosChess::game.setFen("4k3/P6P/8/8/8/8/5p2/4K3 w - - 0 40");
            GosChess::pending_promotion = GosChess::action_t(
                    static_cast<std::int8_t>(GosChess::SquareFromName("a7")),
                    static_cast<std::int8_t>(GosChess::SquareFromName("a8")));
            return SaveFrameAfterWarmup(window, path);
        }
        PlaySampleOpening();
        if (screenshot_resigned) {
            // Resignation is not something the rules know about, so this is
            // exactly the case where asking game.finished() used to be wrong.
            GosChess::game_result = GosChess::GameResult::LOST;
            GosChess::game_over_detail = "by resignation";
            GosChess::SetGameOver(true);
            return SaveFrameAfterWarmup(window, path);
        }
        if (screenshot_animating) {
            // Caught part way along, which is the only way to see that the
            // piece is drawn in flight rather than on either square.
            GosChess::StartMoveAnimation(GosChess::SquareFromName("f8"),
                                         GosChess::SquareFromName("e7"));
            GosChess::move_animation.elapsed = GosChess::move_animation.duration * 0.35f;
            return SaveFrameAfterWarmup(window, path);
        }
        // A piece picked up, so the selection and the legal move hints appear.
        GosChess::selected_square = GosChess::SquareFromName(screenshot_as_black ? "c6" : "f3");

        return SaveFrameAfterWarmup(window, path);
    }

}


int main(int argc, char **argv) {
    switch (ParseArguments(argc, argv)) {
        case Startup::EXIT_OK:
            return 0;
        case Startup::EXIT_ERROR:
            return 1;
        case Startup::RUN:
            break;
    }

    // Antialiasing, because the board is drawn as geometry now and the move
    // hints are circles: without it the rings come out visibly faceted.
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height),
                            "GosChess", sf::Style::Default, settings);
    // The alternative is a game that renders as fast as it can and heats the
    // machine up to draw a stationary chess board a thousand times a second.
    window.setVerticalSyncEnabled(true);
    GosChess::RecalculateLayout();

    // Once, here. This used to happen every time the menu was entered, which
    // leaked a fresh ImGui context per game.
    ImGui::SFML::Init(window);
    GosChess::LoadUiFonts();
    GosChess::ApplyUiTheme();
    GosChess::MenuRenderConfig();
    GosChess::LoadBoardAssets();
    GosChess::LoadSounds();
    if (!GosChess::SoundsLoaded()) {
        std::cerr << "sound is unavailable; the game will run silent" << std::endl;
    }

    if (!dump_sounds_dir.empty()) {
        const bool ok = GosChess::SaveSoundsTo(dump_sounds_dir);
        std::cout << (ok ? "wrote the sounds to " : "could not write the sounds to ")
                  << dump_sounds_dir << std::endl;
        ImGui::SFML::Shutdown();
        return ok ? 0 : 1;
    }

    if (!screenshot_path.empty()) {
        // A capture run is headless as far as the user is concerned; it should
        // not make a noise on the way past.
        GosChess::SetSoundEnabled(false);
        const int status = auto_find_game
                           ? WriteOnlineScreenshot(window, screenshot_path, auto_minutes)
                           : WriteScreenshot(window, screenshot_path);
        GosChess::LeaveServer();
        ImGui::SFML::Shutdown();
        return status;
    }

    while (window.isOpen()) {
        GosChess::MainMenuListener menu_listener(window);
        ApplyAutoFindGame();
        GosChess::GameLoop(window, GosChess::MenuInit, GosChess::MenuDraw,
                           GosChess::CheckMenuModeFinished, &menu_listener);
        if (!window.isOpen()) break;

        if (GosChess::game_mode == GosChess::GameMode::MULTI_PLAYER) {
            GosChess::MultiPlayerListener game_listener(window);
            GosChess::GameLoop(window, GosChess::OnlineGameInit, GosChess::OnlineGameDraw,
                               GosChess::CheckOnlineModeFinished, &game_listener);
        } else if (GosChess::game_mode == GosChess::GameMode::SINGLE_PLAYER) {
            GosChess::GamePlayAIListener game_listener(window);
            GosChess::GameLoop(window, GosChess::AIGameInit, GosChess::AIGameDraw,
                               GosChess::CheckSinglePLayerFinished, &game_listener);
        }

        if (!window.isOpen()) break;
        GosChess::ResetGame(window);
    }

    ImGui::SFML::Shutdown();
    return 0;
}
