
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../global/MainGameObjects/GameObjects.h"
#include "../global/BoardAtributtes.h"
#include "../render/GameDraw.h"
#include "../computation/GamePlayFunctional.h"


int main() {
    std::string fen_string = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr/";
    GosChess::Board board(fen_string);
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "GosChess");
    //std::cout << board.BoardStateToFen() << std::endl;
    GosChess::DrawingConfig();
    GosChess::LoadChessFigureSprites();
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(GosChess::background_color);
        GosChess::DrawCurrentBoardState(board.GetRawBoard(), window);
        window.display();
    }

    return 0;
}







