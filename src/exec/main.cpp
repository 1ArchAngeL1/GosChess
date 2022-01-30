
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../chess_api/global/MainObjects/GameObjects.h"
#include "../chess_api/global/GameGlobals.h"
#include "../render/GameDraw.h"
#include "GamePlay/GamePlayFunctional.h"


int main() {
    std::string fen_string = GosChess::GetInitialFenBoard();
    GosChess::Board board(fen_string);
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "GosChess");
    std::cout << board.BoardStateToFen() << std::endl;
    GosChess::DrawingConfig();
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
    std::optional<GosChess::Cell> src_cell;
    std::optional<GosChess::Cell> trg_cell;
    while (window.isOpen()) {
        GosChess::InputHandle::Listen();
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
            if (!GosChess::highlited) {
                src_cell = GosChess::ChooseSrcFigure(board, window);
            } else {
                trg_cell = GosChess::ChooseTrgFigure(board, window);
            }
            if (src_cell.has_value() && trg_cell.has_value()) {
                int8_t from = static_cast<int8_t>(GosChess::GetNumFromNode(src_cell.value()));
                int8_t to = static_cast<int8_t>(GosChess::GetNumFromNode(trg_cell.value()));
                if (GosChess::Play(board, GosChess::Move(GosChess::Move(from, to)))) {
                    GosChess::ChangeActiveColour();
                }
                src_cell = std::nullopt;
                trg_cell = std::nullopt;
                GosChess::CalculateAvailableMoves(board.GetRawBoard());
                if (GosChess::CheckMate(board, GosChess::color_to_play))break;
            }
        }

        window.clear(GosChess::background_color);
        GosChess::DrawCurrentBoardState(board.GetRawBoard(), window);
        window.display();
    }

    return 0;
}







