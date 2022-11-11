
#include <iostream>
#include "../chess/global/main_objects/GameObjects.h"
#include "../render/GamePlayRender.h"
#include "game_play/GamePlayFunctional.h"
#include "game_play/GameLoops.h"

#include <SFML/Graphics/RenderWindow.hpp>


int main() {
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height, 24), "GosChess");
    GosChess::MenuRenderConfig();
    GosChess::ChessDrawingConfig();
    while (true) {
        GosChess::MenuNetworkMode();
        GosChess::MainMenuListener menu_listener(window);
        GosChess::GameLoop(window, GosChess::MenuInit, GosChess::MenuUpdate, GosChess::CheckMenuModeFinished,
                           &menu_listener,
                           nullptr);
        if (GosChess::game_mode == GosChess::GameMode::MULTI_PLAYER) {
            GosChess::GamePlayNetworkMode();
            GosChess::MultiPlayerListener game_listener(window);
            GosChess::Board board;
            GosChess::GameLoop(window, GosChess::OnlineGameInit, GosChess::OnlineGameUpdate,
                               GosChess::CheckOnlineModeFinished,
                               &game_listener,
                               &board);
        } else if (GosChess::game_mode == GosChess::GameMode::SINGLE_PLAYER) {
            GosChess::Board board;
            GosChess::GamePlayAIListener game_listener(window);
            GosChess::GameLoop(window, GosChess::AIGameInit, GosChess::AIGameUpdate,
                               GosChess::CheckSinglePLayerFinished,
                               &game_listener,
                               &board);

        }

        GosChess::ResetGame(window);
        window.clear();
    }


}







