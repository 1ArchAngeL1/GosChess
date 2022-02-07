//
// Created by Leo Gogichaishvili on 12.12.21.
//


#include "GameDraw.h"

static sf::Image board_image;

static sf::Texture board_texture;

static sf::Sprite board_sprite;

static sf::Texture root_texture;

static void ColoriseBoardSeed(sf::Color first_color, sf::Color second_color) {
    if (GosChess::player_color == GosChess::Color::WHITE);
    for (int i = 0; i < GosChess::Board::ROW_LENGTH; i++)
        for (int j = 0; j < GosChess::Board::ROW_NUM; j++)
            (i + j) % 2 == 0 ? board_image.setPixel(i, j,
                                                    GosChess::player_color == GosChess::Color::WHITE ? second_color
                                                                                                     : first_color) :
            board_image.setPixel(i, j, GosChess::player_color == GosChess::Color::WHITE ? first_color : second_color);
}


static void LoadFigureTexturesByColor(sf::Texture &root_texture, int8_t color) {
    const float figure_texture_width = root_texture.getSize().x / 6.f;
    const float figure_texture_height = root_texture.getSize().y / 2.f;
    unsigned char *figure_chars;
    switch (color) {
        case 0:
            figure_chars = new unsigned char[]{'K', 'Q', 'B', 'N', 'R', 'P'};
            break;
        case 1:
            figure_chars = new unsigned char[]{'k', 'q', 'b', 'n', 'r', 'p'};
            break;
    }
    for (int i = 0; i < 6; i++) {
        sf::Sprite curr_figure(root_texture, sf::IntRect(i * figure_texture_width,
                                                         color * figure_texture_height,
                                                         figure_texture_width,
                                                         figure_texture_height));
        curr_figure.setScale(
                sf::Vector2f((float) GosChess::board_width / (float) GosChess::Board::ROW_LENGTH / figure_texture_width,
                             (float) GosChess::board_height / (float) GosChess::Board::ROW_NUM /
                             figure_texture_height));

        GosChess::figure_sprites[figure_chars[i]] = curr_figure;
    }
    if (figure_chars != nullptr)delete[] figure_chars;
}


void GosChess::ChessDrawingConfig() {
    board_image.create(GosChess::Board::ROW_LENGTH, GosChess::Board::ROW_NUM);
    ColoriseBoardSeed(GosChess::main_color, GosChess::secondary_color);
    board_texture.loadFromImage(board_image);
    board_sprite.setTexture(board_texture);
    board_sprite.setScale((float) GosChess::board_width / GosChess::Board::ROW_LENGTH,
                          (float) GosChess::board_height / GosChess::Board::ROW_NUM);
    board_sprite.setPosition(GosChess::board_position);
}


void GosChess::ColoriseAvailableMoves(const int &index) {
    board_image.create(GosChess::Board::ROW_LENGTH, GosChess::Board::ROW_NUM);
    ColoriseBoardSeed(GosChess::main_color, GosChess::secondary_color);
    for (auto &move: GosChess::available_moves[index]) {
        GosChess::Cell move_to = GosChess::GetNode(move.move_to);
        if (GosChess::CanMakeMove(move))
            board_image.setPixel(move_to.x, GosChess::Board::ROW_NUM - move_to.y - 1,
                                 GosChess::available_move_color);

    }
    board_texture.loadFromImage(board_image);
}

void GosChess::DrawBoard(sf::RenderWindow &game_window) {
    game_window.draw(board_sprite);
}

void GosChess::LoadChessFigureSprites() {
    if (!root_texture.loadFromFile("../resources/chessPieces.png")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    LoadFigureTexturesByColor(root_texture, GosChess::Color::WHITE);
    LoadFigureTexturesByColor(root_texture, GosChess::Color::BLACK);
}

void GosChess::DrawFigure(unsigned char figure_type, sf::Vector2f figure_pos, sf::RenderWindow &game_window) {
    sf::Sprite figure_sprite = GosChess::figure_sprites.at(Board::FIG_TO_FEN.at(figure_type));
    figure_sprite.setPosition(sf::Vector2f(
            (figure_pos.x * GosChess::board_width / (float) GosChess::Board::ROW_LENGTH) + GosChess::board_position.x,
            (figure_pos.y * GosChess::board_height / (float) GosChess::Board::ROW_NUM) + GosChess::board_position.y));
    game_window.draw(figure_sprite);
}

void GosChess::DrawCurrentBoardState(const unsigned char *board, sf::RenderWindow &game_window) {
    GosChess::DrawBoard(game_window);
    for (int i = 0; i < Board::BOARD_SIZE; i++) {
        int curr_x = i % Board::ROW_LENGTH;
        int curr_y = GosChess::Board::ROW_NUM - i / Board::ROW_NUM - 1;
        if (board[i] == 0)continue;
        GosChess::DrawFigure(board[i], sf::Vector2f(curr_x, curr_y), game_window);
    }
}

void GosChess::ResetBoardColours() {
    ColoriseBoardSeed(GosChess::main_color, GosChess::secondary_color);
    board_texture.loadFromImage(board_image);
}







