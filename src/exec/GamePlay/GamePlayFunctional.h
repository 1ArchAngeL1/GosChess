//
// Created by Leo Gogichaishvili on 29.01.22.
//

#ifndef PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
#define PROJECT_NAME_GAMEPLAYFUNCTIONAL_H

#include <iostream>
#include <SFML/Graphics.hpp>
#include "../../chess/global/MainObjects/GameObjects.h"
#include "../../chess/global/GameGlobals.h"
#include "../../render/GameDraw.h"
#include "../../chess/computation/InGameCalculations.h"
#include "../../network/GameNetwork.h"
#include "../../util/GameInput.h"
#include <optional>

namespace GosChess {

    class GameModeListener {
    protected:
        sf::RenderWindow &game_window;
    public:
        explicit GameModeListener(sf::RenderWindow &game_window) : game_window(game_window) {}

        virtual void MouseClicked(GosChess::Board &board) = 0;

    };


    class MultiPlayerListener : public GameModeListener {
    public:
        explicit MultiPlayerListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void MouseClicked(GosChess::Board &board) override;
    };


    class MainMenuListener : public GameModeListener {
    public:
        explicit MainMenuListener(sf::RenderWindow &game_window) : GameModeListener(game_window) {}

        void MouseClicked(GosChess::Board &board) override;
    };


    std::string GetInitialFenBoard();

    bool Play(GosChess::Board &, const GosChess::Move &);

    std::optional<GosChess::Cell> ChooseTrgFigure(GosChess::Board &, sf::Window &);

    std::optional<GosChess::Cell> ChooseSrcFigure(GosChess::Board &, sf::Window &);

    void CheckReceivedMove(std::optional<GosChess::Move>, GosChess::Board &);

    void SetGameFlagFinished();

    bool IsGameFinished();

    extern bool highlited;

    extern char opponent_ip[];

}


#endif //PROJECT_NAME_GAMEPLAYFUNCTIONAL_H
