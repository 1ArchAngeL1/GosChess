#include "GosChess/GameState.hh"

#include <algorithm>

#include "GosChess/Notation.hh"

namespace {

    // Fifty moves each way, counted in half-moves.
    constexpr std::uint16_t FIFTY_MOVE_PLIES = 100;

    constexpr int REPETITIONS_FOR_DRAW = 3;

}

const char *GosChess::Describe(GosChess::GameStatus status) {
    switch (status) {
        case GameStatus::IN_PROGRESS:                return "in progress";
        case GameStatus::CHECKMATE:                  return "checkmate";
        case GameStatus::STALEMATE:                  return "stalemate";
        case GameStatus::DRAW_FIFTY_MOVE:            return "draw by the fifty move rule";
        case GameStatus::DRAW_REPETITION:            return "draw by threefold repetition";
        case GameStatus::DRAW_INSUFFICIENT_MATERIAL: return "draw by insufficient material";
    }
    return "unknown";
}

GosChess::GameState::GameState() {
    reset();
}

void GosChess::GameState::reset() {
    setFen(INITIAL_FEN);
}

bool GosChess::GameState::setFen(const std::string &fen) {
    const std::optional<fen_t> parsed = ParseFen(fen);
    if (!parsed.has_value()) return false;
    rebuildFrom(parsed.value());
    return true;
}

void GosChess::GameState::rebuildFrom(const GosChess::fen_t &parsed) {
    board_.setPosition(parsed.position);
    color_to_play_ = parsed.color_to_play;
    fullmove_number_ = parsed.fullmove_number;
    ply_ = 0;
    last_move_ = action_t();
    last_move_notation_.clear();
    history_.clear();
    position_keys_.clear();
    // The starting position counts as the first occurrence of itself.
    position_keys_.push_back(board_.key(color_to_play_));
    refresh();
}

bool GosChess::GameState::applyMove(GosChess::action_t move) {
    if (status_ != GameStatus::IN_PROGRESS) return false;
    if (!IsSquare(move.move_from) || !IsSquare(move.move_to)) return false;
    // Only squares occupied by the side to play appear in the move map, so a
    // hit here already proves the mover owns the piece.
    const auto moves_from_square = available_moves_.find(move.move_from);
    if (moves_from_square == available_moves_.end()) return false;
    if (!CanMakeMove(move, moves_from_square->second)) return false;

    // The notation has to be worked out before the move is played, because it
    // describes the position the move was made in - which pieces could have
    // reached the square, and what stood on it.
    const std::string notation = ToSan(board_, color_to_play_, move);

    if (!MakeMove(move, board_, color_to_play_, moves_from_square->second)) return false;

    last_move_ = move;
    last_move_notation_ = notation;
    history_.push_back(notation);
    ply_++;
    if (color_to_play_ == color_t::BLACK) fullmove_number_++;
    color_to_play_ = OppositeColor(color_to_play_);
    position_keys_.push_back(board_.key(color_to_play_));
    refresh();
    return true;
}

const GosChess::actions_set_t &GosChess::GameState::movesFrom(int square) const {
    static const actions_set_t empty;
    const auto found = available_moves_.find(static_cast<std::int8_t>(square));
    return found == available_moves_.end() ? empty : found->second;
}

GosChess::actions_set_t GosChess::GameState::legalMovesFrom(int square) {
    actions_set_t legal;
    const actions_set_t &candidates = movesFrom(square);
    for (const action_t &move: candidates) {
        if (!MakeMove(move, board_, color_to_play_, candidates)) continue;
        UndoMove(board_);
        legal.insert(move);
    }
    return legal;
}

std::string GosChess::GameState::fen() const {
    return MakeFen(board_.position(), color_to_play_, fullmove_number_);
}

void GosChess::GameState::refresh() {
    CalculateAvailableMoves(board_, color_to_play_, available_moves_);
    in_check_ = IsKingInCheck(board_, color_to_play_);

    // Mate and stalemate come first: a game that has just been won is not a
    // draw, however long it has been since the last capture.
    if (HasNoLegalMoves(board_, color_to_play_, available_moves_)) {
        status_ = in_check_ ? GameStatus::CHECKMATE : GameStatus::STALEMATE;
        return;
    }
    if (IsInsufficientMaterial(board_)) {
        status_ = GameStatus::DRAW_INSUFFICIENT_MATERIAL;
        return;
    }
    if (board_.halfmoveClock() >= FIFTY_MOVE_PLIES) {
        status_ = GameStatus::DRAW_FIFTY_MOVE;
        return;
    }
    if (!position_keys_.empty()) {
        const std::uint64_t current = position_keys_.back();
        const auto occurrences = std::count(position_keys_.begin(), position_keys_.end(), current);
        if (occurrences >= REPETITIONS_FOR_DRAW) {
            status_ = GameStatus::DRAW_REPETITION;
            return;
        }
    }
    status_ = GameStatus::IN_PROGRESS;
}
