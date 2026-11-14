//
// Created by Leo Gogichaishvili on 31.01.22.
//

#ifndef GOSCHESS_RENDERGLOBALS_HH
#define GOSCHESS_RENDERGLOBALS_HH

#include "GosChess/Board.hh"
#include "../BoardView.hh"
#include "SFML/Graphics.hpp"
#include <unordered_map>

namespace GosChess {

    // Returned by reference from a function-local static rather than declared
    // as a global: an sf::Texture constructed before main() touches SFML's
    // OpenGL context mutex while that mutex is still uninitialised.
    sf::Texture &MenuWallpaperTexture();

    extern sf::Sprite menu_wallpaper_sprite;

    sf::Texture &ChessLogoTexture();

    extern sf::Sprite chess_logo_sprite;

    extern sf::Color main_color;

    extern sf::Color secondary_color;

    extern sf::Color available_move_color;

    extern unsigned int window_height;

    extern unsigned int window_width;

    extern unsigned int board_width;

    extern unsigned int board_height;

    extern unsigned int logo_width;

    extern unsigned int logo_height;

    extern float board_offset_ver;

    extern float board_offset_hor;

    extern float square_size;

    extern sf::Vector2f board_position;

    extern std::unordered_map<unsigned char, sf::Sprite> figure_sprites;

    // Screen pixels to a cell of the drawn board. The rest of the board
    // orientation lives in BoardView.hh; only this step needs to know where
    // the board was actually placed in the window.
    view_cell_t ViewCellFromScreen(float screen_y, float screen_x);

}


#endif //PROJECT_NAME_RENDERGLOBALS_H
