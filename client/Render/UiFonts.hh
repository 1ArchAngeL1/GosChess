#ifndef GOSCHESS_UIFONTS_HH
#define GOSCHESS_UIFONTS_HH

#include <imgui.h>

// Fonts for the ImGui widgets.
//
// The interface used to be drawn in ImGui's built-in bitmap font with
// FontGlobalScale turned up to three, which is why every label looked soft:
// that setting stretches an atlas baked at thirteen pixels. Loading a real
// typeface at the sizes actually wanted costs nothing and is the single
// biggest difference to how the menus read.

namespace GosChess {

    // Builds the font atlas. Must be called after ImGui::SFML::Init and before
    // anything draws.
    void LoadUiFonts();

    // Large, for the menu pages.
    ImFont *MenuFont();

    // Smaller, for the dense panel beside the board.
    ImFont *PanelFont();

    // Smaller still, for the move record.
    ImFont *HistoryFont();

}

#endif
