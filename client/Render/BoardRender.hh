#ifndef GOSCHESS_BOARDRENDER_HH
#define GOSCHESS_BOARDRENDER_HH

#include <optional>
#include <string>

#include <SFML/Graphics.hpp>

#include "GosChess/Board.hh"
#include "GosChess/Move.hh"
#include "Globals/RenderGlobals.hh"

// Drawing the board.
//
// The squares used to be an eight by eight image blown up by a factor of a
// hundred, which is why the board had no crisp edges and why a highlighted
// square could only ever be a flat block of colour. They are geometry now, so
// highlights can be translucent overlays, legal moves can be the dots and
// rings people expect from a chess board, and the files and ranks can be
// labelled.

namespace GosChess {

    // Loads the fonts and piece textures. Safe to call more than once.
    void LoadBoardAssets();

    // What to draw on top of the squares this frame.
    struct board_overlay_t {
        // The square the player has picked up, if any.
        int selected_square = -1;
        // Where the piece on the selected square may legally go.
        actions_set_t legal_moves;
        // The move that produced this position.
        int last_move_from = -1;
        int last_move_to = -1;
        // The king to mark as being in check.
        int checked_king_square = -1;
        // The square under the pointer, for a light hover outline.
        int hovered_square = -1;
    };

    void DrawBoard(sf::RenderWindow &window, const unsigned char *squares,
                   const board_overlay_t &overlay);

    // The window title strip: who is playing whom, and which side is which.
    void DrawHeader(sf::RenderWindow &window, const std::string &text,
                    const std::string &subtext);

    sf::Font &BoardFont();

}

#endif
