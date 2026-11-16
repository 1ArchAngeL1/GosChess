//
// Created by Leo Gogichaishvili on 01.02.22.
//

#include "MenuRender.hh"

#include <algorithm>
#include <iostream>
#include <string>

#include "../../Audio/GameAudio.hh"
#include "../UiFonts.hh"
#include "../../Network/ServerConnection.hh"

// The menu pages.
//
// They used to be laid out by pushing Indent by eyeballed pixel counts, which
// is why labels sat to the right of their fields and nothing lined up with
// anything else. Everything here goes through the two helpers below, so a page
// is a list of rows and the centring is done once.

namespace {

    constexpr const char *TIME_CHOICES[] = {"No clock", "1 minute", "3 minutes",
                                            "10 minutes", "30 minutes"};
    constexpr std::uint16_t TIME_MINUTES[] = {0, 1, 3, 10, 30};
    constexpr int TIME_CHOICE_COUNT = 5;

    int chosen_time = 0;

    ImVec4 main_clr(GosChess::main_color.r / 255.f, GosChess::main_color.g / 255.f,
                    GosChess::main_color.b / 255.f, 1.f);
    ImVec4 sec_clr(GosChess::secondary_color.r / 255.f, GosChess::secondary_color.g / 255.f,
                   GosChess::secondary_color.b / 255.f, 1.f);

    const ImVec4 MUTED = ImVec4(0.62f, 0.65f, 0.70f, 1.f);

    float ContentWidth() {
        return ImGui::GetContentRegionAvail().x;
    }

    // The offset is clamped at zero: text wider than the card used to get a
    // negative indent, which pushed it off both edges at once.
    void CentreNext(const char *text) {
        const float width = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                             std::max(0.f, (ContentWidth() - width) * 0.5f));
    }

    void Heading(const char *text) {
        CentreNext(text);
        ImGui::TextUnformatted(text);
    }

    // Set in the smaller face, both for the hierarchy and because a sentence
    // at heading size does not fit on a card.
    void Subheading(const char *text) {
        if (GosChess::PanelFont() != nullptr) ImGui::PushFont(GosChess::PanelFont());
        CentreNext(text);
        ImGui::TextColored(MUTED, "%s", text);
        if (GosChess::PanelFont() != nullptr) ImGui::PopFont();
    }

    void SmallLabel(const char *text) {
        if (GosChess::PanelFont() != nullptr) ImGui::PushFont(GosChess::PanelFont());
        ImGui::TextColored(MUTED, "%s", text);
        if (GosChess::PanelFont() != nullptr) ImGui::PopFont();
    }

    // One full-width row, so every button on every page is the same size and
    // in the same place.
    bool MenuButton(const char *label) {
        const bool pressed = ImGui::Button(label, ImVec2(-1.f, ImGui::GetFontSize() * 1.9f));
        if (pressed) GosChess::PlayUiClickSound();
        return pressed;
    }

    void Gap(float height) {
        ImGui::Dummy(ImVec2(0.f, height));
    }

    // A labelled field with its caption above it rather than to the right,
    // where ImGui puts it by default and where it ran off the panel.
    void LabelledInput(const char *caption, const char *id, char *buffer, std::size_t size) {
        SmallLabel(caption);
        ImGui::SetNextItemWidth(-1.f);
        ImGui::InputText(id, buffer, size);
    }

    void UpdateColors() {
        GosChess::main_color.r = static_cast<sf::Uint8>(255.f * main_clr.x);
        GosChess::main_color.g = static_cast<sf::Uint8>(255.f * main_clr.y);
        GosChess::main_color.b = static_cast<sf::Uint8>(255.f * main_clr.z);
        GosChess::secondary_color.r = static_cast<sf::Uint8>(255.f * sec_clr.x);
        GosChess::secondary_color.g = static_cast<sf::Uint8>(255.f * sec_clr.y);
        GosChess::secondary_color.b = static_cast<sf::Uint8>(255.f * sec_clr.z);
    }

    void InitWallpaperAssets() {
        if (!GosChess::MenuWallpaperTexture().loadFromFile(GOSCHESS_RESOURCES "/image.png")) {
            std::cerr << "could not load the menu wallpaper" << std::endl;
            return;
        }
        GosChess::MenuWallpaperTexture().setSmooth(true);
        GosChess::menu_wallpaper_sprite.setTexture(GosChess::MenuWallpaperTexture(), true);
    }

}

void GosChess::RenderBackgroundWallpaper(sf::RenderWindow &window) {
    // Rescaled every frame so the wallpaper still covers the window after a
    // resize instead of leaving a bar of background down one side.
    const sf::Vector2u texture_size = MenuWallpaperTexture().getSize();
    if (texture_size.x == 0 || texture_size.y == 0) return;
    menu_wallpaper_sprite.setScale(static_cast<float>(window.getSize().x) / texture_size.x,
                                   static_cast<float>(window.getSize().y) / texture_size.y);
    menu_wallpaper_sprite.setPosition(0.f, 0.f);
    window.draw(menu_wallpaper_sprite);
}

void GosChess::RenderMainMenuBackground(sf::RenderWindow &window, ImGuiContext *context) {
}

void GosChess::RenderOptionsBackground(sf::RenderWindow &window, ImGuiContext *context) {
}

void GosChess::RenderAiGameBackground(sf::RenderWindow &window, ImGuiContext *context) {
}

void GosChess::RenderGameResultBackground(sf::RenderWindow &, ImGuiContext *) {
}

void GosChess::RenderMainMenuWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    Gap(10.f);
    Heading("GosChess");
    Subheading("Online, or against the computer");
    Gap(40.f);

    if (MenuButton("Play Online")) {
        render_menu_flag = RenderMenuFLag::ONLINE;
    }
    if (MenuButton("Play the Computer")) {
        render_menu_flag = RenderMenuFLag::COMPUTER;
    }
    if (MenuButton("Settings")) {
        render_menu_flag = RenderMenuFLag::OPTION;
    }
    Gap(20.f);
    if (MenuButton("Quit")) {
        LeaveServer();
        window.close();
    }
}

void GosChess::RenderAiGameWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    Gap(10.f);
    Heading("Play the Computer");
    Gap(30.f);

    SmallLabel("Difficulty");
    ImGui::SetNextItemWidth(-1.f);
    ImGui::SliderInt("##level", &ai_level, 1, MAX_AI_LEVEL);
    Subheading("Higher levels look further ahead");
    Gap(30.f);

    if (MenuButton("Start")) {
        game_mode = GameMode::SINGLE_PLAYER;
        // Leaving the menu used to go through the peer-to-peer "connected"
        // flag, even for a game against the computer where nothing connects.
        menu_active_flag = false;
    }
    if (MenuButton("Back")) {
        render_menu_flag = RenderMenuFLag::MAIN_MENU;
    }
}

void GosChess::RenderOptionsWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    Gap(10.f);
    Heading("Settings");
    Gap(30.f);

    SmallLabel("Dark squares");
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::ColorEdit3("##dark", &main_clr.x, ImGuiColorEditFlags_NoInputs)) UpdateColors();
    Gap(10.f);
    SmallLabel("Light squares");
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::ColorEdit3("##light", &sec_clr.x, ImGuiColorEditFlags_NoInputs)) UpdateColors();
    Gap(20.f);

    bool sound = SoundEnabled();
    if (ImGui::Checkbox("Sound", &sound)) {
        SetSoundEnabled(sound);
        // Played after the switch so turning it on is audible.
        PlayUiClickSound();
    }
    Gap(30.f);

    if (MenuButton("Back")) {
        render_menu_flag = RenderMenuFLag::MAIN_MENU;
    }
}

void GosChess::RenderGameResultWidgets(sf::RenderWindow &, ImGuiContext *) {
    Gap(10.f);
    Heading("Game over");
    Gap(30.f);
    if (MenuButton("Back")) {
        render_menu_flag = RenderMenuFLag::MAIN_MENU;
    }
}

void GosChess::RenderOnlineWidgets(sf::RenderWindow &window, ImGuiContext *context) {
    const LinkState state = GetLinkState();

    if (state == LinkState::CONNECTING || state == LinkState::QUEUED) {
        Gap(10.f);
        Heading(state == LinkState::CONNECTING ? "Connecting..." : "Looking for an opponent...");
        Gap(20.f);
        Subheading(state == LinkState::CONNECTING
                   ? "Reaching the server"
                   : "Pairing you with the next player on the same clock");
        Gap(40.f);
        if (MenuButton("Cancel")) {
            LeaveServer();
        }
        return;
    }

    Gap(10.f);
    Heading("Play Online");
    Gap(30.f);

    LabelledInput("Server", "##server", server_address, sizeof(server_address));
    Gap(10.f);
    SmallLabel("Port");
    ImGui::SetNextItemWidth(-1.f);
    ImGui::InputInt("##port", &server_port);
    Gap(10.f);

    SmallLabel("Clock");
    ImGui::SetNextItemWidth(-1.f);
    if (ImGui::BeginCombo("##clock", TIME_CHOICES[chosen_time])) {
        for (int i = 0; i < TIME_CHOICE_COUNT; i++) {
            const bool selected = chosen_time == i;
            if (ImGui::Selectable(TIME_CHOICES[i], selected)) chosen_time = i;
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (state == LinkState::FAILED && !GetLinkError().empty()) {
        Gap(10.f);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.90f, 0.45f, 0.40f, 1.f));
        ImGui::TextWrapped("%s", GetLinkError().c_str());
        ImGui::PopStyleColor();
    }

    Gap(30.f);
    if (MenuButton("Find Game")) {
        // The server is told the clock and hands one back with every position,
        // so the client never has to work one out for itself.
        JoinServer(server_address, static_cast<unsigned short>(server_port),
                   TIME_MINUTES[chosen_time]);
    }
    if (MenuButton("Back")) {
        LeaveServer();
        render_menu_flag = RenderMenuFLag::MAIN_MENU;
    }
}

void GosChess::RenderMenu(void (*background_renderer)(sf::RenderWindow &, ImGuiContext *),
                          void (*widget_renderer)(sf::RenderWindow &, ImGuiContext *),
                          sf::RenderWindow &window) {
    // A dark scrim over the wallpaper. Without it the purple photograph fights
    // the board's own colours on every other screen.
    sf::RectangleShape scrim(sf::Vector2f(static_cast<float>(window.getSize().x),
                                          static_cast<float>(window.getSize().y)));
    scrim.setFillColor(sf::Color(14, 15, 18, 170));
    window.draw(scrim);

    const float width = std::min(560.f, static_cast<float>(window.getSize().x) - 80.f);
    main_menu_width = static_cast<int>(width);

    // Centred by pivot rather than by subtracting a size, so the card can size
    // itself to its contents and still sit in the middle. The pages have
    // different numbers of rows and a fixed height left dead space below the
    // short ones.
    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(window.getSize().x) * 0.5f,
                                   static_cast<float>(window.getSize().y) * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.11f, 0.96f));
    ImGui::Begin("menu", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar |
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
    // Pins the width; the height follows whatever the page puts in it.
    ImGui::Dummy(ImVec2(width - ImGui::GetStyle().WindowPadding.x * 2.f, 0.f));
    background_renderer(window, ImGui::GetCurrentContext());
    widget_renderer(window, ImGui::GetCurrentContext());
    main_menu_height = static_cast<int>(ImGui::GetWindowHeight());
    ImGui::End();
    ImGui::PopStyleColor();
}

void GosChess::MenuRenderConfig() {
    InitWallpaperAssets();
}


int GosChess::main_menu_width = 620;

int GosChess::main_menu_height = 760;
