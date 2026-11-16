#include "UiTheme.hh"

#include <imgui.h>

#include "Globals/RenderGlobals.hh"

namespace {

    ImVec4 Rgb(int red, int green, int blue, float alpha = 1.f) {
        return {red / 255.f, green / 255.f, blue / 255.f, alpha};
    }

}

void GosChess::ApplyUiTheme() {
    ImGuiStyle &style = ImGui::GetStyle();

    // Rounded, generously padded and without the heavy borders: the panel is
    // meant to read as one surface beside the board.
    style.WindowRounding = 10.f;
    style.ChildRounding = 8.f;
    style.FrameRounding = 7.f;
    style.GrabRounding = 7.f;
    style.ScrollbarRounding = 7.f;
    style.WindowBorderSize = 0.f;
    style.ChildBorderSize = 0.f;
    style.FrameBorderSize = 0.f;
    style.WindowPadding = ImVec2(18.f, 18.f);
    style.FramePadding = ImVec2(12.f, 9.f);
    style.ItemSpacing = ImVec2(10.f, 10.f);
    style.ScrollbarSize = 14.f;

    ImVec4 *colors = style.Colors;
    colors[ImGuiCol_Text] = Rgb(232, 234, 237);
    colors[ImGuiCol_TextDisabled] = Rgb(130, 136, 144);
    colors[ImGuiCol_WindowBg] = Rgb(30, 32, 37, 0.97f);
    colors[ImGuiCol_ChildBg] = Rgb(42, 45, 51);
    colors[ImGuiCol_PopupBg] = Rgb(30, 32, 37, 0.98f);

    colors[ImGuiCol_FrameBg] = Rgb(52, 56, 63);
    colors[ImGuiCol_FrameBgHovered] = Rgb(64, 69, 77);
    colors[ImGuiCol_FrameBgActive] = Rgb(72, 78, 87);

    // The buttons borrow the board's own green so the two halves of the screen
    // belong to the same thing.
    colors[ImGuiCol_Button] = Rgb(main_color.r, main_color.g, main_color.b, 0.85f);
    colors[ImGuiCol_ButtonHovered] = Rgb(132, 164, 112);
    colors[ImGuiCol_ButtonActive] = Rgb(96, 124, 80);

    colors[ImGuiCol_Header] = Rgb(main_color.r, main_color.g, main_color.b, 0.55f);
    colors[ImGuiCol_HeaderHovered] = Rgb(main_color.r, main_color.g, main_color.b, 0.75f);
    colors[ImGuiCol_HeaderActive] = Rgb(main_color.r, main_color.g, main_color.b, 0.95f);

    colors[ImGuiCol_SliderGrab] = Rgb(188, 206, 168);
    colors[ImGuiCol_SliderGrabActive] = Rgb(216, 230, 200);
    colors[ImGuiCol_CheckMark] = Rgb(216, 230, 200);

    colors[ImGuiCol_ScrollbarBg] = Rgb(34, 36, 41);
    colors[ImGuiCol_ScrollbarGrab] = Rgb(70, 76, 84);
    colors[ImGuiCol_ScrollbarGrabHovered] = Rgb(86, 93, 103);
    colors[ImGuiCol_ScrollbarGrabActive] = Rgb(104, 112, 124);

    colors[ImGuiCol_TableRowBg] = Rgb(46, 49, 56);
    colors[ImGuiCol_TableRowBgAlt] = Rgb(52, 56, 63);
    colors[ImGuiCol_TableHeaderBg] = Rgb(40, 43, 49);
    colors[ImGuiCol_Separator] = Rgb(62, 67, 75);
}
