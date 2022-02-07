//
// Created by Leo Gogichaishvili on 01.02.22.
//


#include "MenuRender.h"
#include "../../exec/GamePlay/GamePlayFunctional.h"
#include "../../exec/GamePlay/GameLoops.h"


static constexpr int buffer_size = 100;

static char user_name[buffer_size];

static std::string welcome_text = "Welcome To Chess!";
static std::string entered_ip = "Eenter oponents public ip";
static std::string waiting_to_join = "Waiting someone to join ...";


void GosChess::RenderBackgroundWallpaper(sf::RenderWindow &window) {
    window.draw(GosChess::menu_wallpaper_sprite);
}

void GosChess::RenderMainMenuBackground(sf::RenderWindow & window, ImGuiContext * context) {
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

void GosChess::RenderMainMenuWidgets(sf::RenderWindow & window, ImGuiContext * context) {
    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    auto textWidth = ImGui::CalcTextSize(welcome_text.c_str()).x;
    ImGui::Indent((GosChess::main_menu_width - textWidth) * 0.5f);
    ImGui::Text(welcome_text.c_str());
    ImGui::Dummy(ImVec2(0.0f, 40.0f));
    ImGui::Indent((GosChess::main_menu_width - textWidth) * -0.5f);

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
    }
    ImGui::Indent(-200);
    ImGui::Indent(200);
    if (ImGui::Button("Options", ImVec2(500, 100))) {
        GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::OPTION;
    }
    ImGui::Indent(-200);
}

void GosChess::RenderOptionsBackground(sf::RenderWindow & window,ImGuiContext * context) {

}

void GosChess::RenderOptionsWidgets(sf::RenderWindow & window, ImGuiContext * context) {

}

void GosChess::RenderHostGameBackground(sf::RenderWindow & window, ImGuiContext * context) {

}

void GosChess::RenderHostGameWidgets(sf::RenderWindow &, ImGuiContext * context) {
    ImGui::Dummy(ImVec2(0.0f, GosChess::main_menu_height / 3.0f));
    auto text_width = ImGui::CalcTextSize(waiting_to_join.c_str()).x;
    ImGui::Indent((GosChess::main_menu_width - text_width) * 0.5f);
    ImGui::Text(waiting_to_join.c_str());
    ImGui::Indent((GosChess::main_menu_width - text_width) * -0.5f);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Return", ImVec2(500, 100))) {
        GosChess::RenderFlags::render_menu_flag = GosChess::RenderFlags::MAIN_MENU;
    }
    ImGui::Indent(-200);
}

void GosChess::RenderJoinGameBackground(sf::RenderWindow &, ImGuiContext * context) {

}

void GosChess::RenderJoinGameWidgets(sf::RenderWindow &, ImGuiContext * context) {
    ImGui::Dummy(ImVec2(0.0f, GosChess::main_menu_height / 3.0f));
    auto text_width = ImGui::CalcTextSize(entered_ip.c_str()).x;
    ImGui::Indent((GosChess::main_menu_width - text_width) * 0.5f);
    ImGui::Text(entered_ip.c_str());
    ImGui::Indent((GosChess::main_menu_width - text_width) * -0.5f);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    ImGui::Indent(200);
    ImGui::InputTextMultiline("Ip", GosChess::opponent_ip, 100, ImVec2(500, 60));
    ImGui::Indent(-200);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));
    ImGui::Indent(200);
    if (ImGui::Button("Join", ImVec2(500, 100))) {

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
            ImVec2(GosChess::window_width / 2 - GosChess::main_menu_width / 2,
                   GosChess::window_height / 2 - GosChess::main_menu_height / 2));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.188, 0.09f, 0.203f, 0.6f));
    ImGui::Begin("Hello", nullptr, ImGuiWindowFlags_::ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoResize);
    ImGuiContext *context = ImGui::GetCurrentContext();
    background_renderer(window, context);
    widget_renderer(window, context);
    ImGui::PopStyleColor();
    ImGui::End();
}

void GosChess::MenuRenderConfig() {
    if (!GosChess::menu_wallpaper_txt.loadFromFile("../resources/menu_wallpaper3.jpeg")) {
        std::cout << "couldn't load root_texture for figures" << std::endl;
        return;
    }
    sf::RectangleShape ae;
    GosChess::menu_wallpaper_sprite.setTexture(GosChess::menu_wallpaper_txt);
    sf::Vector2f texture_dimensions = {static_cast<float>(menu_wallpaper_txt.getSize().y),
                                       static_cast<float>(menu_wallpaper_txt.getSize().x)};
    GosChess::menu_wallpaper_sprite.scale(sf::Vector2f(GosChess::window_width / texture_dimensions.y,
                                                       GosChess::window_height / texture_dimensions.x));
    GosChess::menu_wallpaper_sprite.setPosition({0.f, 0.f});
}

namespace GosChess {

    int main_menu_width = 900;

    int main_menu_height = 900;

    bool main_menu_opened = false;

}




