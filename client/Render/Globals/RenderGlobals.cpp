//
// Created by Leo Gogichaishvili on 31.01.22.
//

#include "RenderGlobals.hh"

#include <cmath>


sf::Texture &GosChess::MenuWallpaperTexture() {
    static sf::Texture menu_wallpaper_txt;
    return menu_wallpaper_txt;
}

sf::Sprite GosChess::menu_wallpaper_sprite;

sf::Texture &GosChess::ChessLogoTexture() {
    static sf::Texture chess_logo_txt;
    return chess_logo_txt;
}

sf::Sprite GosChess::chess_logo_sprite;

sf::Color GosChess::main_color = sf::Color(128, 150, 86);

sf::Color GosChess::secondary_color = sf::Color(250, 246, 227);

sf::Color GosChess::available_move_color = sf::Color(246, 239, 110);

unsigned int GosChess::window_height = 1000;

unsigned int GosChess::window_width = 1400;

unsigned int GosChess::board_width = GosChess::window_height * 0.9f;

unsigned int GosChess::board_height = GosChess::window_height * 0.9f;

unsigned int GosChess::logo_width = 200;

unsigned int GosChess::logo_height = 200;

float GosChess::board_offset_ver = 0.050f * window_height;

float GosChess::board_offset_hor = (window_width - board_width) * 0.5f;

float GosChess::square_size = GosChess::board_width / GosChess::board_t::ROW_NUM;

sf::Vector2f GosChess::board_position = sf::Vector2f(static_cast<float>(window_width - board_width) * 0.5f, board_offset_ver);

std::unordered_map<unsigned char, sf::Sprite> GosChess::figure_sprites;


GosChess::view_cell_t GosChess::ViewCellFromScreen(float screen_y, float screen_x) {
    const int row = static_cast<int>(std::floor((screen_y - GosChess::board_position.y) / GosChess::square_size));
    const int column = static_cast<int>(std::floor((screen_x - GosChess::board_position.x) / GosChess::square_size));
    return {row, column};
}
