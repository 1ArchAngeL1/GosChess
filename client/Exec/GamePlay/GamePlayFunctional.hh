//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef GOSCHESS_GAMEPLAYFUNCTIONAL_HH
#define GOSCHESS_GAMEPLAYFUNCTIONAL_HH


#include "../../chess/global/MainOjbects//GameObjects.hh"
#include "../../chess/global/GameGlobals.hh"
#include "../../render/GamePlayRender.hh"
#include "../../chess/computation/InGameCalculations.hh"
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

    class GameModeListener {
    protected:
        sf::RenderWindow &game_window;
    public:
        explicit GameModeListener(sf::RenderWindow &game_window) : game_window(game_window) {}

        virtual void action(GosChess::board_t &) = 0;

    };

    class MultiPlayerListener : public GameModeListener {
    public:
        explicit MultiPlayerListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void action(GosChess::board_t &) override;
    };


    class GamePlayAIListener : public GameModeListener {
    public:
        explicit GamePlayAIListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void action(GosChess::board_t &) override;
    };


    class MainMenuListener : public GameModeListener {
    public:
        explicit MainMenuListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void action(GosChess::board_t &) override;
    };


    struct InitialTransferObject {

        InitialTransferObject() = default;

        InitialTransferObject(GosChess::color_t color, int time_limit) : player_color(color), time_limit(time_limit) {}

        GosChess::color_t player_color;
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
            void convert(const float &);

            short minutes;

            float seconds;
        public:
            explicit Timer(const float &);

            Timer() = default;

            bool subtract(const float &);

            void set(const float &);

            std::string toString() const;

            float getAmount() const;

        };

        struct TimerTransferObject {
            TimerTransferObject(float player, float enemy) : player_timer_amount(player), enemy_timer_amount(enemy) {}

            TimerTransferObject() = default;

            float player_timer_amount;
            float enemy_timer_amount;
        };

    }


    std::string GetInitialFenBoard();

    std::optional<GosChess::board_node_t> ChooseTrgFigure(GosChess::board_t &, sf::Window &);

    std::optional<GosChess::board_node_t> ChooseSrcFigure(GosChess::board_t &, sf::Window &);

    void CheckReceivedMove(std::optional<GosChess::action_t>, GosChess::board_t &);

    void CheckReceivedTime(GosChess::Time::Timer &, GosChess::Time::Timer &, GosChess::Time::TimerTransferObject);

    void SetGameFlagFinished();

    void ResetGame(sf::RenderWindow &);

    void Disconnected();

    void ProcessGameResult(GosChess::GameResultTransfer &);

    extern bool highlited;

    extern char opponent_ip[];

    extern game_status_t game_status_flag;

    extern GameResult game_result;

    extern GameMode game_mode;


}


#endif //PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
