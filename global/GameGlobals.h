
#ifndef PROJECT_NAME_GAMEGLOBALS_H
#define PROJECT_NAME_GAMEGLOBALS_H

#include "MainGameObjects/GameObjects.h"
#include <unordered_map>

namespace GosChess {

    extern sf::Color main_color;

    extern sf::Color secondary_color;

    extern sf::Color background_color;

    extern unsigned int window_height;

    extern unsigned int window_width;

    extern unsigned int board_width;

    extern unsigned int board_height;

    extern float square_size;

    extern sf::Vector2f board_position;

    extern std::unordered_map<unsigned char, sf::Sprite> figure_sprites;

    extern GosChess::Color color_to_play;

}

#endif
