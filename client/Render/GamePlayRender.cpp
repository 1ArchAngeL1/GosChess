//
// Created by Leo Gogichaishvili on 12.12.21.
//


#include "GamePlayRender.hh"

#include "../Game/ClientGame.hh"

#include <iostream>

static sf::Image board_image;

// sf::Texture creates an OpenGL resource, which must not happen before
// main() runs: SFML's own context mutex is itself a global and may not be
// constructed yet. A function-local static defers it to first use.
static sf::Texture &BoardTexture() {
    static sf::Texture board_texture;
    return board_texture;
}

static sf::Sprite board_sprite;

static sf::Texture &RootTexture() {
    static sf::Texture root_texture;
    return root_texture;
}

static constexpr int TIMER_STAND_OFFSET = 18;

static constexpr int TIMER_STAND_HEIGHT = 10;

static sf::RectangleShape timer_stand;

static sf::RectangleShape game_theme;

static sf::Font timer_font;

static sf::Text player_timer_display;

static sf::Text enemy_timer_display;

static constexpr char *DISPLAY_FORMAT = "99:99";

// The light and dark squares follow the cell's own parity, which a 180 degree
// rotation preserves, so the same pattern is correct from either side of the
// board and no longer depends on which colour the player has.
static void ColorizeBoardSeed(sf::Color dark_color, sf::Color light_color) {
    for (int i = 0; i < GosChess::board_t::ROW_LENGTH; i++)
        for (int j = 0; j < GosChess::board_t::ROW_NUM; j++)
            board_image.setPixel(i, j, (i + j) % 2 == 0 ? light_color : dark_color);
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
    sf::IntRect txt_crop;
    for (int i = 0; i < 6; i++) {
        txt_crop = sf::IntRect(i * figure_texture_width, color * figure_texture_height,
                               figure_texture_width, figure_texture_height);
        sf::Sprite curr_figure(root_texture, txt_crop);
        sf::Vector2f scalar;
        scalar.x = static_cast<float>(GosChess::board_width) / static_cast<float>(GosChess::board_t::ROW_LENGTH) /
                   figure_texture_width;
        scalar.y = static_cast<float> (GosChess::board_height) /
                   static_cast<float>(GosChess::board_t::ROW_NUM) / figure_texture_height;
        curr_figure.setScale(scalar);
        GosChess::figure_sprites[figure_chars[i]] = curr_figure;
    }
    if (figure_chars != nullptr)delete[] figure_chars;
}


void GosChess::BoardRenderConfig() {
    board_image.create(GosChess::board_t::ROW_LENGTH, GosChess::board_t::ROW_NUM);
    ColorizeBoardSeed(GosChess::main_color, GosChess::secondary_color);
    BoardTexture().loadFromImage(board_image);
    board_sprite.setTexture(BoardTexture());
    board_sprite.setScale(static_cast<float>(GosChess::board_width) / GosChess::board_t::ROW_LENGTH,
                          static_cast<float>(GosChess::board_height) / GosChess::board_t::ROW_NUM);
    board_sprite.setPosition(GosChess::board_position);
}

void GosChess::ChessDrawingConfig() {
    BoardRenderConfig();
    board_sprite.setPosition(GosChess::board_position);
    game_theme.setPosition(sf::Vector2f(0, 0));
    game_theme.setFillColor(sf::Color(64, 64, 64));
    game_theme.setSize(sf::Vector2f(GosChess::window_width, GosChess::window_height));
    timer_stand.setFillColor(sf::Color::White);
    timer_stand.setSize(sf::Vector2f(GosChess::board_offset_hor - 2 * TIMER_STAND_OFFSET, TIMER_STAND_HEIGHT));
    timer_stand.setPosition(
            sf::Vector2f(GosChess::board_offset_hor + GosChess::board_width + TIMER_STAND_OFFSET,
                         GosChess::window_height * 0.5f));
    timer_font.loadFromFile(GOSCHESS_RESOURCES "/Lato2OFL/Lato-Black.ttf");
    player_timer_display.setFont(timer_font);
    enemy_timer_display.setFont(timer_font);
    player_timer_display.setStyle(sf::Text::Bold);
    enemy_timer_display.setStyle(sf::Text::Bold);
    player_timer_display.setFillColor(sf::Color::White);
    enemy_timer_display.setFillColor(sf::Color::White);
    player_timer_display.setString(DISPLAY_FORMAT);
    enemy_timer_display.setString(DISPLAY_FORMAT);
    player_timer_display.scale(sf::Vector2f(1.5f, 1.5f));
    enemy_timer_display.scale(sf::Vector2f(1.5f, 1.5f));
    float display_width = player_timer_display.getGlobalBounds().width;
    float display_height = player_timer_display.getGlobalBounds().height * 1.5f;
    player_timer_display.setPosition(
            sf::Vector2f(1.5f * GosChess::board_offset_hor + GosChess::board_width - display_width / 2,
                         GosChess::window_height * 0.5f + TIMER_STAND_HEIGHT));
    enemy_timer_display.setPosition(
            sf::Vector2f(1.5f * GosChess::board_offset_hor + GosChess::board_width - display_width / 2,
                         GosChess::window_height * 0.5f - display_height));

}


void GosChess::ColorizeAvailableMoves(int board_index) {
    board_image.create(GosChess::board_t::ROW_LENGTH, GosChess::board_t::ROW_NUM);
    ColorizeBoardSeed(GosChess::main_color, GosChess::secondary_color);
    const actions_set_t &moves = GosChess::game.movesFrom(board_index);
    // Legality is probed on a copy so that highlighting can never disturb the
    // position the game is actually being played on.
    board_t probe = GosChess::game.board();
    const color_t mover = static_cast<color_t>(probe.getFigure(board_index).color_);
    for (const auto &move: moves) {
        if (!GosChess::MakeMove(move, probe, mover, moves)) continue;
        GosChess::UndoMove(probe);
        const view_cell_t cell = GosChess::BoardIndexToViewCell(move.move_to);
        board_image.setPixel(cell.x_, cell.y_, GosChess::available_move_color);
    }
    BoardTexture().loadFromImage(board_image);
}


void GosChess::DrawBoard(sf::RenderWindow &game_window) {
    game_window.draw(game_theme);
    game_window.draw(board_sprite);
}


void GosChess::DrawTimer(sf::RenderWindow &game_window, const std::string &player_str, const std::string &enemy_str) {
    game_window.draw(timer_stand);
    player_timer_display.setString(player_str), enemy_timer_display.setString(enemy_str);
    game_window.draw(player_timer_display);
    game_window.draw(enemy_timer_display);
}

void GosChess::LoadChessFigureSprites() {
    if (!RootTexture().loadFromFile(GOSCHESS_RESOURCES "/chessPieces.png")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    RootTexture().setSmooth(true);
    RootTexture().setSmooth(true);
    LoadFigureTexturesByColor(RootTexture(), GosChess::color_t::WHITE);
    LoadFigureTexturesByColor(RootTexture(), GosChess::color_t::BLACK);
}

void GosChess::DrawFigure(unsigned char figure_type, sf::Vector2f figure_pos, sf::RenderWindow &game_window) {
    sf::Sprite figure_sprite = GosChess::figure_sprites.at(GosChess::FenCharFromFigure(figure_type));
    figure_sprite.setPosition(sf::Vector2f(
            (figure_pos.x * GosChess::board_width / static_cast<float>(GosChess::board_t::ROW_LENGTH)) +
            GosChess::board_position.x,
            (figure_pos.y * GosChess::board_height / static_cast<float>(GosChess::board_t::ROW_NUM)) +
            GosChess::board_position.y));
    game_window.draw(figure_sprite);
}


void GosChess::DrawCurrentBoardState(const unsigned char *board, sf::RenderWindow &game_window,
                                     const std::string &player_time,
                                     const std::string &enemy_player) {
    GosChess::DrawBoard(game_window);
    GosChess::DrawTimer(game_window, player_time, enemy_player);
    for (int i = 0; i < board_t::BOARD_SIZE; i++) {
        if (board[i] == 0)continue;
        const view_cell_t cell = GosChess::BoardIndexToViewCell(i);
        GosChess::DrawFigure(board[i], sf::Vector2f(cell.x_, cell.y_), game_window);
    }
}

void GosChess::ResetBoardColours() {
    ColorizeBoardSeed(GosChess::main_color, GosChess::secondary_color);
    BoardTexture().loadFromImage(board_image);
}







