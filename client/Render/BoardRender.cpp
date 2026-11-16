#include "BoardRender.hh"

#include <cmath>
#include <iostream>

#include "../Game/ClientGame.hh"
#include "BoardView.hh"

namespace {

    // Textures are OpenGL resources and must not be created before main()
    // runs, so they live in function-local statics rather than globals.
    sf::Texture &PieceSheet() {
        static sf::Texture sheet;
        return sheet;
    }

    bool assets_loaded = false;

    constexpr int SHEET_COLUMNS = 6;
    constexpr int SHEET_ROWS = 2;

    // The pieces are drawn a little inside their squares; filling the square
    // edge to edge makes a crowded board look like a solid block.
    constexpr float PIECE_INSET = 0.06f;

    // Order of the piece sheet, left to right.
    constexpr unsigned char SHEET_ORDER[SHEET_COLUMNS] = {
            GosChess::figure_types_t::KING,
            GosChess::figure_types_t::QUEEN,
            GosChess::figure_types_t::BISHOP,
            GosChess::figure_types_t::KNIGHT,
            GosChess::figure_types_t::ROOK,
            GosChess::figure_types_t::PAWN
    };

    void DrawQuad(sf::RenderWindow &window, const GosChess::view_cell_t &cell, sf::Color color) {
        sf::RectangleShape square(sf::Vector2f(GosChess::square_size, GosChess::square_size));
        square.setPosition(GosChess::ViewCellTopLeft(cell));
        square.setFillColor(color);
        window.draw(square);
    }

    void DrawOutline(sf::RenderWindow &window, const GosChess::view_cell_t &cell, sf::Color color,
                     float thickness) {
        sf::RectangleShape square(sf::Vector2f(GosChess::square_size - thickness,
                                               GosChess::square_size - thickness));
        const sf::Vector2f corner = GosChess::ViewCellTopLeft(cell);
        square.setPosition(corner.x + thickness * 0.5f, corner.y + thickness * 0.5f);
        square.setFillColor(sf::Color::Transparent);
        square.setOutlineColor(color);
        square.setOutlineThickness(-thickness);
        window.draw(square);
    }

    // An empty destination gets a dot, an occupied one a ring around the piece
    // it would take. Recolouring the whole square hides what is standing on it.
    void DrawMoveHint(sf::RenderWindow &window, const GosChess::view_cell_t &cell, bool capture) {
        const sf::Vector2f corner = GosChess::ViewCellTopLeft(cell);
        const sf::Vector2f centre(corner.x + GosChess::square_size * 0.5f,
                                  corner.y + GosChess::square_size * 0.5f);
        if (!capture) {
            const float radius = GosChess::square_size * 0.16f;
            sf::CircleShape dot(radius);
            dot.setPointCount(40);
            dot.setOrigin(radius, radius);
            dot.setPosition(centre);
            dot.setFillColor(GosChess::available_move_color);
            window.draw(dot);
            return;
        }
        const float radius = GosChess::square_size * 0.44f;
        const float thickness = GosChess::square_size * 0.08f;
        sf::CircleShape ring(radius - thickness);
        ring.setPointCount(48);
        ring.setOrigin(radius - thickness, radius - thickness);
        ring.setPosition(centre);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(GosChess::available_move_color);
        ring.setOutlineThickness(thickness);
        window.draw(ring);
    }

    // Draws a piece with its top left corner inset into the given cell corner.
    void DrawPieceAt(sf::RenderWindow &window, unsigned char raw, sf::Vector2f corner) {
        const auto sprite = GosChess::figure_sprites.find(raw);
        if (sprite == GosChess::figure_sprites.end()) return;
        sf::Sprite piece = sprite->second;
        const float source_width = static_cast<float>(piece.getTextureRect().width);
        const float source_height = static_cast<float>(piece.getTextureRect().height);
        const float target = GosChess::square_size * (1.f - PIECE_INSET * 2.f);
        piece.setScale(target / source_width, target / source_height);
        piece.setPosition(corner.x + GosChess::square_size * PIECE_INSET,
                          corner.y + GosChess::square_size * PIECE_INSET);
        window.draw(piece);
    }

    sf::Vector2f Lerp(sf::Vector2f from, sf::Vector2f to, float t) {
        return {from.x + (to.x - from.x) * t, from.y + (to.y - from.y) * t};
    }

    void DrawCoordinates(sf::RenderWindow &window) {
        const float character_size = std::max(11.f, GosChess::square_size * 0.20f);
        sf::Text label;
        label.setFont(GosChess::BoardFont());
        label.setCharacterSize(static_cast<unsigned int>(character_size));
        label.setStyle(sf::Text::Bold);

        for (int i = 0; i < GosChess::board_t::ROW_LENGTH; i++) {
            // The labels follow the board round when it is flipped, so they
            // always read correctly from where the player is sitting.
            const int file = GosChess::BoardIsFlipped() ? GosChess::board_t::ROW_LENGTH - i - 1 : i;
            const int rank = GosChess::BoardIsFlipped() ? i : GosChess::board_t::ROW_NUM - i - 1;

            // Files along the bottom, tinted to contrast with their square.
            const bool bottom_is_light = ((GosChess::board_t::ROW_NUM - 1) + i) % 2 == 0;
            label.setFillColor(bottom_is_light ? GosChess::main_color : GosChess::secondary_color);
            label.setString(std::string(1, static_cast<char>('a' + file)));
            label.setPosition(
                    GosChess::board_position.x + static_cast<float>(i) * GosChess::square_size +
                    GosChess::square_size - label.getLocalBounds().width - character_size * 0.35f,
                    GosChess::board_position.y + GosChess::board_size -
                    character_size * 1.45f);
            window.draw(label);

            // Ranks down the left edge.
            const bool left_is_light = (i + 0) % 2 == 0;
            label.setFillColor(left_is_light ? GosChess::main_color : GosChess::secondary_color);
            label.setString(std::to_string(rank + 1));
            label.setPosition(GosChess::board_position.x + character_size * 0.3f,
                              GosChess::board_position.y +
                              static_cast<float>(i) * GosChess::square_size +
                              character_size * 0.25f);
            window.draw(label);
        }
    }

}

sf::Font &GosChess::BoardFont() {
    static sf::Font font;
    return font;
}

void GosChess::LoadBoardAssets() {
    if (assets_loaded) return;

    if (!BoardFont().loadFromFile(GOSCHESS_RESOURCES "/Lato2OFL/Lato-Bold.ttf")) {
        // Not fatal: the board is still playable without its labels.
        std::cerr << "could not load the board font" << std::endl;
    }

    if (!PieceSheet().loadFromFile(GOSCHESS_RESOURCES "/chessPieces.png")) {
        std::cerr << "could not load the piece sheet" << std::endl;
        return;
    }
    // The sheet is 200 pixels per piece and the squares are smaller than that,
    // so this is a downscale - which only looks like it should as long as the
    // texture is filtered and the sprite is not fighting a scaled board image.
    PieceSheet().setSmooth(true);

    const float piece_width = static_cast<float>(PieceSheet().getSize().x) / SHEET_COLUMNS;
    const float piece_height = static_cast<float>(PieceSheet().getSize().y) / SHEET_ROWS;

    figure_sprites.clear();
    for (int row = 0; row < SHEET_ROWS; row++) {
        const color_t color = row == 0 ? color_t::WHITE : color_t::BLACK;
        for (int column = 0; column < SHEET_COLUMNS; column++) {
            figure_t figure;
            figure.color_ = color;
            figure.type_ = SHEET_ORDER[column];
            sf::Sprite sprite(PieceSheet(), sf::IntRect(
                    static_cast<int>(column * piece_width), static_cast<int>(row * piece_height),
                    static_cast<int>(piece_width), static_cast<int>(piece_height)));
            figure_sprites[figure.full_type] = sprite;
        }
    }
    assets_loaded = true;
}

void GosChess::DrawBoard(sf::RenderWindow &window, const unsigned char *squares,
                         const GosChess::board_overlay_t &overlay) {
    LoadBoardAssets();

    // A single drop shadow and frame, so the board reads as an object sitting
    // on the background rather than a rectangle of colour.
    sf::RectangleShape frame(sf::Vector2f(board_size + 16.f, board_size + 16.f));
    frame.setPosition(board_position.x - 8.f, board_position.y - 8.f);
    frame.setFillColor(sf::Color(26, 28, 31));
    window.draw(frame);

    for (int rank = 0; rank < board_t::ROW_NUM; rank++) {
        for (int file = 0; file < board_t::ROW_LENGTH; file++) {
            const view_cell_t cell(rank, file);
            DrawQuad(window, cell, (rank + file) % 2 == 0 ? secondary_color : main_color);
        }
    }

    // Highlights go under the pieces, hints and outlines over the squares.
    for (int square: {overlay.last_move_from, overlay.last_move_to}) {
        if (!IsSquare(square)) continue;
        DrawQuad(window, BoardIndexToViewCell(square), last_move_color);
    }
    if (IsSquare(overlay.selected_square)) {
        DrawQuad(window, BoardIndexToViewCell(overlay.selected_square), selected_color);
    }
    if (IsSquare(overlay.checked_king_square)) {
        DrawQuad(window, BoardIndexToViewCell(overlay.checked_king_square), check_color);
    }
    if (IsSquare(overlay.hovered_square)) {
        DrawOutline(window, BoardIndexToViewCell(overlay.hovered_square),
                    sf::Color(255, 255, 255, 70), 3.f);
    }

    DrawCoordinates(window);

    // A piece in flight is drawn at an interpolated position instead of on its
    // destination square, so the destination is skipped here and the piece is
    // drawn afterwards, on top of everything.
    const move_animation_t &animation = move_animation;
    for (int square = 0; square < board_t::BOARD_SIZE; square++) {
        if (squares[square] == 0) continue;
        if (animation.active && (square == animation.to || square == animation.rook_to)) continue;
        DrawPieceAt(window, squares[square], ViewCellTopLeft(BoardIndexToViewCell(square)));
    }

    // Drawn last so a ring sits around the piece it would capture rather than
    // behind it.
    for (const action_t &move: overlay.legal_moves) {
        if (!IsSquare(move.move_to)) continue;
        const bool capture = squares[move.move_to] != 0;
        DrawMoveHint(window, BoardIndexToViewCell(move.move_to), capture);
    }

    if (!animation.active) return;
    const float travelled = MoveAnimationProgress();
    DrawPieceAt(window, animation.piece,
                Lerp(ViewCellTopLeft(BoardIndexToViewCell(animation.from)),
                     ViewCellTopLeft(BoardIndexToViewCell(animation.to)), travelled));
    if (IsSquare(animation.rook_from) && IsSquare(animation.rook_to)) {
        DrawPieceAt(window, animation.rook_piece,
                    Lerp(ViewCellTopLeft(BoardIndexToViewCell(animation.rook_from)),
                         ViewCellTopLeft(BoardIndexToViewCell(animation.rook_to)), travelled));
    }
}

void GosChess::DrawHeader(sf::RenderWindow &window, const std::string &text,
                          const std::string &subtext) {
    sf::Text title;
    title.setFont(BoardFont());
    title.setCharacterSize(30);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(236, 238, 240));
    title.setString(text);
    title.setPosition(board_position.x, 34.f);
    window.draw(title);

    if (subtext.empty()) return;
    sf::Text detail;
    detail.setFont(BoardFont());
    detail.setCharacterSize(19);
    detail.setFillColor(sf::Color(150, 156, 164));
    detail.setString(subtext);
    detail.setPosition(board_position.x, 72.f);
    window.draw(detail);
}
