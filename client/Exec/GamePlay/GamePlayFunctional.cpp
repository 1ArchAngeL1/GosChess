//
// Created by Leo Gogichaishvili on 29.01.22.
//


#include "GamePlayFunctional.hh"
#include "../../Network/GameNetwork.hh"


static bool CheckForGameFinish(GosChess::board_t &board, GosChess::color_t color) {
    if (GosChess::connection_role == GosChess::ConnectionType::HOST) {
        if (GosChess::CheckMate(board, color)) return true;
    }
    return false;
}

static bool CheckForGameDraw(GosChess::board_t &board, GosChess::color_t color) {
    if (GosChess::connection_role == GosChess::ConnectionType::HOST) {
        if (!GosChess::CheckMate(board, color) && GosChess::CheckForDraw(board, color))return true;
    }
    return false;
}


void GosChess::MultiPlayerListener::action(GosChess::board_t &board) {
    static std::optional<GosChess::board_node_t> src_cell = std::nullopt;
    static std::optional<GosChess::board_node_t> trg_cell = std::nullopt;
    if (CheckForGameFinish(board, color_to_play)) {
        GosChess::SetGameFlagFinished();
        GosChess::game_result = GosChess::GameResult::LOST;
        GosChess::SendResult(GosChess::GameResultTransfer(GosChess::GameResult::WON));
        return;
    } else if (CheckForGameDraw(board, color_to_play)) {
        GosChess::SetGameFlagFinished();
        GosChess::game_result = GosChess::GameResult::DRAW;
        GosChess::SendResult(GosChess::GameResultTransfer(GosChess::GameResult::DRAW));
    }
    if (GosChess::player_color == GosChess::color_to_play &&
        GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
        if (!GosChess::highlited) src_cell = GosChess::ChooseSrcFigure(board, this->game_window);
        else trg_cell = GosChess::ChooseTrgFigure(board, this->game_window);
        if (src_cell.has_value() && trg_cell.has_value()) {
            auto from = static_cast<int8_t>(GosChess::GetNumFromNode(src_cell.value()));
            auto to = static_cast<int8_t>(GosChess::GetNumFromNode(trg_cell.value()));
            if (GosChess::MakeMove(GosChess::action_t(GosChess::action_t(from, to)), board, GosChess::color_to_play,
                                   GosChess::game_available_moves[from])) {
                GosChess::ChangeActiveColour(board);
                GosChess::SendMove(GosChess::action_t(from, to));
                src_cell = std::nullopt, trg_cell = std::nullopt;
                if (CheckForGameFinish(board, color_to_play)) {
                    GosChess::SetGameFlagFinished();
                    GosChess::game_result = GosChess::GameResult::WON;
                    GosChess::SendResult(GosChess::GameResultTransfer(GosChess::GameResult::LOST));
                } else if (CheckForGameDraw(board, color_to_play)) {
                    GosChess::SetGameFlagFinished();
                    GosChess::game_result = GosChess::GameResult::DRAW;
                    GosChess::SendResult(GosChess::GameResultTransfer(GosChess::GameResult::DRAW));
                }
            }
        }
    }
}

//coming soon
void GosChess::GamePlayAIListener::action(GosChess::board_t &board) {
    static std::optional<GosChess::board_node_t> src_cell = std::nullopt;
    static std::optional<GosChess::board_node_t> trg_cell = std::nullopt;
    if (GosChess::player_color == GosChess::color_to_play) {
        if (GosChess::CheckMate(board, GosChess::player_color)) {
            GosChess::SetGameFlagFinished();
            GosChess::game_result = GosChess::GameResult::LOST;
            return;
        } else if (CheckForDraw(board, player_color)) {
            GosChess::SetGameFlagFinished();
            GosChess::game_result = GosChess::GameResult::DRAW;
            return;
        }
        if (GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) {
            if (!GosChess::highlited) src_cell = GosChess::ChooseSrcFigure(board, this->game_window);
            else trg_cell = GosChess::ChooseTrgFigure(board, this->game_window);
            if (src_cell.has_value() && trg_cell.has_value()) {
                auto from = static_cast<int8_t>(GosChess::GetNumFromNode(src_cell.value()));
                auto to = static_cast<int8_t>(GosChess::GetNumFromNode(trg_cell.value()));
                if (GosChess::MakeMove(GosChess::action_t(GosChess::action_t(from, to)), board, GosChess::color_to_play,
                                       GosChess::game_available_moves[from])) {
                    GosChess::ChangeActiveColour(board);
                    src_cell = std::nullopt, trg_cell = std::nullopt;
                    if (GosChess::CheckMate(board, GosChess::enemy_color)) {
                        GosChess::SetGameFlagFinished();
                        GosChess::game_result = GosChess::GameResult::WON;
                        return;
                    } else if (CheckForDraw(board, enemy_color)) {
                        GosChess::SetGameFlagFinished();
                        GosChess::game_result = GosChess::GameResult::DRAW;
                        return;
                    }
                }
            }
        }
    }


}

void GosChess::MainMenuListener::action(GosChess::board_t &board) {
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

static constexpr std::string_view WHITE_STARTS_FEN_TEST = "KQQ5/8/8/8/8/8/8/3k4/";

static constexpr std::string_view BLACK_STARTS_FEN_TEST = "3k4/8/8/8/8/8/8/KQQ5/";

std::string GosChess::GetInitialFenBoard() {
    return GosChess::player_color == GosChess::color_t::WHITE ? std::string(WHITE_STARTS_FEN) :
           std::string(BLACK_STARTS_FEN);
}

std::optional<GosChess::board_node_t> GosChess::ChooseTrgFigure(GosChess::board_t &brd, sf::Window &window) {
    if (!GosChess::highlited) {
        return std::nullopt;
    }
    GosChess::board_node_t trg_node = GosChess::GetNodeFromScreen(sf::Mouse::getPosition(window).y,
                                                                  sf::Mouse::getPosition(window).x);
    GosChess::ResetBoardColours();
    highlited = false;
    return {trg_node};
}

std::optional<GosChess::board_node_t> GosChess::ChooseSrcFigure(GosChess::board_t &brd, sf::Window &window) {
    if (GosChess::highlited) {
        GosChess::ResetBoardColours();
    }
    GosChess::board_node_t src_node = GosChess::GetNodeFromScreen(sf::Mouse::getPosition(window).y,
                                                                  sf::Mouse::getPosition(window).x);
    GosChess::ColorizeAvailableMoves(GosChess::GetNumFromNode(src_node),brd);
    highlited = true;
    return {src_node};
}


void GosChess::CheckReceivedMove(std::optional<GosChess::action_t> move, GosChess::board_t &board) {
    if (move.has_value()) {
        GosChess::MakeMoveForce(GosChess::InvertMove(move.value()), board);
        GosChess::ChangeActiveColour(board);
    }
}

void GosChess::CheckReceivedTime(GosChess::Time::Timer &player_timer, GosChess::Time::Timer &enemy_timer,
                                 GosChess::Time::TimerTransferObject dto) {
    player_timer.set(dto.player_timer_amount);
    enemy_timer.set(dto.enemy_timer_amount);

}

void GosChess::SetGameFlagFinished() {
    GosChess::game_status_flag = GosChess::game_status_t::FINISHED;
}

void GosChess::ResetGame(sf::RenderWindow &window) {
    window.clear();
    GosChess::game_available_moves.clear();
    GosChess::game_status_flag = GosChess::INGAME;
    GosChess::render_menu_flag = GosChess::RenderMenuFLag::GAME_RESULT;
    GosChess::color_to_play = GosChess::color_t::WHITE;
    GosChess::game_mode = GosChess::GameMode::None;
    GosChess::highlited = false;
    GosChess::menu_active_flag = true;
    GosChess::connected = false;
    GosChess::listen_flag = false;
    GosChess::time_limit_minutes = 0;
}

void GosChess::Disconnected() {
    GosChess::game_status_flag = GosChess::game_status_t::FINISHED;
    GosChess::game_result = GosChess::GameResult::WON;
}


void GosChess::ProcessGameResult(GosChess::GameResultTransfer &result) {
    GosChess::game_result = result.result;
}

namespace GosChess::Time {

    Timer::Timer(const float &time) {
        this->convert(time);
    }

    bool Timer::subtract(const float &amount) {
        this->seconds -= amount;
        if (this->seconds < 0) {
            this->seconds += 60;
            this->minutes--;
        }
        return minutes >= 0;
    }

    std::string Timer::toString() const {
        std::string minute_str = std::to_string(this->minutes);
        std::string seconds_str = std::to_string(this->seconds).substr(0, 2);
        if (seconds_str[1] == '.')seconds_str.erase(seconds_str.begin() + 1);
        if (seconds_str.length() == 1)seconds_str = "0" + seconds_str;
        if (minute_str.length() == 1)minute_str = "0" + minute_str;
        return minute_str + " : " + seconds_str;
    }

    void Timer::convert(const float &time) {
        this->minutes = static_cast<short>(time) / 60;
        this->seconds = time - this->minutes * 60;
    }

    void Timer::set(const float &time) {
        this->convert(time);
    }

    float Timer::getAmount() const {
        return static_cast<float>(this->minutes * 60) + this->seconds;
    }

}

char GosChess::opponent_ip[50];

bool GosChess::highlited = false;

GosChess::game_status_t GosChess::game_status_flag = GosChess::game_status_t::INGAME;

GosChess::GameResult GosChess::game_result = GosChess::GameResult::DRAW;

GosChess::GameMode GosChess::game_mode = GosChess::GameMode::None;