
#include<SFML/Graphics.hpp>
#include "BoardAtributtes.h"


sf::Color GosChess::main_color = sf::Color(128, 150, 86);

sf::Color GosChess::secondary_color = sf::Color(255, 255, 255);

sf::Color GosChess::background_color = sf::Color(220,220,220);

unsigned int GosChess::window_height = 1280;

unsigned int GosChess::window_width = 1280;

unsigned int GosChess::board_width = GosChess::window_height;

unsigned int GosChess::board_height = GosChess::window_height;

sf::Vector2f GosChess::board_position = sf::Vector2f((float) (window_width - board_width) / 2.f, 0.f);

std::unordered_map<unsigned char,sf::Sprite> GosChess::figure_sprites;

GosChess::Color GosChess::color_to_play = GosChess::Color::WHITE;