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

    // --- layout ------------------------------------------------------------
    //
    // The board is square and sits on the left; the panel beside it carries
    // the clocks, the move record and the buttons. The board used to take the
    // whole window with a strip of timer bolted to one side, which left
    // nowhere to put any of that.

    extern unsigned int window_height;

    extern unsigned int window_width;

    extern float board_size;

    extern sf::Vector2f board_position;

    extern float square_size;

    extern float panel_x;

    extern float panel_y;

    extern float panel_width;

    extern float panel_height;

    // --- colours -----------------------------------------------------------

    extern sf::Color main_color;        // dark squares

    extern sf::Color secondary_color;   // light squares

    extern sf::Color available_move_color;

    extern sf::Color last_move_color;

    extern sf::Color selected_color;

    extern sf::Color check_color;

    extern sf::Color background_color;

    extern std::unordered_map<unsigned char, sf::Sprite> figure_sprites;

    // Recomputes the derived layout values after a size change.
    void RecalculateLayout();

    // Screen pixels to a cell of the drawn board. The rest of the board
    // orientation lives in BoardView.hh; only this step needs to know where
    // the board was actually placed in the window.
    view_cell_t ViewCellFromScreen(float screen_y, float screen_x);

    // The top left corner of a cell, in pixels.
    sf::Vector2f ViewCellTopLeft(const view_cell_t &cell);

}


#endif //PROJECT_NAME_RENDERGLOBALS_H
