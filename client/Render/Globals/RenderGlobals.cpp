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

unsigned int GosChess::window_width = 1480;

unsigned int GosChess::window_height = 980;

float GosChess::board_size = 760.f;

sf::Vector2f GosChess::board_position = sf::Vector2f(48.f, 110.f);

float GosChess::square_size = GosChess::board_size / GosChess::board_t::ROW_LENGTH;

float GosChess::panel_x = 852.f;

float GosChess::panel_y = 40.f;

float GosChess::panel_width = 586.f;

float GosChess::panel_height = 900.f;

// A muted green board rather than the usual harsh brown: the pieces are black
// and white line art, and they need a background that does not compete.
sf::Color GosChess::main_color = sf::Color(112, 142, 94);

sf::Color GosChess::secondary_color = sf::Color(234, 235, 212);

sf::Color GosChess::available_move_color = sf::Color(20, 20, 20, 60);

sf::Color GosChess::last_move_color = sf::Color(246, 234, 112, 130);

sf::Color GosChess::selected_color = sf::Color(246, 214, 86, 160);

sf::Color GosChess::check_color = sf::Color(214, 72, 60, 170);

sf::Color GosChess::background_color = sf::Color(38, 40, 44);

std::unordered_map<unsigned char, sf::Sprite> GosChess::figure_sprites;

void GosChess::RecalculateLayout() {
    // The board takes the height it can and stays square; the panel gets what
    // is left. Keeping this in one place is what lets the window be resized
    // without every drawing call growing its own opinion about the layout.
    const float vertical_margin = 110.f;
    const float horizontal_margin = 48.f;
    const float panel_minimum = 420.f;

    const float available_height = static_cast<float>(window_height) - vertical_margin - 70.f;
    const float available_width = static_cast<float>(window_width) - horizontal_margin * 2.f
                                  - panel_minimum - 44.f;
    board_size = std::floor(std::min(available_height, available_width) /
                            board_t::ROW_LENGTH) * board_t::ROW_LENGTH;
    if (board_size < board_t::ROW_LENGTH) board_size = board_t::ROW_LENGTH;

    board_position = sf::Vector2f(horizontal_margin, vertical_margin);
    square_size = board_size / board_t::ROW_LENGTH;

    panel_x = board_position.x + board_size + 44.f;
    panel_y = vertical_margin;
    panel_width = static_cast<float>(window_width) - panel_x - horizontal_margin;
    panel_height = board_size;
}

GosChess::view_cell_t GosChess::ViewCellFromScreen(float screen_y, float screen_x) {
    const int row = static_cast<int>(std::floor((screen_y - board_position.y) / square_size));
    const int column = static_cast<int>(std::floor((screen_x - board_position.x) / square_size));
    return {row, column};
}

sf::Vector2f GosChess::ViewCellTopLeft(const GosChess::view_cell_t &cell) {
    return {board_position.x + static_cast<float>(cell.x_) * square_size,
            board_position.y + static_cast<float>(cell.y_) * square_size};
}
