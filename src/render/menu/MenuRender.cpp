//
// Created by Leo Gogichaishvili on 01.02.22.
//

#include "MenuRender.h"

static constexpr int buffer_size = 100;

static char user_name[buffer_size];

static const char *welcome_text = "Welcome To Chess!";
static const char *entered_ip = "Enter opponents public ip";
static const char *waiting_to_join = "Waiting someone to join ...";
static const char *choose_color_prim = "choose first color";
static const char *choose_color_sec = "choose second color";

static ImVec4 main_clr(GosChess::main_color.r / 255.f, GosChess::main_color.g / 255.f,
                       GosChess::main_color.b / 255.f, GosChess::main_color.a / 255.f);
static ImVec4 sec_clr(GosChess::secondary_color.r / 255.f, GosChess::secondary_color.g / 255.f,
                      GosChess::secondary_color.b / 255.f, GosChess::secondary_color.a / 255.f);

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
}

void GosChess::RenderMainMenuWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    auto textWidth = ImGui::CalcTextSize(welcome_text).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * 0.5f);
    ImGui::Text("%s", welcome_text);
    ImGui::Dummy(ImVec2(0.0f, 40.0f));
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - textWidth) * -0.5f);

    ImGui::Indent(200);
    ImGui::InputTextMultiline("Name", user_name, 100, ImVec2(500, 60));
    ImGui::Indent(-200);

    ImGui::Dummy(ImVec2(0.0f, 80.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Join Game", ImVec2(500, 100))) {
        GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::JOINING;
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Host Game", ImVec2(500, 100))) {
        GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::HOSTING;
        GosChess::HostInit();
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Options", ImVec2(500, 100))) {
        GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::OPTION;
    }
    ImGui::Indent(-200);
}

void GosChess::RenderOptionsBackground(sf::RenderWindow &window, ImGuiContext *context) {

}

void GosChess::RenderOptionsWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    static ImVec4 color;
    ImGuiStyle *style = &ImGui::GetStyle();
    ImGui::Dummy(ImVec2(0, 40));
    auto text_width = ImGui::CalcTextSize(choose_color_prim).x;
    ImGui::Indent(GosChess::main_menu_width * 0.5f - 600 * 0.5f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(128.f / 255.f, 0.f / 255.f, 128.f / 255.f, 0.60f));
    ImGui::BeginChild("color picker", ImVec2(600, 200));
        ImGui::Indent(600 * 0.5f - text_width * 0.5f);
            ImGui::Text("%s", choose_color_prim);
        ImGui::Indent(-600 * 0.5f + text_width * 0.5f);
            ImGui::Indent(50);
            ImGui::PushItemWidth(500);
            if(ImGui::ColorEdit3("c1", &main_clr.x, true))UpdateColors();
            ImGui::Indent(-50);
            ImGui::PopItemWidth();
            ImGui::Indent(50);
            ImGui::PushItemWidth(500);
            if(ImGui::ColorEdit3("c2", &sec_clr.x, true))UpdateColors();
            ImGui::Indent(-50);
            ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::Indent(-GosChess::main_menu_width * 0.5f + 600 * 0.5f);
    ImGui::Dummy(ImVec2(0,80));
    ImGui::Indent(GosChess::main_menu_width * 0.5f - 250);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::MAIN_MENU;
    }
    ImGui::Indent(-GosChess::main_menu_width * 0.5f + 250);
    ImGui::PopStyleColor();
    ImGui::Indent(-GosChess::main_menu_width * 0.5f + 500 * 0.5f);
    ImGui::Dummy(ImVec2(0, 40));
    ImGui::Text("%s", choose_color_sec);
    ImGui::Indent(-GosChess::main_menu_width * 0.5f + 250.f);

    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(255, 0, 0, 1.00f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(255, 0, 0, 1.00f);
    style->Colors[ImGuiCol_ChildBg] = ImVec4(52.f / 255.f, 61.f / 255.f, 70.f / 255.f, 1.00f);
    style->ChildRounding = 25;
}

void GosChess::RenderHostGameBackground(sf::RenderWindow &window, ImGuiContext *context) {

}

void GosChess::RenderHostGameWidgets(sf::RenderWindow &, ImGuiContext *context) {
    ImGui::Dummy(ImVec2(0.0f, static_cast<float>(GosChess::main_menu_height) * 0.3f));
    auto text_width = ImGui::CalcTextSize(waiting_to_join).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
    ImGui::Text("%s", waiting_to_join);
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::MAIN_MENU;
        GosChess::listen_flag = false;
    }
    ImGui::Indent(-200);
}

void GosChess::RenderJoinGameBackground(sf::RenderWindow &, ImGuiContext *context) {

}

void GosChess::RenderJoinGameWidgets(sf::RenderWindow &, ImGuiContext *context) {
    ImGui::Dummy(ImVec2(0.0f, static_cast<float>(GosChess::main_menu_height) * 0.3f));
    auto text_width = ImGui::CalcTextSize(entered_ip).x;
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * 0.5f);
    ImGui::Text("%s", entered_ip);
    ImGui::Indent((static_cast<float>(GosChess::main_menu_width) - text_width) * -0.5f);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    ImGui::Indent(200);
    ImGui::InputTextMultiline("ip", GosChess::opponent_ip, 100, ImVec2(500, 60));
    ImGui::Indent(-200);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Join", ImVec2(500, 100))) {
        GosChess::remote_ip = sf::IpAddress(GosChess::opponent_ip);
        GosChess::JoinInit();
    }
    ImGui::Indent(-200);
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::MAIN_MENU;
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

void GosChess::MenuRenderConfig() {
    if (!GosChess::menu_wallpaper_txt.loadFromFile("../resources/image.png")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    sf::RectangleShape ae;
    GosChess::menu_wallpaper_sprite.setTexture(GosChess::menu_wallpaper_txt);
    sf::Vector2f texture_dimensions = {static_cast<float>(menu_wallpaper_txt.getSize().y),
                                       static_cast<float>(menu_wallpaper_txt.getSize().x)};
    GosChess::menu_wallpaper_sprite.scale(
            sf::Vector2f(static_cast<float>(GosChess::window_width) / texture_dimensions.y,
                         static_cast<float>(GosChess::window_height) / texture_dimensions.x));
    GosChess::menu_wallpaper_sprite.setPosition({0.f, 0.f});
}

namespace GosChess {

    int main_menu_width = 900;

    int main_menu_height = 900;

    bool menu_active_flag = true;

}




