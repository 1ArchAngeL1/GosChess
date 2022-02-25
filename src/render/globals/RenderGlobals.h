//
// Created by Leo Gogichaishvili on 31.01.22.
//

#ifndef GOSCHESS_RENDERGLOBALS_H
#define GOSCHESS_RENDERGLOBALS_H

#include "../../chess/global/main_objects/GameObjects.h"
#include "SFML/Graphics.hpp"
#include <unordered_map>

namespace GosChess {

    extern sf::Texture menu_wallpaper_txt;

    extern sf::Sprite menu_wallpaper_sprite;

    extern sf::Texture chess_logo_txt;

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

}


#endif //PROJECT_NAME_RENDERGLOBALS_H
