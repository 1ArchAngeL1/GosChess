
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../global/MainGameObjects/GameObjects.h"
#include "../global/GameGlobals.h"
#include "../render/GameDraw.h"
#include "../computation/GamePlayFunctional.h"
#include "../GameUtil/GameInput.h"


int main() {
    /*std::string fen_string = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr/";*/
    std::string fen_string = "/NNNNNNNN/8/3R4/8/8/8/8/8/";
    GosChess::Board board(fen_string);
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "GosChess");
    std::cout << board.BoardStateToFen() << std::endl;
    GosChess::DrawingConfig();
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
    GosChess::CalculateAvailableMoves(board.GetRawBoard());
    std::vector<GosChess::Move> oe = GosChess::available_moves[19];
   /* for(int i = 0;i < oe.size();i++) {
        std:: cout << (int)oe[i].move_from << " move to " << "y: " << (int)oe[i].move_to / 8 << " "
                                                                                                "x :" << (int)oe[i].move_to % 8 << std::endl;
    }*/

    while (window.isOpen()) {
        GosChess::InputHandle::Listen();
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if(GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
            std::cout << "y : " << (int)sf::Mouse::getPosition(window).y / (int)GosChess::square_size << std::endl << " x : "
            << (int)sf::Mouse::getPosition(window).x / (int)GosChess::square_size << std::endl;
        }

        window.clear(GosChess::background_color);
        GosChess::DrawCurrentBoardState(board.GetRawBoard(), window);
        window.display();
    }

    return 0;
}







