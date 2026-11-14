#include "ClientGame.hh"

#include <algorithm>
#include <cmath>

namespace {

    // Wound down for display between server updates; reset by every one of
    // them. Never used to decide anything.
    float display_white_millis = 0.f;
    float display_black_millis = 0.f;

    std::string Format(float millis) {
        const int total_seconds = static_cast<int>(std::max(0.f, millis) / 1000.f);
        const int minutes = total_seconds / 60;
        const int seconds = total_seconds % 60;
        std::string minute_text = std::to_string(minutes);
        std::string second_text = std::to_string(seconds);
        if (minute_text.size() == 1) minute_text.insert(minute_text.begin(), '0');
        if (second_text.size() == 1) second_text.insert(second_text.begin(), '0');
        return minute_text + " : " + second_text;
    }

}

GosChess::GameState GosChess::game;

GosChess::color_t GosChess::player_color = GosChess::color_t::WHITE;

GosChess::color_t GosChess::enemy_color = GosChess::color_t::BLACK;

std::uint32_t GosChess::confirmed_ply = 0;

bool GosChess::awaiting_confirmation = false;

std::uint16_t GosChess::time_limit_minutes = 0;

void GosChess::SetPlayerColor(GosChess::color_t color) {
    GosChess::player_color = color;
    GosChess::enemy_color = GosChess::OppositeColor(color);
}

bool GosChess::PlayerToMove() {
    return GosChess::game.colorToPlay() == GosChess::player_color;
}

void GosChess::AdoptServerState(const GosChess::Protocol::GameStateUpdate &update) {
    game.setPosition(update.fen, update.color_to_play);
    confirmed_ply = update.ply;
    awaiting_confirmation = false;
    display_white_millis = static_cast<float>(update.white_millis_left);
    display_black_millis = static_cast<float>(update.black_millis_left);
}

bool GosChess::PredictMove(GosChess::action_t move) {
    if (!game.applyMove(move)) return false;
    awaiting_confirmation = true;
    return true;
}

void GosChess::ResetClientGame() {
    game.reset();
    confirmed_ply = 0;
    awaiting_confirmation = false;
    time_limit_minutes = 0;
    display_white_millis = display_black_millis = 0.f;
}

void GosChess::TickClockDisplay(float delta_seconds) {
    if (!GameHasClock()) return;
    if (game.finished()) return;
    float &running = game.colorToPlay() == color_t::WHITE ? display_white_millis : display_black_millis;
    running = std::max(0.f, running - delta_seconds * 1000.f);
}

std::string GosChess::PlayerClockText() {
    if (!GameHasClock()) return "--:--";
    return Format(player_color == color_t::WHITE ? display_white_millis : display_black_millis);
}

std::string GosChess::OpponentClockText() {
    if (!GameHasClock()) return "--:--";
    return Format(enemy_color == color_t::WHITE ? display_white_millis : display_black_millis);
}

bool GosChess::GameHasClock() {
    return time_limit_minutes > 0;
}
