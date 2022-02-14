//
// Created by Leo Gogichaishvili on 01.02.22.
//

#include <imgui_internal.h>
#include "MenuRender.h"

static constexpr int buffer_size = 100;

static char user_name[buffer_size];

static const char *welcome_text = "Welcome To Chess!";
static const char *entered_ip = "Enter opponents public ip";
static const char *waiting_to_join = "Waiting someone to join ...";
static const char *choose_color = "Choose Board Colors";
static const char *toggle_button = "Toogle button";
static const char *choose_time = "Choose Time limit";
static const char *game_options = "Game Options";

static const char *time_choose[] = {"0","3", "5", "10", "30"};
static bool play_on_time = false;
static const char *current_choice = time_choose[0];

static ImVec4 main_clr(GosChess::main_color.r / 255.f, GosChess::main_color.g / 255.f,
                       GosChess::main_color.b / 255.f, GosChess::main_color.a / 255.f);
static ImVec4 sec_clr(GosChess::secondary_color.r / 255.f, GosChess::secondary_color.g / 255.f,
                      GosChess::secondary_color.b / 255.f, GosChess::secondary_color.a / 255.f);


static void ToggleButton(const char *str_id, bool *v) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;

    ImGuiContext &g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id)) {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg;
    if (ImGui::IsItemHovered())
        col_bg = ImGui::GetColorU32(
                ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(165.f / 255.f, 0.f / 255.f, 255.f / 255.f, 0.60f), t));
    else
        col_bg = ImGui::GetColorU32(
                ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(165.f / 255.f, 0.f / 255.f, 255.f / 255.f, 0.60f), t));

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f,
                               IM_COL32(255, 255, 255, 255));
}

static void UpdateColors() {
    GosChess::main_color.r = 255.f * main_clr.x, GosChess::main_color.g = 255.f * main_clr.y,
    GosChess::main_color.b = 255.f * main_clr.w, GosChess::main_color.g = 255.f * main_clr.z;

    GosChess::secondary_color.r = 255.f * sec_clr.x, GosChess::secondary_color.g = 255.f * sec_clr.y,
    GosChess::secondary_color.b = 255.f * sec_clr.w, GosChess::secondary_color.g = 255.f * sec_clr.z;
}

typedef float slider_colors;

static slider_colors R, G, B;


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

void GosChess::RenderMainMenuWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    auto textWidth = ImGui::CalcTextSize(welcome_text).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * 0.5f);
    ImGui::Text("%s", welcome_text);
    ImGui::Dummy(ImVec2(0.0f, 40.0f));
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * -0.5f);

    ImGui::Indent(200);
    ImGui::PushItemWidth(500);
    ImGui::InputTextWithHint("Name", "enter username", user_name, buffer_size);
    ImGui::PopItemWidth();
    ImGui::Indent(-200);

    ImGui::Dummy(ImVec2(0.0f, 80.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Join Game", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::JOINING;
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Host Game", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::HOSTING;
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Options", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::OPTION;
    }
    ImGui::Indent(-200);
}

void GosChess::RenderOptionsBackground(sf::RenderWindow &window, ImGuiContext *context) {

}

void GosChess::RenderOptionsWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    static bool button;
    ImGuiStyle *style = &ImGui::GetStyle();
    ImGui::Dummy(ImVec2(0, 40));
    auto text_width = ImGui::CalcTextSize(choose_color).x;
    ImGui::Indent(GosChess::main_menu_width * 0.5f - 600 * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(165.f / 255.f, 0.f / 255.f, 255.f / 255.f, 0.60f));
    ImGui::BeginChild("color picker", ImVec2(600, 200));
    ImGui::Indent(600 * 0.5f - text_width * 0.5f);
    ImGui::Text("%s", choose_color);
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

void GosChess::RenderHostGameBackground(sf::RenderWindow &window, ImGuiContext *context) {

}


static void HostingPage() {
    ImGui::Dummy(ImVec2(0.0f, static_cast<float>(GosChess::main_menu_height) * 0.1f));
    auto text_width = ImGui::CalcTextSize(waiting_to_join).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
    ImGui::Text("%s", waiting_to_join);
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);
    ImGui::Dummy(ImVec2(0.0f, static_cast<float>(GosChess::main_menu_height) * 0.2f));
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::listen_flag = false;
    }
    ImGui::Indent(-200);
}


void GosChess::RenderHostGameWidgets(sf::RenderWindow &, ImGuiContext *context) {
    if (listen_flag) {
        HostingPage();
        return;
    }
    ImGui::Dummy(ImVec2(0.0f, static_cast<float>(GosChess::main_menu_height) * 0.1f));
    auto text_width = ImGui::CalcTextSize(game_options).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
    ImGui::Text("%s", game_options);
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);
    ImGui::Dummy(ImVec2(0.0f, static_cast<float>(GosChess::main_menu_height) * 0.1f));
    text_width = ImGui::CalcTextSize(choose_time).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
    ImGui::Text("%s", choose_time);
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);
    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(200);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0.188, 0.09f, 0.203f, 0.4f));
    ImGui::PushItemWidth(100);
    ToggleButton("a", &play_on_time);
    ImGui::SameLine();
    ImGui::Text("Time");
    ImGui::PushItemWidth(500);
    if (ImGui::BeginCombo("##combo", current_choice)) {
        if (play_on_time) {
            for (int i = 0; i < IM_ARRAYSIZE(time_choose); i++) {
                bool is_selected = (current_choice == time_choose[i]);
                if (ImGui::Selectable(time_choose[i], is_selected))
                    current_choice = time_choose[i];
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleColor();
    ImGui::PopItemWidth();

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Host", ImVec2(500, 100))) {
        GosChess::time_limit_minutes = std::stoi(current_choice);
        GosChess::HostInit();
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::render_menu_flag = GosChess::RenderMenuFLag::MAIN_MENU;
        GosChess::listen_flag = false;
    }
    ImGui::Indent(-200);
}

void GosChess::RenderJoinGameBackground(sf::RenderWindow &, ImGuiContext *context) {

}

void GosChess::RenderJoinGameWidgets(sf::RenderWindow &, ImGuiContext *context) {
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    auto textWidth = ImGui::CalcTextSize(entered_ip).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * 0.5f);
    ImGui::Text("%s", entered_ip);
    ImGui::Dummy(ImVec2(0.0f, 40.0f));
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * -0.5f);

    ImGui::Indent(200);
    ImGui::PushItemWidth(500);
    ImGui::InputTextWithHint("ip", "enter ip adress", GosChess::opponent_ip, buffer_size);
    ImGui::PopItemWidth();
    ImGui::Indent(-200);

    ImGui::Dummy(ImVec2(0.0f, 80.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Join", ImVec2(500, 100))) {
        GosChess::remote_ip = sf::IpAddress(GosChess::opponent_ip);
        GosChess::JoinInit();
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
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
    if (!GosChess::menu_wallpaper_txt.loadFromFile("../resources/image.png")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    sf::RectangleShape ae;
    GosChess::menu_wallpaper_sprite.setTexture(GosChess::menu_wallpaper_txt);
    sf::Vector2f texture_dimensions = {static_cast<float>(GosChess::menu_wallpaper_txt.getSize().y),
                                       static_cast<float>(GosChess::menu_wallpaper_txt.getSize().x)};
    GosChess::menu_wallpaper_sprite.scale(
            sf::Vector2f(static_cast<float>(GosChess::window_width) / texture_dimensions.y,
                         static_cast<float>(GosChess::window_height) / texture_dimensions.x));
    GosChess::menu_wallpaper_sprite.setPosition({0.f, 0.f});
}

static void InitLogoAssets() {
    if (!GosChess::chess_logo_txt.loadFromFile("../resources/chess_logo.png")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    GosChess::chess_logo_txt.setSmooth(true);
    GosChess::chess_logo_sprite.setTexture(GosChess::chess_logo_txt);
    sf::Vector2f texture_dimensions = {static_cast<float>(GosChess::chess_logo_txt.getSize().y),
                                       static_cast<float>(GosChess::chess_logo_txt.getSize().x)};
    GosChess::chess_logo_sprite.scale(
            sf::Vector2f(static_cast<float>(GosChess::logo_width) / texture_dimensions.y,
                         static_cast<float>(GosChess::logo_height) / texture_dimensions.x));
    GosChess::chess_logo_sprite.setPosition({GosChess::window_width * 0.5f - GosChess::logo_width * 0.5f, 16.f});
}

void GosChess::MenuRenderConfig() {
    InitWallpaperAssets();
    InitLogoAssets();
}


int GosChess::main_menu_width = 900;

int GosChess::main_menu_height = 900;

bool GosChess::menu_active_flag = true;





