#include "GamePanel.hh"

#include <string>
#include <vector>

#include <imgui.h>
#include <imgui-SFML.h>

#include "../Audio/GameAudio.hh"
#include "../Game/ClientGame.hh"
#include "Globals/RenderGlobals.hh"
#include "UiFonts.hh"

namespace {

    // Every button in the panel makes the same small noise as the menu ones.
    bool PanelButton(const char *label, ImVec2 size) {
        const bool pressed = ImGui::Button(label, size);
        if (pressed) GosChess::PlayUiClickSound();
        return pressed;
    }

    ImVec4 ToImGui(sf::Color color) {
        return {color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f};
    }

    const ImVec4 PANEL_BACKGROUND = ImVec4(0.12f, 0.13f, 0.15f, 0.96f);
    const ImVec4 ROW_BACKGROUND = ImVec4(0.17f, 0.18f, 0.21f, 1.f);
    const ImVec4 ACTIVE_BACKGROUND = ImVec4(0.21f, 0.30f, 0.21f, 1.f);
    const ImVec4 MUTED_TEXT = ImVec4(0.60f, 0.63f, 0.67f, 1.f);

    // Resigning is one click away from the only other button on the screen,
    // and it ends the game outright. It asks first.
    bool confirming_resign = false;

    const char *ColorName(GosChess::color_t color) {
        return color == GosChess::color_t::WHITE ? "White" : "Black";
    }

    // One clock row: whose it is, the time, and whether it is running. Shading
    // the running one is what makes "whose turn is it" answerable at a glance.
    void DrawClockRow(const char *who, GosChess::color_t color, const std::string &clock,
                      bool running, int material_edge) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, running ? ACTIVE_BACKGROUND : ROW_BACKGROUND);
        ImGui::BeginChild(who, ImVec2(0, ImGui::GetFontSize() * 2.6f), false);
        ImGui::Dummy(ImVec2(0.f, 4.f));
        ImGui::Indent(12.f);
        ImGui::Text("%s", who);
        ImGui::SameLine();
        ImGui::TextColored(MUTED_TEXT, "(%s)", ColorName(color));
        if (material_edge > 0) {
            ImGui::SameLine();
            ImGui::TextColored(MUTED_TEXT, "+%d", material_edge);
        }

        // The clock is right-aligned so the digits do not shuffle sideways as
        // the minutes tick down.
        const float clock_width = ImGui::CalcTextSize(clock.c_str()).x;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - clock_width - 12.f);
        ImGui::Text("%s", clock.c_str());
        ImGui::Unindent(12.f);
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    void DrawHistory() {
        ImGui::TextColored(MUTED_TEXT, "Moves");
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ROW_BACKGROUND);
        // Leaves room for the status line and the buttons underneath.
        const float reserved = ImGui::GetFontSize() * 9.f;
        ImGui::BeginChild("history", ImVec2(0, -reserved), false,
                          ImGuiWindowFlags_HorizontalScrollbar);
        if (GosChess::HistoryFont() != nullptr) ImGui::PushFont(GosChess::HistoryFont());

        const std::vector<GosChess::history_row_t> rows = GosChess::HistoryRows();
        if (rows.empty()) {
            ImGui::Dummy(ImVec2(0.f, 6.f));
            ImGui::Indent(12.f);
            ImGui::TextColored(MUTED_TEXT, "No moves yet.");
            ImGui::Unindent(12.f);
        } else if (ImGui::BeginTable("record", 3,
                                     ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed,
                                    ImGui::GetFontSize() * 2.4f);
            ImGui::TableSetupColumn("white", ImGuiTableColumnFlags_WidthFixed,
                                    ImGui::GetFontSize() * 4.6f);
            ImGui::TableSetupColumn("black", ImGuiTableColumnFlags_WidthFixed,
                                    ImGui::GetFontSize() * 4.6f);
            for (const GosChess::history_row_t &row: rows) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextColored(MUTED_TEXT, "%d.", row.move_number);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", row.white.c_str());
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%s", row.black.c_str());
            }
            ImGui::EndTable();
        }

        // Follows the game rather than making the player drag the scrollbar.
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - ImGui::GetFontSize()) {
            ImGui::SetScrollHereY(1.f);
        }

        if (GosChess::HistoryFont() != nullptr) ImGui::PopFont();
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // Four buttons rather than a silent queen: a player who wanted a knight
    // has to be able to say so.
    std::optional<unsigned char> DrawPromotionChoice() {
        std::optional<unsigned char> choice;
        ImGui::TextColored(ToImGui(GosChess::selected_color), "Promote to");
        const struct {
            const char *label;
            unsigned char piece;
        } options[] = {{"Queen",  GosChess::figure_types_t::QUEEN},
                       {"Rook",   GosChess::figure_types_t::ROOK},
                       {"Bishop", GosChess::figure_types_t::BISHOP},
                       {"Knight", GosChess::figure_types_t::KNIGHT}};
        const float width = (ImGui::GetContentRegionAvail().x - 24.f) / 4.f;
        for (int i = 0; i < 4; i++) {
            if (i != 0) ImGui::SameLine();
            if (PanelButton(options[i].label, ImVec2(width, ImGui::GetFontSize() * 1.9f))) {
                choice = options[i].piece;
            }
        }
        return choice;
    }

}

GosChess::panel_result_t GosChess::DrawGamePanel(const std::string &status_line) {
    panel_result_t result;

    ImGui::SetNextWindowPos(ImVec2(panel_x, panel_y));
    ImGui::SetNextWindowSize(ImVec2(panel_width, panel_height));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, PANEL_BACKGROUND);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.f, 16.f));
    if (PanelFont() != nullptr) ImGui::PushFont(PanelFont());

    ImGui::Begin("panel", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoScrollbar);

    const int balance = MaterialBalanceForPlayer();

    // The opponent sits at the top and the player at the bottom, matching how
    // the board itself is laid out.
    const bool finished = game_over || game.finished();
    DrawClockRow("Opponent", enemy_color, OpponentClockText(), !PlayerClockRunning() && !finished,
                 balance < 0 ? -balance : 0);
    ImGui::Dummy(ImVec2(0.f, 10.f));
    DrawHistory();
    ImGui::Dummy(ImVec2(0.f, 10.f));
    DrawClockRow("You", player_color, PlayerClockText(), PlayerClockRunning(),
                 balance > 0 ? balance : 0);

    ImGui::Dummy(ImVec2(0.f, 10.f));
    if (pending_promotion.has_value()) {
        result.promotion = DrawPromotionChoice();
    } else {
        ImGui::TextWrapped("%s", status_line.c_str());
        ImGui::Dummy(ImVec2(0.f, 8.f));
        const float button_height = ImGui::GetFontSize() * 2.f;
        if (finished) {
            confirming_resign = false;
            if (PanelButton("Back to menu", ImVec2(-1.f, button_height))) result.leave = true;
        } else if (confirming_resign) {
            // Resigning is one click from the only other button on the screen
            // and ends the game outright, so it asks first.
            ImGui::TextColored(ToImGui(GosChess::check_color), "Resign and lose the game?");
            const float half = (ImGui::GetContentRegionAvail().x - 10.f) * 0.5f;
            if (PanelButton("Yes, resign", ImVec2(half, button_height))) {
                result.resign = true;
                confirming_resign = false;
            }
            ImGui::SameLine();
            if (PanelButton("Keep playing", ImVec2(half, button_height))) {
                confirming_resign = false;
            }
        } else {
            if (PanelButton("Resign", ImVec2(-1.f, button_height))) confirming_resign = true;
        }
    }

    ImGui::End();
    if (PanelFont() != nullptr) ImGui::PopFont();
    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor();
    return result;
}
