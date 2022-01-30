//
// Created by Leo Gogichaishvili on 29.01.22.
//

#include "GamePlayFunctional.h"



std::string GosChess::GetInitialFenBoard() {
    return GosChess::player_color == GosChess::Color::WHITE ? "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr/" :
    "rnbqkbnr/PPPPPPPP/8/8/8/8/pppppppp/RNBQKBNR/";
}

bool GosChess::Play(GosChess::Board &brd, const GosChess::Move &move) {
    if (GosChess::CanMakeMove(move)) {
        return GosChess::MakeMove(move, brd);
    }
}


std::optional<GosChess::Cell> GosChess::ChooseTrgFigure(GosChess::Board &brd, sf::Window &window) {
    if (!GosChess::highlited) {
        return std::nullopt;
    }
    GosChess::Cell trg_node = GosChess::GetNodeFromScreen(sf::Mouse::getPosition(window).y,
                                                          sf::Mouse::getPosition(window).x);
    GosChess::ResetBoardColours();
    highlited = false;
    return std::optional<GosChess::Cell>(trg_node);
}

std::optional<GosChess::Cell> GosChess::ChooseSrcFigure(GosChess::Board &brd, sf::Window &window) {
    if (GosChess::highlited) {
        GosChess::ResetBoardColours();
    } else {
        GosChess::CalculateAvailableMoves(brd.GetRawBoard());
    }
    GosChess::Cell src_node = GosChess::GetNodeFromScreen(sf::Mouse::getPosition(window).y,
                                                          sf::Mouse::getPosition(window).x);
    GosChess::ColoriseAvailableMoves(GosChess::GetNumFromNode(src_node));
    highlited = true;
    return std::optional<GosChess::Cell>(src_node);
}


bool GosChess::highlited = false;