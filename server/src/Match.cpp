#include "Match.hh"

namespace {

    constexpr std::uint32_t MILLIS_PER_MINUTE = 60u * 1000u;

}

GosChess::Match::Match(std::uint32_t id, std::uint16_t time_limit_minutes)
        : id_(id),
          time_limit_minutes_(time_limit_minutes),
          has_clock_(time_limit_minutes > 0) {
    white_millis_ = black_millis_ = time_limit_minutes * MILLIS_PER_MINUTE;
}

bool GosChess::Match::setFen(const std::string &fen) {
    if (!game_.setFen(fen)) return false;
    finished_ = false;
    finishIfPositionIsTerminal();
    return true;
}

GosChess::Match::MoveResult GosChess::Match::applyMove(GosChess::color_t mover,
                                                       const GosChess::Protocol::MoveRequest &request) {
    MoveResult result;
    if (finished_) {
        result.reject_reason = Protocol::RejectReason::GAME_FINISHED;
        return result;
    }
    if (mover != game_.colorToPlay()) {
        result.reject_reason = Protocol::RejectReason::NOT_YOUR_TURN;
        return result;
    }
    // Checked before the move itself: a stale ply means the client is talking
    // about a position that no longer exists, and the move might well be legal
    // in the current one by coincidence.
    if (request.ply != game_.ply()) {
        result.reject_reason = Protocol::RejectReason::STALE_PLY;
        return result;
    }
    if (!game_.applyMove(action_t(request.move_from, request.move_to, request.promotion))) {
        result.reject_reason = Protocol::RejectReason::ILLEGAL_MOVE;
        return result;
    }
    finishIfPositionIsTerminal();
    result.accepted = true;
    return result;
}

bool GosChess::Match::chargeTime(std::uint32_t elapsed_millis) {
    if (finished_ || !has_clock_) return false;
    std::uint32_t &clock = clockFor(game_.colorToPlay());
    if (elapsed_millis >= clock) {
        clock = 0;
        finish(Protocol::OutcomeForLosingSide(game_.colorToPlay()), Protocol::GameOverReason::TIMEOUT);
        return true;
    }
    clock -= elapsed_millis;
    return false;
}

void GosChess::Match::resign(GosChess::color_t who) {
    if (finished_) return;
    finish(Protocol::OutcomeForLosingSide(who), Protocol::GameOverReason::RESIGNATION);
}

void GosChess::Match::abandon(GosChess::color_t who_left) {
    if (finished_) return;
    finish(Protocol::OutcomeForLosingSide(who_left), Protocol::GameOverReason::OPPONENT_LEFT);
}

GosChess::Protocol::GameStateUpdate GosChess::Match::state() const {
    Protocol::GameStateUpdate update;
    update.ply = game_.ply();
    update.fen = game_.fen();
    update.color_to_play = game_.colorToPlay();
    update.last_move_from = game_.lastMove().move_from;
    update.last_move_to = game_.lastMove().move_to;
    update.in_check = game_.inCheck();
    update.white_millis_left = white_millis_;
    update.black_millis_left = black_millis_;
    update.history = game_.history();
    return update;
}

GosChess::Protocol::GameOver GosChess::Match::result() const {
    Protocol::GameOver over;
    over.outcome = outcome_;
    over.reason = reason_;
    return over;
}

void GosChess::Match::finish(GosChess::Protocol::GameOutcome outcome,
                             GosChess::Protocol::GameOverReason reason) {
    finished_ = true;
    outcome_ = outcome;
    reason_ = reason;
}

void GosChess::Match::finishIfPositionIsTerminal() {
    switch (game_.status()) {
        case GameStatus::CHECKMATE:
            // The side left to move is the side that has been mated.
            finish(Protocol::OutcomeForLosingSide(game_.colorToPlay()),
                   Protocol::GameOverReason::CHECKMATE);
            break;
        case GameStatus::STALEMATE:
            finish(Protocol::GameOutcome::DRAW, Protocol::GameOverReason::STALEMATE);
            break;
        // A game can end without either player resigning or being mated, and
        // the players are told which of the three it was.
        case GameStatus::DRAW_FIFTY_MOVE:
            finish(Protocol::GameOutcome::DRAW, Protocol::GameOverReason::FIFTY_MOVE_RULE);
            break;
        case GameStatus::DRAW_REPETITION:
            finish(Protocol::GameOutcome::DRAW, Protocol::GameOverReason::THREEFOLD_REPETITION);
            break;
        case GameStatus::DRAW_INSUFFICIENT_MATERIAL:
            finish(Protocol::GameOutcome::DRAW, Protocol::GameOverReason::INSUFFICIENT_MATERIAL);
            break;
        case GameStatus::IN_PROGRESS:
            break;
    }
}

std::uint32_t &GosChess::Match::clockFor(GosChess::color_t color) {
    return color == color_t::WHITE ? white_millis_ : black_millis_;
}
