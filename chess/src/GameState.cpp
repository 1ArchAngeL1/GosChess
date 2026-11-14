#include "GosChess/GameState.hh"

GosChess::GameState::GameState() : color_to_play_(color_t::WHITE), status_(GameStatus::IN_PROGRESS) {
    reset();
}

void GosChess::GameState::reset() {
    setPosition(INITIAL_FEN, color_t::WHITE);
}

void GosChess::GameState::setPosition(const std::string &fen, GosChess::color_t color_to_play) {
    board_.setState(fen);
    color_to_play_ = color_to_play;
    refresh();
}

bool GosChess::GameState::applyMove(GosChess::action_t move) {
    if (status_ != GameStatus::IN_PROGRESS) return false;
    if (move.move_from < 0 || move.move_from >= board_t::BOARD_SIZE ||
        move.move_to < 0 || move.move_to >= board_t::BOARD_SIZE) {
        return false;
    }
    // Only squares occupied by the side to play appear in the move map, so a
    // hit here already proves the mover owns the piece.
    const auto moves_from_square = available_moves_.find(move.move_from);
    if (moves_from_square == available_moves_.end()) return false;
    if (!MakeMove(move, board_, color_to_play_, moves_from_square->second)) return false;
    color_to_play_ = OppositeColor(color_to_play_);
    refresh();
    return true;
}

const GosChess::actions_set_t &GosChess::GameState::movesFrom(int square) const {
    static const actions_set_t empty;
    const auto found = available_moves_.find(static_cast<std::int8_t>(square));
    return found == available_moves_.end() ? empty : found->second;
}

void GosChess::GameState::refresh() {
    CalculateAvailableMoves(board_.getRawBoard(), color_to_play_, available_moves_);
    if (HasNoLegalMoves(board_, color_to_play_, available_moves_)) {
        status_ = IsKingInCheck(board_.getRawBoard(), color_to_play_) ? GameStatus::CHECKMATE
                                                                     : GameStatus::STALEMATE;
    } else {
        status_ = GameStatus::IN_PROGRESS;
    }
}
