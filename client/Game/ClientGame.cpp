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
        return minute_text + ":" + second_text;
    }

    constexpr int PIECE_VALUE[8] = {0, 5, 3, 1, 3, 0, 9, 0};

    constexpr float ANIMATION_SECONDS = 0.16f;

    // Which ply the animation currently on screen belongs to. Without it, our
    // own move would animate twice: once when predicted and again when the
    // server confirms the same move back to us.
    std::uint32_t last_animated_ply = 0;

    int CountPieces(const GosChess::board_t &board) {
        int count = 0;
        const unsigned char *squares = board.getRawBoard();
        for (int square = 0; square < GosChess::board_t::BOARD_SIZE; square++) {
            if (squares[square] != 0) count++;
        }
        return count;
    }

    // Decided by comparing piece counts rather than by looking at the
    // destination square, because en passant takes a pawn that was never
    // standing where the capturer lands.
    void AnnounceMove(int pieces_before, int pieces_after, bool gives_check) {
        if (gives_check) {
            GosChess::pending_sound = GosChess::SoundCue::CHECK;
        } else if (pieces_after < pieces_before) {
            GosChess::pending_sound = GosChess::SoundCue::CAPTURE;
        } else {
            GosChess::pending_sound = GosChess::SoundCue::MOVE;
        }
    }

}

GosChess::GameState GosChess::game;

GosChess::color_t GosChess::player_color = GosChess::color_t::WHITE;

GosChess::color_t GosChess::enemy_color = GosChess::color_t::BLACK;

std::uint32_t GosChess::confirmed_ply = 0;

bool GosChess::awaiting_confirmation = false;

bool GosChess::game_over = false;

std::int8_t GosChess::last_move_from = -1;

std::int8_t GosChess::last_move_to = -1;

std::uint16_t GosChess::time_limit_minutes = 0;

std::optional<GosChess::action_t> GosChess::pending_promotion = std::nullopt;

GosChess::move_animation_t GosChess::move_animation;

GosChess::SoundCue GosChess::pending_sound = GosChess::SoundCue::NONE;

void GosChess::SetGameOver(bool over) {
    const bool newly_over = over && !GosChess::game_over;
    GosChess::game_over = over;
    if (newly_over) pending_sound = SoundCue::GAME_OVER;
}

void GosChess::StartMoveAnimation(int from, int to) {
    if (!IsSquare(from) || !IsSquare(to)) return;
    move_animation = move_animation_t();
    move_animation.from = from;
    move_animation.to = to;
    move_animation.piece = game.board().getRawBoard()[to];
    move_animation.duration = ANIMATION_SECONDS;
    move_animation.active = true;

    // The king has already landed, so a two file step is what identifies the
    // move as a castle after the fact.
    const figure_t mover(move_animation.piece);
    if (mover.type_ == figure_types_t::KING && std::abs(FileOf(to) - FileOf(from)) == 2) {
        const int home_rank = RankOf(to) * board_t::ROW_LENGTH;
        const bool king_side = FileOf(to) > FileOf(from);
        move_animation.rook_from = home_rank + (king_side ? 7 : 0);
        move_animation.rook_to = home_rank + (king_side ? 5 : 3);
        move_animation.rook_piece = game.board().getRawBoard()[move_animation.rook_to];
    }
}

void GosChess::TickMoveAnimation(float delta_seconds) {
    if (!move_animation.active) return;
    move_animation.elapsed += delta_seconds;
    if (move_animation.elapsed >= move_animation.duration) move_animation.active = false;
}

float GosChess::MoveAnimationProgress() {
    if (!move_animation.active || move_animation.duration <= 0.f) return 1.f;
    const float t = std::clamp(move_animation.elapsed / move_animation.duration, 0.f, 1.f);
    // Eased out, so the piece arrives softly instead of stopping dead.
    return 1.f - (1.f - t) * (1.f - t) * (1.f - t);
}

void GosChess::SetPlayerColor(GosChess::color_t color) {
    GosChess::player_color = color;
    GosChess::enemy_color = GosChess::OppositeColor(color);
}

bool GosChess::PlayerToMove() {
    return GosChess::game.colorToPlay() == GosChess::player_color;
}

void GosChess::AdoptServerState(const GosChess::Protocol::GameStateUpdate &update) {
    const int pieces_before = CountPieces(game.board());
    game.setFen(update.fen);
    // The server's record replaces ours rather than being merged into it, so a
    // refused move cannot leave a phantom entry behind.
    game.replaceHistory(update.history);
    confirmed_ply = update.ply;
    awaiting_confirmation = false;
    pending_promotion = std::nullopt;
    last_move_from = update.last_move_from;
    last_move_to = update.last_move_to;

    // Only a position we have not already shown is worth animating: adopting
    // the confirmation of our own predicted move must not replay it.
    if (update.ply != last_animated_ply && IsSquare(update.last_move_from)) {
        StartMoveAnimation(update.last_move_from, update.last_move_to);
        AnnounceMove(pieces_before, CountPieces(game.board()), update.in_check);
        last_animated_ply = update.ply;
    }
    display_white_millis = static_cast<float>(update.white_millis_left);
    display_black_millis = static_cast<float>(update.black_millis_left);
}

bool GosChess::PredictMove(GosChess::action_t move) {
    const int pieces_before = CountPieces(game.board());
    if (!game.applyMove(move)) return false;
    awaiting_confirmation = true;
    last_move_from = move.move_from;
    last_move_to = move.move_to;
    StartMoveAnimation(move.move_from, move.move_to);
    AnnounceMove(pieces_before, CountPieces(game.board()), game.inCheck());
    // The ply this move will become once the server agrees.
    last_animated_ply = confirmed_ply + 1;
    return true;
}

bool GosChess::PlayLocalMove(GosChess::action_t move) {
    const int pieces_before = CountPieces(game.board());
    if (!game.applyMove(move)) return false;
    last_move_from = move.move_from;
    last_move_to = move.move_to;
    StartMoveAnimation(move.move_from, move.move_to);
    AnnounceMove(pieces_before, CountPieces(game.board()), game.inCheck());
    return true;
}

void GosChess::ResetClientGame() {
    game.reset();
    game_over = false;
    confirmed_ply = 0;
    awaiting_confirmation = false;
    pending_promotion = std::nullopt;
    last_move_from = last_move_to = -1;
    move_animation = move_animation_t();
    last_animated_ply = 0;
    pending_sound = SoundCue::NONE;
    time_limit_minutes = 0;
    display_white_millis = display_black_millis = 0.f;
}

std::vector<GosChess::history_row_t> GosChess::HistoryRows() {
    std::vector<history_row_t> rows;
    const std::vector<std::string> &moves = game.history();
    for (std::size_t i = 0; i < moves.size(); i += 2) {
        history_row_t row;
        row.move_number = static_cast<int>(i / 2) + 1;
        row.white = moves[i];
        if (i + 1 < moves.size()) row.black = moves[i + 1];
        rows.push_back(row);
    }
    return rows;
}

bool GosChess::MoveNeedsPromotionChoice(GosChess::action_t move) {
    if (!IsSquare(move.move_from) || !IsSquare(move.move_to)) return false;
    const figure_t mover = game.board().getFigure(move.move_from);
    if (mover.full_type == 0 || mover.type_ != figure_types_t::PAWN) return false;
    const int last_rank = mover.color_ == color_t::WHITE ? board_t::ROW_NUM - 1 : 0;
    if (RankOf(move.move_to) != last_rank) return false;
    // Only worth asking if at least one of the four is actually legal.
    const actions_set_t legal = game.legalMovesFrom(move.move_from);
    for (const action_t &candidate: legal) {
        if (candidate.move_to == move.move_to && candidate.isPromotion()) return true;
    }
    return false;
}

void GosChess::TickClockDisplay(float delta_seconds) {
    if (!GameHasClock()) return;
    // Stops for a resignation or a flag fall too, not only for a mate.
    if (game_over || game.finished()) return;
    float &running = game.colorToPlay() == color_t::WHITE ? display_white_millis
                                                          : display_black_millis;
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

bool GosChess::PlayerClockRunning() {
    return PlayerToMove() && !game_over && !game.finished();
}

int GosChess::MaterialBalanceForPlayer() {
    int balance = 0;
    const unsigned char *squares = game.board().getRawBoard();
    for (int square = 0; square < board_t::BOARD_SIZE; square++) {
        if (squares[square] == 0) continue;
        const figure_t piece(squares[square]);
        const int value = PIECE_VALUE[piece.type_];
        balance += piece.color_ == player_color ? value : -value;
    }
    return balance;
}
