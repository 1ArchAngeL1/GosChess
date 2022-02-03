//
// Created by Leo Gogichaishvili on 29.01.22.
//

#include "GamePlayFunctional.h"


static constexpr std::string_view WHITE_STARTS_FEN = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr/";
static constexpr std::string_view BLACK_STARTS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/";

std::string GosChess::GetInitialFenBoard() {
    return GosChess::player_color == GosChess::Color::WHITE ? std::string(WHITE_STARTS_FEN) :
           std::string(BLACK_STARTS_FEN);
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


void GosChess::CheckReceivedMove(std::optional<GosChess::Move> move, GosChess::Board &board) {
    if (move.has_value()) {
        GosChess::MakeMoveForce(GosChess::InvertMove(move.value()), board);
        GosChess::ChangeActiveColour();
    }
}


bool GosChess::highlited = false;