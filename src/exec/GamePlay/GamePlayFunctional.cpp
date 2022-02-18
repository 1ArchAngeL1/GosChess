//
// Created by Leo Gogichaishvili on 29.01.22.
//


#include "GamePlayFunctional.h"
#include "../../network/GameNetwork.h"


void GosChess::MultiPlayerListener::Action(GosChess::Board &board) {
    static std::optional<GosChess::Vector2i> src_cell = std::nullopt;
    static std::optional<GosChess::Vector2i> trg_cell = std::nullopt;
    if (GosChess::CheckMate(board, GosChess::color_to_play)) {
        GosChess::SetGameFlagFinished();
        GosChess::game_result = GosChess::GameResult::LOST;
        return;
    }
    if (GosChess::player_color == GosChess::color_to_play &&
        GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {

        if (!GosChess::highlited) {
            src_cell = GosChess::ChooseSrcFigure(board, this->game_window);
        } else {
            trg_cell = GosChess::ChooseTrgFigure(board, this->game_window);
        }

        if (src_cell.has_value() && trg_cell.has_value()) {
            auto from = static_cast<int8_t>(GosChess::GetNumFromNode(src_cell.value()));
            auto to = static_cast<int8_t>(GosChess::GetNumFromNode(trg_cell.value()));
            if (GosChess::Play(board, GosChess::Move(GosChess::Move(from, to)))) {
                GosChess::ChangeActiveColour(board);
                GosChess::SendMove(GosChess::Move(from, to));
                src_cell = std::nullopt;
                trg_cell = std::nullopt;
            }
            if (GosChess::CheckMate(board, GosChess::color_to_play)) {
                GosChess::SetGameFlagFinished();
                GosChess::game_result = GosChess::GameResult::WON;
                return;
            }
        }
    }
}

//coming soon
void GosChess::GamePlayAIListener::Action(GosChess::Board &board) {
    static std::optional<GosChess::Vector2i> src_cell = std::nullopt;
    static std::optional<GosChess::Vector2i> trg_cell = std::nullopt;

    if (GosChess::player_color == GosChess::color_to_play &&
        GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
        if (!GosChess::highlited) {
            src_cell = GosChess::ChooseSrcFigure(board, this->game_window);
        } else {
            trg_cell = GosChess::ChooseTrgFigure(board, this->game_window);
        }
        if (src_cell.has_value() && trg_cell.has_value()) {
            auto from = static_cast<int8_t>(GosChess::GetNumFromNode(src_cell.value()));
            auto to = static_cast<int8_t>(GosChess::GetNumFromNode(trg_cell.value()));
            if (GosChess::Play(board, GosChess::Move(GosChess::Move(from, to)))) {
                GosChess::ChangeActiveColour(board);
                src_cell = std::nullopt;
                trg_cell = std::nullopt;

                if (GosChess::CheckMate(board, GosChess::enemy_color)) {
                    GosChess::SetGameFlagFinished();
                    GosChess::game_result = GosChess::GameResult::WON;
                    return;
                }

            }

        }
    }
}

void GosChess::MainMenuListener::Action(GosChess::Board &board) {
    if (connected) {
        GosChess::menu_active_flag = false;
        if (GosChess::connection_role == GosChess::HOST) {
            GosChess::InitialSend();
        } else if (GosChess::connection_role == GosChess::CLIENT) {
            GosChess::InitialReceive();
        }
    }

}


static constexpr std::string_view WHITE_STARTS_FEN = "RNBQKBNR/PPPPPPPP/8/8/8/8/pppppppp/rnbqkbnr/";

static constexpr std::string_view BLACK_STARTS_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/";


std::string GosChess::GetInitialFenBoard() {
    return GosChess::player_color == GosChess::Color::WHITE ? std::string(WHITE_STARTS_FEN) :
           std::string(BLACK_STARTS_FEN);
}

bool GosChess::Play(GosChess::Board &brd, const GosChess::Move &move) {
    if (GosChess::CanMakeMove(move)) {
        return GosChess::MakeMove(move, brd);
    }
}

std::optional<GosChess::Vector2i> GosChess::ChooseTrgFigure(GosChess::Board &brd, sf::Window &window) {
    if (!GosChess::highlited) {
        return std::nullopt;
    }
    GosChess::Vector2i trg_node = GosChess::GetNodeFromScreen(sf::Mouse::getPosition(window).y,
                                                              sf::Mouse::getPosition(window).x);
    GosChess::ResetBoardColours();
    highlited = false;
    return {trg_node};
}

std::optional<GosChess::Vector2i> GosChess::ChooseSrcFigure(GosChess::Board &brd, sf::Window &window) {
    if (GosChess::highlited) {
        GosChess::ResetBoardColours();
    }
    GosChess::Vector2i src_node = GosChess::GetNodeFromScreen(sf::Mouse::getPosition(window).y,
                                                              sf::Mouse::getPosition(window).x);
    GosChess::ColorizeAvailableMoves(GosChess::GetNumFromNode(src_node));
    highlited = true;
    return {src_node};
}


void GosChess::CheckReceivedMove(std::optional<GosChess::Move> move, GosChess::Board &board) {
    if (move.has_value()) {
        GosChess::MakeMoveForce(GosChess::InvertMove(move.value()), board);
        GosChess::ChangeActiveColour(board);
    }
}

void GosChess::CheckReceivedTime(GosChess::Time::Timer &player_timer, GosChess::Time::Timer &enemy_timer,
                                 GosChess::Time::TimerTransferObject dto) {
    player_timer.Set(dto.player_timer_amount);
    enemy_timer.Set(dto.enemy_timer_amount);

}

void GosChess::SetGameFlagFinished() {
    GosChess::game_status_flag = GosChess::GameStatus::FINISHED;
}

void GosChess::ResetGame(sf::RenderWindow &window) {
    window.clear();
    GosChess::available_moves.clear();
    GosChess::game_status_flag = GosChess::INGAME;
    GosChess::menu_active_flag = true;
    GosChess::render_menu_flag = GosChess::RenderMenuFLag::GAME_RESULT;
    GosChess::highlited = false;
    GosChess::color_to_play = GosChess::Color::WHITE;
    GosChess::connected = false;
    GosChess::listen_flag = false;
}

void GosChess::Disconnected() {
    GosChess::game_status_flag = GosChess::GameStatus::FINISHED;
    GosChess::game_result = GosChess::GameResult::WON;
}

namespace GosChess::Time {

    Timer::Timer(const float &time) {
        this->Convert(time);
    }

    bool Timer::Subtract(const float &amount) {
        this->seconds -= amount;
        if (this->seconds < 0) {
            this->seconds += 60;
            this->minutes--;
        }
        return minutes >= 0;
    }

    std::string Timer::ToString() const {
        std::string minute_str = std::to_string(this->minutes);
        std::string seconds_str = std::to_string(this->seconds).substr(0, 2);
        if (seconds_str[1] == '.')seconds_str.erase(seconds_str.begin() + 1);
        if (seconds_str.length() == 1)seconds_str = "0" + seconds_str;
        if (minute_str.length() == 1)minute_str = "0" + minute_str;
        return minute_str + " : " + seconds_str;
    }

    void Timer::Convert(const float &time) {
        this->minutes = static_cast<short>(time) / 60;
        this->seconds = time - this->minutes * 60;
    }

    void Timer::Set(const float &time) {
        this->Convert(time);
    }

    float Timer::GetAmount() const {
        return static_cast<float>(this->minutes * 60) + this->seconds;
    }

}

char GosChess::opponent_ip[50];

bool GosChess::highlited = false;

GosChess::GameStatus GosChess::game_status_flag = GosChess::GameStatus::INGAME;

GosChess::GameResult GosChess::game_result = GosChess::GameResult::NOT_FINISHED;

GosChess::GameMode GosChess::game_mode = GosChess::GameMode::SINGLE_PLAYER;