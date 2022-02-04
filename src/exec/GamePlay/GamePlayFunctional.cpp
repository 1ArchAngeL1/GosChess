//
// Created by Leo Gogichaishvili on 29.01.22.
//

#include "GamePlayFunctional.h"


void GosChess::MultiPlayer::MouseClicked(GosChess::Board &board) {
    static std::optional<GosChess::Cell> src_cell = std::nullopt;
    static std::optional<GosChess::Cell> trg_cell = std::nullopt;

    if (!GosChess::highlited) {
        src_cell = GosChess::ChooseSrcFigure(board, this->game_window);
    } else {
        trg_cell = GosChess::ChooseTrgFigure(board, this->game_window);
    }
    if (src_cell.has_value() && trg_cell.has_value()) {
        int8_t from = static_cast<int8_t>(GosChess::GetNumFromNode(src_cell.value()));
        int8_t to = static_cast<int8_t>(GosChess::GetNumFromNode(trg_cell.value()));
        if (GosChess::Play(board, GosChess::Move(GosChess::Move(from, to)))) {
            GosChess::ChangeActiveColour(board);
            GosChess::SendMove(GosChess::Move(from, to));
        }
        src_cell, trg_cell = std::nullopt;
        if (GosChess::CheckMate(board, GosChess::color_to_play)) GosChess::SetGameFlagFinished();

    }
}


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
        GosChess::ChangeActiveColour(board);
    }
}

void GosChess::SetGameFlagFinished() {
    GosChess::game_status_flag = GosChess::GameStatus::FINISHED;
}

bool GosChess::IsGameFinished() {
    return GosChess::game_status_flag == GosChess::GameStatus::FINISHED;
}


bool GosChess::highlited = false;