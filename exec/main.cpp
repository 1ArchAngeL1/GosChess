
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../computation/GameObjects.h"
#include "../global/BoardAtributtes.h"
#include "../render/GameDraw.h"

int main() {
    std::string fen_string = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr/";
    GosChess::Board board(fen_string);
    GosChess::DrawingConfig();
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "GosChess!");

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        window.clear();
        GosChess::DrawBoard(board, window);
        window.display();
    }

    return 0;
}






