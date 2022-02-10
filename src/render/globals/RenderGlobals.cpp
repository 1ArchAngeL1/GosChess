//
// Created by Leo Gogichaishvili on 31.01.22.
//

#include "RenderGlobals.h"


sf::Sprite GosChess::menu_wallpaper_sprite;

sf::Texture GosChess::menu_wallpaper_txt;

sf::Color GosChess::main_color = sf::Color(128, 150, 86);

sf::Color GosChess::secondary_color = sf::Color(255, 255, 255);

sf::Color GosChess::available_move_color = sf::Color(246, 239, 110);

unsigned int GosChess::window_height = 1400;

unsigned int GosChess::window_width = 1800;

unsigned int GosChess::board_width = GosChess::window_height;

unsigned int GosChess::board_height = GosChess::window_height;

float GosChess::square_size = GosChess::board_width / GosChess::Board::ROW_NUM;

sf::Vector2f GosChess::board_position = sf::Vector2f(static_cast<float>(window_width - board_width) * 0.5f, 0.f);

std::unordered_map<unsigned char, sf::Sprite> GosChess::figure_sprites;

