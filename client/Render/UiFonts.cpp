#include "UiFonts.hh"

#include <iostream>

#include <imgui-SFML.h>

namespace {

    ImFont *menu_font = nullptr;
    ImFont *panel_font = nullptr;
    ImFont *history_font = nullptr;
    bool loaded = false;

}

void GosChess::LoadUiFonts() {
    if (loaded) return;
    ImGuiIO &io = ImGui::GetIO();

    // Scaling is left at one: each size is rasterised at the size it is drawn.
    io.FontGlobalScale = 1.f;

    const char *regular = GOSCHESS_RESOURCES "/Lato2OFL/Lato-Regular.ttf";
    const char *bold = GOSCHESS_RESOURCES "/Lato2OFL/Lato-Bold.ttf";

    io.Fonts->Clear();
    menu_font = io.Fonts->AddFontFromFileTTF(bold, 38.f);
    panel_font = io.Fonts->AddFontFromFileTTF(bold, 24.f);
    history_font = io.Fonts->AddFontFromFileTTF(regular, 20.f);

    if (menu_font == nullptr || panel_font == nullptr || history_font == nullptr) {
        // Falling back to the built-in font keeps the game usable if the
        // typeface is missing, rather than crashing on a null font.
        std::cerr << "could not load the interface fonts, falling back to the built-in one"
                  << std::endl;
        io.Fonts->Clear();
        io.Fonts->AddFontDefault();
        menu_font = panel_font = history_font = nullptr;
        io.FontGlobalScale = 2.f;
    }

    ImGui::SFML::UpdateFontTexture();
    loaded = true;
}

ImFont *GosChess::MenuFont() { return menu_font; }

ImFont *GosChess::PanelFont() { return panel_font; }

ImFont *GosChess::HistoryFont() { return history_font; }
