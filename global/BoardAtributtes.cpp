
#include<SFML/Graphics.hpp>
#include "BoardAtributtes.h"


sf::Color GosChess::main_color = sf::Color(128, 150, 86);

sf::Color GosChess::secondaryColor = sf::Color(255, 255, 255);

unsigned int GosChess::window_height = 1080;

unsigned int GosChess::window_width = 1920;

unsigned int GosChess::board_width = GosChess::window_height;

unsigned int GosChess::board_height = GosChess::window_height;

sf::Vector2f GosChess::board_position = sf::Vector2f((float) (window_width - board_width) / 2.f, 0.f);
