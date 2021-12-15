//
// Created by Leo Gogichaishvili on 12.12.21.
//

#include "GameDraw.h"

static sf::Image board_image;

static sf::Texture board_texture;

static sf::Sprite board_sprite;


static void ColoriseBoardSeed(sf::Color first_color, sf::Color second_color) {
    for (int i = 0; i < GosChess::Board::ROW_LENGTH; i++) {
        for (int j = 0; j < GosChess::Board::ROW_NUM; j++) {
            (i + j) % 2 == 0 ? board_image.setPixel(i, j, first_color) : board_image.setPixel(i, j, second_color);
        }
    }
}

void GosChess::DrawingConfig() {
    board_image.create(GosChess::Board::ROW_LENGTH, GosChess::Board::ROW_NUM);
    ColoriseBoardSeed(GosChess::main_color, GosChess::secondaryColor);
    board_texture.loadFromImage(board_image);
    board_sprite.setTexture(board_texture);
    board_sprite.setScale((float) GosChess::board_width / GosChess::Board::ROW_LENGTH,
                          (float) GosChess::board_height / GosChess::Board::ROW_NUM);
    board_sprite.setPosition(GosChess::board_position);
}

void GosChess::DrawBoard(GosChess::Board board, sf::RenderWindow &game_window) {
    game_window.draw(board_sprite);
}

void GosChess::DrawFiuge(GosChess::Figure, float x, float y, sf::RenderWindow &game_window) {

}








