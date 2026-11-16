//
// Created by Leo Gogichaishvili on 29.01.22.
//


#include "GamePlayFunctional.hh"
#include "../../Network/ServerConnection.hh"
#include "GameLoops.hh"

#include <imgui.h>

#include <iostream>


namespace {

    void ClearSelection() {
        GosChess::selected_square = -1;
    }

    // The board square under the pointer, or nothing when the pointer is off
    // the board. The index is absolute, so it already accounts for the board
    // being drawn rotated for the player on the black side.
    std::optional<int> SquareUnderPointer(const sf::RenderWindow &window) {
        const sf::Vector2i mouse = sf::Mouse::getPosition(window);
        const GosChess::view_cell_t cell = GosChess::ViewCellFromScreen(
                static_cast<float>(mouse.y), static_cast<float>(mouse.x));
        if (!GosChess::ViewCellOnBoard(cell)) return std::nullopt;
        return GosChess::ViewCellToBoardIndex(cell);
    }

    bool HasALegalMove(int square) {
        return !GosChess::game.legalMovesFrom(square).empty();
    }

    bool CanMoveTo(int from, int to) {
        for (const GosChess::action_t &move: GosChess::game.legalMovesFrom(from)) {
            if (move.move_to == to) return true;
        }
        return false;
    }

    // Click to pick a piece up, click again to put it down. Returns a move only
    // on the click that completes one.
    //
    // Moving used to require holding the pointer over a square and pressing
    // Enter, because the mouse state was never actually reported (see
    // GameInput.cpp). Clicking is what anybody will try first.
    std::optional<GosChess::action_t> PollPlayerMove(const sf::RenderWindow &window,
                                                     bool player_may_move) {
        if (!GosChess::InputHandle::MousePressed(sf::Mouse::Left)) return std::nullopt;
        // The panel is drawn on top of the window, so a click it wants is not a
        // click on the board underneath it.
        if (ImGui::GetIO().WantCaptureMouse) return std::nullopt;

        const std::optional<int> clicked = SquareUnderPointer(window);
        if (!clicked.has_value()) {
            ClearSelection();
            return std::nullopt;
        }
        const int square = clicked.value();

        if (GosChess::selected_square < 0) {
            if (player_may_move && HasALegalMove(square)) GosChess::selected_square = square;
            return std::nullopt;
        }

        // Clicking the same square again puts the piece back down.
        if (square == GosChess::selected_square) {
            ClearSelection();
            return std::nullopt;
        }

        if (!CanMoveTo(GosChess::selected_square, square)) {
            // Clicking a different piece of ours picks that one up instead of
            // doing nothing, which is what the gesture plainly means.
            GosChess::selected_square = (player_may_move && HasALegalMove(square)) ? square : -1;
            return std::nullopt;
        }

        const GosChess::action_t move(static_cast<std::int8_t>(GosChess::selected_square),
                                      static_cast<std::int8_t>(square));
        ClearSelection();

        // A pawn reaching the far rank is four different moves. Rather than
        // quietly choosing a queen, the move is parked and the panel asks.
        if (GosChess::MoveNeedsPromotionChoice(move)) {
            GosChess::pending_promotion = move;
            return std::nullopt;
        }
        return move;
    }

}

int GosChess::selected_square = -1;

std::string GosChess::game_over_detail;

void GosChess::SubmitMove(GosChess::action_t move) {
    if (game_mode == GameMode::MULTI_PLAYER) {
        // Played locally first so the board answers the click at once. If the
        // server disagrees it sends back the real position and this is undone;
        // because both ends run the same rules over the same board, it does not.
        if (!PredictMove(move)) return;
        SendMove(move);
        return;
    }
    if (!PlayLocalMove(move)) return;
    SettleLocalGame();
}

void GosChess::ChoosePromotion(unsigned char piece) {
    if (!pending_promotion.has_value()) return;
    action_t move = pending_promotion.value();
    move.promotion = piece;
    pending_promotion = std::nullopt;
    SubmitMove(move);
}

bool GosChess::SettleLocalGame() {
    const GameStatus status = game.status();
    if (status == GameStatus::IN_PROGRESS) return false;
    if (status == GameStatus::CHECKMATE) {
        // Whoever is left to move is the side that has been mated.
        game_result = game.colorToPlay() == player_color ? GameResult::LOST : GameResult::WON;
    } else {
        game_result = GameResult::DRAW;
    }
    game_over_detail = Describe(status);
    SetGameFlagFinished();
    return true;
}

void GosChess::MultiPlayerListener::action() {
    // Nothing in here decides anything about the game. It reads the player's
    // intention, shows it immediately, and tells the server - which is the only
    // thing that can make it true.
    if (game_over) return;
    // One move may be in flight at a time. In chess that is automatic, since
    // the turn passes, but relying on that silently would be a trap for later.
    const bool may_move = PlayerToMove() && !awaiting_confirmation &&
                          !pending_promotion.has_value();
    const std::optional<action_t> move = PollPlayerMove(this->game_window, may_move);
    if (move.has_value()) SubmitMove(move.value());
}

void GosChess::GamePlayAIListener::action() {
    // The game against the computer is played entirely here, so this is the
    // one mode where the client really is the authority.
    if (SettleLocalGame()) return;
    const bool may_move = PlayerToMove() && !pending_promotion.has_value();
    const std::optional<action_t> move = PollPlayerMove(this->game_window, may_move);
    if (move.has_value()) SubmitMove(move.value());
}

void GosChess::MainMenuListener::action() {
    // The server can answer while the menu is still up - that is how being
    // paired ends the menu - so its messages are read here too.
    ProcessServerMessages();
}

void GosChess::SetGameFlagFinished() {
    SetGameOver(true);
}

void GosChess::ResetGame(sf::RenderWindow &window) {
    window.clear();
    LeaveServer();
    ResetClientGame();

    // The result is shown on the game screen now, so the menu goes back to
    // its front page rather than to a page that just repeats it.
    render_menu_flag = RenderMenuFLag::MAIN_MENU;
    game_mode = GameMode::None;
    ClearSelection();
    menu_active_flag = true;
}

GosChess::board_overlay_t GosChess::BuildOverlay(const sf::RenderWindow &window) {
    board_overlay_t overlay;
    overlay.last_move_from = last_move_from;
    overlay.last_move_to = last_move_to;

    const int shown_selection = pending_promotion.has_value()
                                ? pending_promotion.value().move_from : selected_square;
    if (IsSquare(shown_selection)) {
        overlay.selected_square = shown_selection;
        overlay.legal_moves = game.legalMovesFrom(shown_selection);
    }

    if (game.inCheck()) overlay.checked_king_square = KingSquare(game.board(), game.colorToPlay());

    const std::optional<int> hovered = SquareUnderPointer(window);
    if (hovered.has_value() && !ImGui::GetIO().WantCaptureMouse) {
        overlay.hovered_square = hovered.value();
    }
    return overlay;
}

std::string GosChess::StatusLine() {
    if (game_over) return ResultHeadline();
    if (pending_promotion.has_value()) return "Choose what the pawn becomes.";
    if (awaiting_confirmation) return "Sending your move...";
    if (PlayerToMove()) {
        return game.inCheck() ? "Your move - you are in check." : "Your move.";
    }
    return game.inCheck() ? "Your opponent is in check." : "Waiting for your opponent...";
}

std::string GosChess::ResultHeadline() {
    std::string headline;
    switch (game_result) {
        case GameResult::WON:  headline = "You won"; break;
        case GameResult::LOST: headline = "You lost"; break;
        case GameResult::DRAW: headline = "Draw"; break;
    }
    if (!game_over_detail.empty()) headline += " - " + game_over_detail;
    return headline + ".";
}

GosChess::GameResult GosChess::game_result = GosChess::GameResult::DRAW;

GosChess::GameMode GosChess::game_mode = GosChess::GameMode::None;

bool GosChess::menu_active_flag = true;
