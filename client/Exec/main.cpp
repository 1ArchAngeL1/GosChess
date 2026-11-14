
#include "../Render/GamePlayRender.hh"
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
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height, 24), "GosChess");
    GosChess::MenuRenderConfig();
    GosChess::ChessDrawingConfig();
    while (true) {
        GosChess::MainMenuListener menu_listener(window);
        ApplyAutoFindGame();
        GosChess::GameLoop(window, GosChess::MenuInit, GosChess::MenuUpdate, GosChess::CheckMenuModeFinished,
                           &menu_listener);
        if (GosChess::game_mode == GosChess::GameMode::MULTI_PLAYER) {
            GosChess::MultiPlayerListener game_listener(window);
            GosChess::GameLoop(window, GosChess::OnlineGameInit, GosChess::OnlineGameUpdate,
                               GosChess::CheckOnlineModeFinished,
                               &game_listener);
        } else if (GosChess::game_mode == GosChess::GameMode::SINGLE_PLAYER) {
            GosChess::GamePlayAIListener game_listener(window);
            GosChess::GameLoop(window, GosChess::AIGameInit, GosChess::AIGameUpdate,
                               GosChess::CheckSinglePLayerFinished,
                               &game_listener);
        }

        GosChess::ResetGame(window);
        window.clear();
    }

}
