
#include <iostream>
#include <SFML/Graphics.hpp>
#include "../global/MainGameObjects/GameObjects.h"
#include "../global/GameGlobals.h"
#include "../render/GameDraw.h"
#include "../computation/GamePlayFunctional.h"
#include "../util/GameInput.h"


int main() {
    std::string fen_string = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr/";
    //std::string fen_string = "/8/8/3N4/8/8/8/8/8/";
    GosChess::Board board(fen_string);
    sf::RenderWindow window(sf::VideoMode(GosChess::window_width, GosChess::window_height), "GosChess");
    std::cout << board.BoardStateToFen() << std::endl;
    GosChess::DrawingConfig();
    GosChess::LoadChessFigureSprites();
    GosChess::GenerateOffsets();
    bool fig_selected = false;
    /* for(int i = 0;i < oe.size();i++) {
         std:: cout << (int)oe[i].move_from << " move to " << "y: " << (int)oe[i].move_to / 8 << " "
                                                                                                 "x :" << (int)oe[i].move_to % 8 << std::endl;
     }*/
    GosChess::Cell selecteed_node;
    GosChess::Cell target_node;
    while (window.isOpen()) {
        GosChess::InputHandle::Listen();
        sf::Event event;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
            if(fig_selected) {
                target_node = GosChess::GetNodeFromScreen(sf::Mouse::getPosition(window).y,sf::Mouse::getPosition(window).x);
                GosChess::Move desired_move = {(std::int8_t)GosChess::GetNumFromNode(selecteed_node),(std::int8_t)GosChess::GetNumFromNode(target_node)};
                if(GosChess::CanMakeMove(desired_move)) {
                    GosChess::MakeMove(desired_move,board);
                    GosChess::ChangeActiveColour();
                }

                GosChess::ResetBoardColours();
                fig_selected = false;
            } else {
                GosChess::CalculateAvailableMoves(board.GetRawBoard());
                selecteed_node = GosChess::GetNodeFromScreen(sf::Mouse::getPosition(window).y,sf::Mouse::getPosition(window).x);
                /*std::cout << "y : " << node.y << std::endl << " x : "
                          << node.x << std::endl;
                std::cout <<  "num : " << GosChess::GetNumFromNode(node) << std::endl;*/
                GosChess::ColoriseAvailableMoves(GosChess::GetNumFromNode(selecteed_node));
                fig_selected = true;
            }

        }





        window.clear(GosChess::background_color);
        GosChess::DrawCurrentBoardState(board.GetRawBoard(), window);
        window.display();
    }

    return 0;
}







