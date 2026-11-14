//
// Created by Leo Gogichaishvili on 29.01.22.
//


#include "GamePlayFunctional.hh"
#include "../../Network/ServerConnection.hh"
#include "GameLoops.hh"

#include <iostream>


// Click handling is the same in both game modes: the first press picks a
// piece and highlights where it may go, the second press tries to move it
// there. Keeping the two presses in one place is what stops a failed move
// from leaving a stale target behind.
static std::optional<int> selected_square = std::nullopt;

static void ClearSelection() {
    if (GosChess::highlited) GosChess::ResetBoardColours();
    GosChess::highlited = false;
    selected_square = std::nullopt;
}

// The board square under the mouse, or nothing when the click landed outside
// the board. The index is absolute, so it already accounts for the board
// being drawn rotated for the player on the black side.
static std::optional<int> ClickedSquare(sf::Window &window) {
    const sf::Vector2i mouse = sf::Mouse::getPosition(window);
    const GosChess::view_cell_t cell = GosChess::ViewCellFromScreen(static_cast<float>(mouse.y),
                                                                    static_cast<float>(mouse.x));
    if (!GosChess::ViewCellOnBoard(cell)) return std::nullopt;
    return GosChess::ViewCellToBoardIndex(cell);
}

// Returns a move only on the click that completes one.
static std::optional<GosChess::action_t> PollPlayerMove(sf::Window &window) {
    if (!GosChess::InputHandle::KeyPressed(sf::Keyboard::Enter)) return std::nullopt;
    const std::optional<int> clicked = ClickedSquare(window);
    if (!clicked.has_value()) return std::nullopt;

    if (!selected_square.has_value()) {
        // Only a square this player can actually move from is worth selecting.
        if (GosChess::game.movesFrom(clicked.value()).empty()) return std::nullopt;
        selected_square = clicked;
        GosChess::highlited = true;
        GosChess::ColorizeAvailableMoves(clicked.value());
        return std::nullopt;
    }

    const GosChess::action_t move(static_cast<std::int8_t>(selected_square.value()),
                                  static_cast<std::int8_t>(clicked.value()));
    ClearSelection();
    return move;
}

bool GosChess::SettleLocalGame() {
    switch (GosChess::game.status()) {
        case GosChess::GameStatus::CHECKMATE:
            // Whoever is left to move is the side that has been mated.
            GosChess::game_result = GosChess::game.colorToPlay() == GosChess::player_color
                                    ? GosChess::GameResult::LOST
                                    : GosChess::GameResult::WON;
            GosChess::SetGameFlagFinished();
            return true;
        case GosChess::GameStatus::STALEMATE:
            GosChess::game_result = GosChess::GameResult::DRAW;
            GosChess::SetGameFlagFinished();
            return true;
        case GosChess::GameStatus::IN_PROGRESS:
            return false;
    }
    return false;
}

void GosChess::MultiPlayerListener::action() {
    // Nothing in here decides anything about the game. It reads the player's
    // intention, shows it immediately, and tells the server - which is the only
    // thing that can make it true.
    if (GosChess::game_status_flag == GosChess::game_status_t::FINISHED) return;
    if (!GosChess::PlayerToMove()) return;
    // One move may be in flight at a time. In chess that is automatic, since
    // the turn passes, but relying on that silently would be a trap for later.
    if (GosChess::awaiting_confirmation) return;

    const std::optional<GosChess::action_t> move = PollPlayerMove(this->game_window);
    if (!move.has_value()) return;

    // Played locally first so the board answers the click at once. If the
    // server disagrees it sends back the real position and this is undone;
    // because both ends run the same rules over the same board, it does not.
    if (!GosChess::PredictMove(move.value())) return;
    GosChess::SendMove(move.value());
}

void GosChess::GamePlayAIListener::action() {
    // The game against the computer is played entirely here, so this is the
    // one mode where the client really is the authority.
    if (GosChess::SettleLocalGame()) return;
    if (!GosChess::PlayerToMove()) return;
    const std::optional<GosChess::action_t> move = PollPlayerMove(this->game_window);
    if (!move.has_value()) return;
    if (!GosChess::game.applyMove(move.value())) return;
    GosChess::SettleLocalGame();
}

void GosChess::MainMenuListener::action() {
    // The server can answer while the menu is still up - that is how being
    // paired ends the menu - so its messages are read here too.
    GosChess::ProcessServerMessages();
}

void GosChess::SetGameFlagFinished() {
    GosChess::game_status_flag = GosChess::game_status_t::FINISHED;
}

void GosChess::ResetGame(sf::RenderWindow &window) {
    window.clear();
    GosChess::LeaveServer();
    GosChess::ResetClientGame();
    GosChess::game_status_flag = GosChess::INGAME;
    GosChess::render_menu_flag = GosChess::RenderMenuFLag::GAME_RESULT;
    GosChess::game_mode = GosChess::GameMode::None;
    ClearSelection();
    GosChess::menu_active_flag = true;
}

bool GosChess::highlited = false;

GosChess::game_status_t GosChess::game_status_flag = GosChess::game_status_t::INGAME;

GosChess::GameResult GosChess::game_result = GosChess::GameResult::DRAW;

GosChess::GameMode GosChess::game_mode = GosChess::GameMode::None;

bool GosChess::menu_active_flag = true;
