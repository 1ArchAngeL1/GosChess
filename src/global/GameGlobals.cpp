
#include<SFML/Graphics.hpp>
#include "GameGlobals.h"


sf::Color GosChess::main_color = sf::Color(128, 150, 86);

sf::Color GosChess::secondary_color = sf::Color(255, 255, 255);

sf::Color GosChess::background_color = sf::Color(220, 220, 220);

extern sf::Color GosChess::available_move_color = sf::Color(246, 239, 110);

unsigned int GosChess::window_height = 1480;

unsigned int GosChess::window_width = 1480;

unsigned int GosChess::board_width = GosChess::window_height;

unsigned int GosChess::board_height = GosChess::window_height;

float GosChess::square_size = GosChess::board_width / GosChess::Board::ROW_NUM;

sf::Vector2f GosChess::board_position = sf::Vector2f((float) (window_width - board_width) / 2.f, 0.f);

std::unordered_map<unsigned char, sf::Sprite> GosChess::figure_sprites;

GosChess::Color GosChess::color_to_play = GosChess::Color::WHITE;

GosChess::Color GosChess::enemy_color;

GosChess::Color GosChess::my_color;
