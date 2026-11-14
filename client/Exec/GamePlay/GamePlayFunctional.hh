//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef GOSCHESS_GAMEPLAYFUNCTIONAL_HH
#define GOSCHESS_GAMEPLAYFUNCTIONAL_HH


#include "GosChess/Board.hh"
#include "GosChess/Rules.hh"
#include "../../Game/ClientGame.hh"
#include "../../Render/GamePlayRender.hh"
#include "../../Utility/GameInput.hh"
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <optional>

namespace GosChess {

    enum GameResult {
        WON,
        LOST,
        DRAW
    };

    enum GameMode {
        SINGLE_PLAYER,
        MULTI_PLAYER,
        None
    };

    enum game_status_t {
        INGAME,
        FINISHED
    };

    // The per-frame behaviour of one game mode. The position is not handed in:
    // there is a single client side GameState (ClientGame.hh) which the
    // listeners read, and in the online mode the server owns the original.
    class GameModeListener {
    protected:
        sf::RenderWindow &game_window;
    public:
        explicit GameModeListener(sf::RenderWindow &game_window) : game_window(game_window) {}

        virtual ~GameModeListener() = default;

        virtual void action() = 0;

    };

    class MultiPlayerListener : public GameModeListener {
    public:
        explicit MultiPlayerListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void action() override;
    };


    class GamePlayAIListener : public GameModeListener {
    public:
        explicit GamePlayAIListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void action() override;
    };


    class MainMenuListener : public GameModeListener {
    public:
        explicit MainMenuListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void action() override;
    };


    // Records the outcome of a game this client is the authority for - which
    // means the game against the computer, and only that one. Online results
    // come from the server.
    bool SettleLocalGame();

    void SetGameFlagFinished();

    void ResetGame(sf::RenderWindow &);

    extern bool highlited;

    extern game_status_t game_status_flag;

    extern GameResult game_result;

    extern GameMode game_mode;

    // Lives here rather than with the menu rendering because the thing that
    // clears it is the server announcing a game, not anything being drawn.
    extern bool menu_active_flag;

}


#endif //PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
