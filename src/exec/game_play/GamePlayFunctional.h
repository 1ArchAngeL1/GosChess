//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef GOSCHESS_GAMEPLAYFUNCTIONAL_H
#define GOSCHESS_GAMEPLAYFUNCTIONAL_H


#include "../../chess/global/MainObjects/GameObjects.h"
#include "../../chess/global/GameGlobals.h"
#include "../../render/GamePlayRender.h"
#include "../../chess/computation/InGameCalculations.h"
#include "../../util/GameInput.h"
#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>

namespace GosChess {

    enum GameResult {
        WON,
        LOST,
        DRAW
    };

    enum GameMode {
        SINGLE_PLAYER,
        MULTI_PLAYER
    };

    class GameModeListener {
    protected:
        sf::RenderWindow &game_window;
    public:
        explicit GameModeListener(sf::RenderWindow &game_window) : game_window(game_window) {}

        virtual void Action(GosChess::Board &) = 0;

    };

    class MultiPlayerListener : public GameModeListener {
    public:
        explicit MultiPlayerListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void Action(GosChess::Board &) override;
    };

    class GamePlayAIListener : public GameModeListener {
    public:
        explicit GamePlayAIListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void Action(GosChess::Board &) override;
    };

    class MainMenuListener : public GameModeListener {
    public:
        explicit MainMenuListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void Action(GosChess::Board &) override;
    };


    struct InitialTransferObject {

        InitialTransferObject() = default;

        InitialTransferObject(GosChess::Color color, int time_limit) : player_color(color), time_limit(time_limit) {}

        GosChess::Color player_color;
        int time_limit;
    };


    struct GameResultTransfer {
        GameResultTransfer() = default;

        GameResultTransfer(GosChess::GameResult result) : result(result) {}

        GosChess::GameResult result;
    };

    namespace Time {

        enum Time {
            ZERO
        };

        class Timer {
        private :
            void Convert(const float &);

            short minutes;

            float seconds;
        public:
            explicit Timer(const float &);

            Timer() = default;

            bool Subtract(const float &);

            void Set(const float &);

            std::string ToString() const;

            float GetAmount() const;

        };

        struct TimerTransferObject {
            TimerTransferObject(float player, float enemy) : player_timer_amount(player), enemy_timer_amount(enemy) {}

            TimerTransferObject() = default;

            float player_timer_amount;
            float enemy_timer_amount;
        };

    }


    std::string GetInitialFenBoard();

    std::optional<GosChess::Vector2i> ChooseTrgFigure(GosChess::Board &, sf::Window &);

    std::optional<GosChess::Vector2i> ChooseSrcFigure(GosChess::Board &, sf::Window &);

    void CheckReceivedMove(std::optional<GosChess::Move>, GosChess::Board &);

    void CheckReceivedTime(GosChess::Time::Timer &, GosChess::Time::Timer &, GosChess::Time::TimerTransferObject);

    void SetGameFlagFinished();

    void ResetGame(sf::RenderWindow &);

    void Disconnected();

    void ProcessGameResult(GosChess::GameResultTransfer &);

    extern bool highlited;

    extern char opponent_ip[];

    extern GameStatus game_status_flag;

    extern GameResult game_result;

    extern GameMode game_mode;


}


#endif //PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
