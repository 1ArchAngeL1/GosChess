//
// Created by Leo Gogichaishvili on 01.02.22.
//

#include <imgui_internal.h>
#include <iostream>
#include "MenuRender.hh"

static constexpr int BUFFER_SIZE = 100;

static char user_name[BUFFER_SIZE];

static constexpr char *WELCOME_TEXT = "Welcome To Chess!";
static constexpr char *SERVER_PROMPT = "Server";
static constexpr char *CONNECTING_TEXT = "Connecting ...";
static constexpr char *SEARCHING_TEXT = "Looking for an opponent ...";
static constexpr char *CHOOSE_COLOR = "Choose Board Colors";
static constexpr char *CHOOSE_TIME = "Choose Time limit";
static constexpr char *GAME_OPTIONS = "Game Options";
static constexpr char *GAME_WON = "Game Won";
static constexpr char *GAME_LOST = "Game Lost";
static constexpr char *GAME_DRAW = "Game Draw";
static constexpr char *CHOOSE_LEVEL = "Choose Difficulty";

static constexpr char *TIME_CHOOSE[] = {" ", "1", "3", "10", "30"};
static const char *current_choice = TIME_CHOOSE[0];

static ImVec4 main_clr(GosChess::main_color.r / 255.f, GosChess::main_color.g / 255.f,
                       GosChess::main_color.b / 255.f, GosChess::main_color.a / 255.f);
static ImVec4 sec_clr(GosChess::secondary_color.r / 255.f, GosChess::secondary_color.g / 255.f,
                      GosChess::secondary_color.b / 255.f, GosChess::secondary_color.a / 255.f);


static void UpdateColors() {
    GosChess::main_color.r = 255.f * main_clr.x, GosChess::main_color.g = 255.f * main_clr.y,
    GosChess::main_color.b = 255.f * main_clr.w, GosChess::main_color.a = 255.f * main_clr.z;

    GosChess::secondary_color.r = 255.f * sec_clr.x, GosChess::secondary_color.g = 255.f * sec_clr.y,
    GosChess::secondary_color.b = 255.f * sec_clr.w, GosChess::secondary_color.a = 255.f * sec_clr.z;
}


void GosChess::RenderBackgroundWallpaper(sf::RenderWindow &window) {
    window.draw(GosChess::menu_wallpaper_sprite);
    window.draw(GosChess::chess_logo_sprite);
}

void GosChess::RenderMainMenuBackground(sf::RenderWindow &window, ImGuiContext *context) {
    ImGuiStyle *style = &ImGui::GetStyle();
    style->WindowRounding = 25;
    style->WindowMinSize = ImVec2(900, 900);
    style->WindowBorderSize = 0;

    style->Colors[ImGuiCol_Button] = ImColor(0.188, 0.09f, 0.203f, 0.4f);
    style->Colors[ImGuiCol_ButtonActive] = ImColor(0.188, 0.09f, 0.203f, 0.85f);
    style->Colors[ImGuiCol_ButtonHovered] = ImColor(0.188, 0.09f, 0.203f, 1.f);

    style->Colors[ImGuiCol_FrameBg] = ImColor(0.188, 0.09f, 0.203f, 0.4);
    style->Colors[ImGuiCol_FrameBgActive] = ImColor(0.188, 0.09f, 0.203f, 0.85);
    style->Colors[ImGuiCol_FrameBgHovered] = ImColor(0.188, 0.09f, 0.203f, 1.f);

    style->Colors[ImGuiCol_PopupBg] = ImColor(0.188, 0.09f, 0.203f, 0.4);
    style->Colors[ImGuiCol_PopupBg] = ImColor(0.188, 0.09f, 0.203f, 0.85);
    style->Colors[ImGuiCol_PopupBg] = ImColor(0.188, 0.09f, 0.203f, 1.f);

    style->Colors[ImGuiCol_SeparatorHovered] = ImColor(0.188, 0.09f, 0.203f, 1.f);
    style->Colors[ImGuiCol_Separator] = ImColor(0.188, 0.09f, 0.203f, 1.f);
    style->Colors[ImGuiCol_SeparatorActive] = ImColor(0.188, 0.09f, 0.203f, 1.f);

}

void GosChess::RenderAiGameBackground(sf::RenderWindow &window, ImGuiContext *context) {

}

void GosChess::RenderAiGameWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    auto textWidth = ImGui::CalcTextSize(CHOOSE_LEVEL).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * 0.5f);
    ImGui::Text("%s", CHOOSE_LEVEL);
    ImGui::Dummy(ImVec2(0.0f, 40.0f));
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * -0.5f);

    ImGui::Indent(200);
    ImGui::PushItemWidth(500);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.f, 1.f, 1.f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(1.f, 1.f, 1.f, 0.8f));
    ImGui::SliderInt("Level", &GosChess::ai_level, 1, 4);
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    ImGui::Indent(-200);
    ImGui::Dummy(ImVec2(0.0f, 80.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Start", ImVec2(500, 100))) {
        GosChess::game_mode = GosChess::GameMode::SINGLE_PLAYER;
        // Leaving the menu used to go through the peer-to-peer "connected"
        // flag, even for a game against the computer where nothing connects.
        GosChess::menu_active_flag = false;
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
    }
    ImGui::Indent(-200);
}


void GosChess::RenderMainMenuWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    auto textWidth = ImGui::CalcTextSize(WELCOME_TEXT).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * 0.5f);
    ImGui::Text("%s", WELCOME_TEXT);
    ImGui::Dummy(ImVec2(0.0f, 40.0f));
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * -0.5f);

    ImGui::Indent(200);
    ImGui::PushItemWidth(500);
    ImGui::InputTextWithHint("Name", "enter username", user_name, BUFFER_SIZE);
    ImGui::PopItemWidth();
    ImGui::Indent(-200);

    ImGui::Dummy(ImVec2(0.0f, 80.0f));

    ImGui::Indent(200);
    if (ImGui::Button("Computer", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::COMPUTER;
    }
    ImGui::Indent(-200);

    ImGui::Indent(200);
    if (ImGui::Button("Play Online", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::ONLINE;
    }
    ImGui::Indent(-200);

    ImGui::Indent(200);
    if (ImGui::Button("Options", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::OPTION;
    }
    ImGui::Indent(-200);

    ImGui::Indent(200);
    if (ImGui::Button("Exit", ImVec2(500, 100))) {
        GosChess::LeaveServer();
        exit(1);
    }
    ImGui::Indent(-200);
}

void GosChess::RenderOptionsBackground(sf::RenderWindow &window, ImGuiContext *context) {
}

void GosChess::RenderOptionsWidgets(sf::RenderWindow &window, ImGuiContext *context) {

    ImGui::Dummy(ImVec2(0, 40));
    auto text_width = ImGui::CalcTextSize(CHOOSE_COLOR).x;
    ImGui::Indent(GosChess::main_menu_width * 0.5f - 600 * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.188, 0.09f, 0.203f, 0.4f));
    ImGui::BeginChild("color picker", ImVec2(600, 200));
    ImGui::Indent(600 * 0.5f - text_width * 0.5f);
    ImGui::Text("%s", CHOOSE_COLOR);
    ImGui::Indent(-600 * 0.5f + text_width * 0.5f);
    ImGui::Indent(50);
    ImGui::PushItemWidth(500);
    if (ImGui::ColorEdit3("c1", &main_clr.x, true))UpdateColors();
    ImGui::Indent(-50);
    ImGui::PopItemWidth();
    ImGui::Indent(50);
    ImGui::PushItemWidth(500);
    if (ImGui::ColorEdit3("c2", &sec_clr.x, true))UpdateColors();
    ImGui::Indent(-50);
    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::Indent(-GosChess::main_menu_width * 0.5f + 600 * 0.5f);
    ImGui::Dummy(ImVec2(0, 80));
    ImGui::Indent(GosChess::main_menu_width * 0.5f - 250);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
    }
    ImGui::PopStyleColor();
}

void GosChess::RenderGameResultBackground(sf::RenderWindow &, ImGuiContext *) {
}

void GosChess::RenderGameResultWidgets(sf::RenderWindow &, ImGuiContext *) {
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    const char *verdict = nullptr;
    switch (GosChess::game_result) {
        case GosChess::GameResult::WON:
            verdict = GAME_WON;
            break;
        case GosChess::GameResult::LOST:
            verdict = GAME_LOST;
            break;
        case GosChess::GameResult::DRAW:
            verdict = GAME_DRAW;
            break;
    }
    if (verdict != nullptr) {
        auto text_width = ImGui::CalcTextSize(verdict).x;
        ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
        ImGui::Text("%s", verdict);
        ImGui::Dummy(ImVec2(0.0f, 40.0f));
        ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);
    }

    ImGui::Dummy(ImVec2(0.0f, 80.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
    }
    ImGui::Indent(-200);
}


static void SearchingPage(GosChess::LinkState state) {
    const char *message = state == GosChess::LinkState::CONNECTING ? CONNECTING_TEXT : SEARCHING_TEXT;
    ImGui::Dummy(ImVec2(0.0f, static_cast<float>(GosChess::main_menu_height) * 0.1f));
    auto text_width = ImGui::CalcTextSize(message).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
    ImGui::Text("%s", message);
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);
    ImGui::Dummy(ImVec2(0.0f, static_cast<float>(GosChess::main_menu_height) * 0.2f));
    ImGui::Indent(200);
    if (ImGui::Button("Cancel", ImVec2(500, 100))) {
        GosChess::LeaveServer();
    }
    ImGui::Indent(-200);
}


void GosChess::RenderOnlineWidgets(sf::RenderWindow &, ImGuiContext *context) {
    const GosChess::LinkState state = GosChess::GetLinkState();
    if (state == GosChess::LinkState::CONNECTING || state == GosChess::LinkState::QUEUED) {
        SearchingPage(state);
        return;
    }

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    auto text_width = ImGui::CalcTextSize(SERVER_PROMPT).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
    ImGui::Text("%s", SERVER_PROMPT);
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);
    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    ImGui::Indent(200);
    ImGui::PushItemWidth(500);
    ImGui::InputTextWithHint("server", "server address", GosChess::server_address,
                             sizeof(GosChess::server_address));
    ImGui::InputInt("port", &GosChess::server_port);
    ImGui::PopItemWidth();
    ImGui::Indent(-200);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    text_width = ImGui::CalcTextSize(CHOOSE_TIME).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
    ImGui::Text("%s", CHOOSE_TIME);
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(200);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0.188, 0.09f, 0.203f, 0.4f));
    ImGui::PushItemWidth(500);
    if (ImGui::BeginCombo("##combo", current_choice)) {
        for (auto i: TIME_CHOOSE) {
            bool is_selected = (current_choice == i);
            if (ImGui::Selectable(i, is_selected))
                current_choice = i;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();
    ImGui::Indent(-200);

    if (state == GosChess::LinkState::FAILED && !GosChess::GetLinkError().empty()) {
        ImGui::Dummy(ImVec2(0.0f, 10.0f));
        ImGui::Indent(200);
        ImGui::TextWrapped("%s", GosChess::GetLinkError().c_str());
        ImGui::Indent(-200);
    }

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Find Game", ImVec2(500, 100))) {
        // The server is told the time control and hands back a clock with every
        // position, so the client never has to work one out for itself.
        const std::uint16_t minutes = current_choice != TIME_CHOOSE[0]
                                      ? static_cast<std::uint16_t>(std::stoi(current_choice)) : 0;
        GosChess::JoinServer(GosChess::server_address,
                             static_cast<unsigned short>(GosChess::server_port), minutes);
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::LeaveServer();
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
    }
    ImGui::Indent(-200);
}


void GosChess::RenderMenu(void (*background_renderer)(sf::RenderWindow &, ImGuiContext *),
                          void (*widget_renderer)(sf::RenderWindow &, ImGuiContext *),
                          sf::RenderWindow &window) {
    ImGui::SetNextWindowPos(
            ImVec2(static_cast<float>(GosChess::window_width) * 0.5f -
                   static_cast<float>(GosChess::main_menu_width) * 0.5f,
                   static_cast<float>(GosChess::window_height) * 0.5f -
                   static_cast<float>(GosChess::main_menu_height) * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.188, 0.09f, 0.203f, 0.6f));
    ImGui::Begin("Hello", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize);
    background_renderer(window, ImGui::GetCurrentContext());
    widget_renderer(window, ImGui::GetCurrentContext());
    ImGui::PopStyleColor();
    ImGui::End();
}


static void InitWallpaperAssets() {
    if (!GosChess::MenuWallpaperTexture().loadFromFile(GOSCHESS_RESOURCES "/image.png")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    GosChess::menu_wallpaper_sprite.setTexture(GosChess::MenuWallpaperTexture());
    sf::Vector2f texture_dimensions = {static_cast<float>(GosChess::MenuWallpaperTexture().getSize().y),
                                       static_cast<float>(GosChess::MenuWallpaperTexture().getSize().x)};
    GosChess::menu_wallpaper_sprite.scale(
            sf::Vector2f(static_cast<float>(GosChess::window_width) / texture_dimensions.y,
                         static_cast<float>(GosChess::window_height) / texture_dimensions.x));
    GosChess::menu_wallpaper_sprite.setPosition({0.f, 0.f});
}

static void InitLogoAssets() {
    if (!GosChess::ChessLogoTexture().loadFromFile(GOSCHESS_RESOURCES "/chess_logo.png")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    GosChess::ChessLogoTexture().setSmooth(true);
    GosChess::chess_logo_sprite.setTexture(GosChess::ChessLogoTexture());
    sf::Vector2f texture_dimensions = {static_cast<float>(GosChess::ChessLogoTexture().getSize().y),
                                       static_cast<float>(GosChess::ChessLogoTexture().getSize().x)};
    GosChess::chess_logo_sprite.scale(
            sf::Vector2f(static_cast<float>(GosChess::logo_width) / texture_dimensions.y,
                         static_cast<float>(GosChess::logo_height) / texture_dimensions.x));
    GosChess::chess_logo_sprite.setPosition({GosChess::window_width * 0.5f - GosChess::logo_width * 0.5f, 16.f});
}

void GosChess::MenuRenderConfig() {
    InitWallpaperAssets();
   // InitLogoAssets();
}


int GosChess::main_menu_width = 900;

int GosChess::main_menu_height = 900;






