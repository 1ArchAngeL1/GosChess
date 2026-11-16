//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef GOSCHESS_GAMEPLAYFUNCTIONAL_HH
#define GOSCHESS_GAMEPLAYFUNCTIONAL_HH


#include <string>

#include "GosChess/Board.hh"
#include "GosChess/Rules.hh"
#include "../../Game/ClientGame.hh"
#include "../../Render/BoardRender.hh"
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


    // Plays a move the way the current mode plays moves: predicted and sent to
    // the server online, or applied outright against the computer. One entry
    // point because a move can arrive either from a click on the board or from
    // the promotion buttons in the panel.
    void SubmitMove(action_t move);

    // Completes a promotion the player has been asked about.
    void ChoosePromotion(unsigned char piece);

    // Records the outcome of a game this client is the authority for - which
    // means the game against the computer, and only that one. Online results
    // come from the server.
    bool SettleLocalGame();

    void SetGameFlagFinished();

    void ResetGame(sf::RenderWindow &);

    // What the board should show this frame: the selection, where it may go,
    // the previous move, a king in check and the square under the pointer.
    board_overlay_t BuildOverlay(const sf::RenderWindow &window);

    // One line telling the player what is happening and what is expected of
    // them. The old build said nothing at all.
    std::string StatusLine();

    std::string ResultHeadline();

    extern GameResult game_result;

    extern GameMode game_mode;

    // Why the game ended, in words, as reported by whoever decided it.
    extern std::string game_over_detail;

    // The square the player has picked up, or -1.
    extern int selected_square;

    // Lives here rather than with the menu rendering because the thing that
    // clears it is the server announcing a game, not anything being drawn.
    extern bool menu_active_flag;

}


#endif //PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
