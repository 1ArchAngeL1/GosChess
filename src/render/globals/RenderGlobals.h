//
// Created by Leo Gogichaishvili on 31.01.22.
//

#ifndef PROJECT_NAME_RENDERGLOBALS_H
#define PROJECT_NAME_RENDERGLOBALS_H

#include "../../chess/global/MainObjects/GameObjects.h"
#include "SFML/Graphics.hpp"
#include <unordered_map>

namespace GosChess {

    extern sf::Color main_color;

    extern sf::Color secondary_color;

    extern sf::Color background_color;

    extern sf::Color available_move_color;

    extern unsigned int window_height;

    extern unsigned int window_width;

    extern unsigned int board_width;

    extern unsigned int board_height;

    extern float square_size;

    extern sf::Vector2f board_position;

    extern std::unordered_map<unsigned char, sf::Sprite> figure_sprites;

}


#endif //PROJECT_NAME_RENDERGLOBALS_H
