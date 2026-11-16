#ifndef GOSCHESS_GAMEPANEL_HH
#define GOSCHESS_GAMEPANEL_HH

#include <optional>

#include <SFML/Graphics.hpp>

// The panel beside the board: both clocks, the move record, the result, and
// the buttons. Drawn with ImGui because a scrolling list of moves and a row of
// promotion buttons are exactly what it is good at, and because hand-rolling
// them in SFML text was what kept this screen down to a single timer strip.

namespace GosChess {

    // What the player asked for this frame.
    struct panel_result_t {
        bool resign = false;
        bool leave = false;
        // Which piece a pawn should become, when a promotion is waiting.
        std::optional<unsigned char> promotion;
    };

    panel_result_t DrawGamePanel(const std::string &status_line);

}

#endif
