//
// Created by Leo Gogichaishvili on 12.12.21.
//


#include "GamePlayRender.hh"

static sf::Image board_image;

static sf::Texture board_texture;

static sf::Sprite board_sprite;

static sf::Texture root_texture;

static constexpr int TIMER_STAND_OFFSET = 18;

static constexpr int TIMER_STAND_HEIGHT = 10;

static sf::RectangleShape timer_stand;

static sf::RectangleShape game_theme;

static sf::Font timer_font;

static sf::Text player_timer_display;

static sf::Text enemy_timer_display;

static constexpr char *DISPLAY_FORMAT = "99:99";

static void ColorizeBoardSeed(sf::Color first_color, sf::Color second_color) {
    for (int i = 0; i < GosChess::board_t::ROW_LENGTH; i++)
        for (int j = 0; j < GosChess::board_t::ROW_NUM; j++)
            (i + j) % 2 == 0 ? board_image.setPixel(i, j,
                                                    GosChess::player_color == GosChess::color_t::WHITE ? second_color
                                                                                                       : first_color) :
            board_image.setPixel(i, j, GosChess::player_color == GosChess::color_t::WHITE ? first_color : second_color);
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
    board_texture.loadFromImage(board_image);
    board_sprite.setTexture(board_texture);
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
    timer_font.loadFromFile("C:/Users/User/CLionProjects/GosChess/resources/Lato2OFL/Lato-Black.ttf");
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


void GosChess::ColorizeAvailableMoves(const int &index, board_t &board) {
    board_image.create(GosChess::board_t::ROW_LENGTH, GosChess::board_t::ROW_NUM);
    ColorizeBoardSeed(GosChess::main_color, GosChess::secondary_color);
    for (auto &move: GosChess::game_available_moves[index]) {
        board_node_t move_to = GosChess::GetSquare(move.move_to);
        color_t color = board.getFigure(move.move_from).color_ == 0 ? color_t::WHITE : color_t::BLACK;
        if (GosChess::MakeMove(move, board, color, GosChess::game_available_moves[move.move_from])) {
            board_image.setPixel(move_to.x_, GosChess::board_t::ROW_NUM - move_to.y_ - 1,
                                 GosChess::available_move_color);
            GosChess::UndoMove(board);
        }

    }
    board_texture.loadFromImage(board_image);
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
    if (!root_texture.loadFromFile("C:/Users/User/CLionProjects/GosChess/resources/chessPieces.png")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    root_texture.setSmooth(true);
    root_texture.setSmooth(true);
    LoadFigureTexturesByColor(root_texture, GosChess::color_t::WHITE);
    LoadFigureTexturesByColor(root_texture, GosChess::color_t::BLACK);
}

void GosChess::DrawFigure(unsigned char figure_type, sf::Vector2f figure_pos, sf::RenderWindow &game_window) {
    sf::Sprite figure_sprite = GosChess::figure_sprites.at(FIG_TO_FEN.at(figure_type));
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
        int curr_x = i % board_t::ROW_LENGTH;
        int curr_y = GosChess::board_t::ROW_NUM - i / board_t::ROW_NUM - 1;
        if (board[i] == 0)continue;
        GosChess::DrawFigure(board[i], sf::Vector2f(curr_x, curr_y), game_window);
    }
}

void GosChess::ResetBoardColours() {
    ColorizeBoardSeed(GosChess::main_color, GosChess::secondary_color);
    board_texture.loadFromImage(board_image);
}







