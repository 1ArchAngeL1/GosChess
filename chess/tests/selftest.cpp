// Behaviour checks for the rules engine.
//
// perft.cpp proves the move generator counts correctly; this proves it means
// the right things. Node counts cannot tell you that a promotion honoured the
// piece the player asked for, that the rook ended up on the right square after
// castling, or that a game drew itself for the stated reason.

#include <cstdio>
#include <string>
#include <vector>

#include "GosChess/Board.hh"
#include "GosChess/GameState.hh"
#include "GosChess/Notation.hh"
#include "GosChess/Rules.hh"

namespace {

    using namespace GosChess;

    int failures = 0;

    void Check(bool condition, const std::string &what) {
        if (condition) return;
        std::printf("  FAIL  %s\n", what.c_str());
        failures++;
    }

    void CheckEqual(const std::string &got, const std::string &want, const std::string &what) {
        if (got == want) return;
        std::printf("  FAIL  %s: got \"%s\", wanted \"%s\"\n", what.c_str(), got.c_str(), want.c_str());
        failures++;
    }

    // "e1" -> 4, "a1" -> 0, "h8" -> 63.
    constexpr std::int8_t Sq(char file, int rank) {
        return static_cast<std::int8_t>((rank - 1) * board_t::ROW_LENGTH + (file - 'a'));
    }

    bool Play(GameState &game, char from_file, int from_rank, char to_file, int to_rank,
              unsigned char promotion = 0) {
        return game.applyMove(action_t(Sq(from_file, from_rank), Sq(to_file, to_rank), promotion));
    }

    std::size_t CountMoves(const action_map_t &moves) {
        std::size_t total = 0;
        for (const auto &[square, from_square]: moves) total += from_square.size();
        return total;
    }

    bool Has(GameState &game, char from_file, int from_rank, char to_file, int to_rank,
             unsigned char promotion = 0) {
        const actions_set_t legal = game.legalMovesFrom(Sq(from_file, from_rank));
        return legal.count(action_t(Sq(from_file, from_rank), Sq(to_file, to_rank), promotion)) != 0;
    }

    unsigned char PieceAt(const GameState &game, char file, int rank) {
        return game.board().getFigure(Sq(file, rank)).type_;
    }

    bool IsEmpty(const GameState &game, char file, int rank) {
        return game.board().getRawBoard()[Sq(file, rank)] == 0;
    }

    void TestOpeningPosition() {
        std::printf("opening position\n");
        GameState game;

        Check(game.colorToPlay() == color_t::WHITE, "white starts");
        Check(game.status() == GameStatus::IN_PROGRESS, "the game is in progress");
        Check(!game.inCheck(), "nobody is in check");
        Check(CountMoves(game.availableMoves()) == 20, "white has 20 opening moves");
        Check(game.ply() == 0, "no moves have been played");
        Check(game.fullmoveNumber() == 1, "it is move one");
        Check(game.history().empty(), "the history is empty");

        Check(PieceAt(game, 'e', 1) == figure_types_t::KING, "white king on e1");
        Check(PieceAt(game, 'e', 8) == figure_types_t::KING, "black king on e8");
        Check(game.board().castlingRights() == ALL_CASTLING, "both sides may castle either way");
        CheckEqual(game.fen(), INITIAL_FEN, "the opening position round trips through FEN");
    }

    void TestFenRoundTrip() {
        std::printf("fen\n");
        // Every field: an en passant square, partial castling rights and a
        // halfmove clock that is not zero.
        const std::string fen = "r3k2r/pp3ppp/8/3pP3/8/8/PP3PPP/R3K2R w Kq d6 7 23";
        GameState game;
        Check(game.setFen(fen), "a full FEN is accepted");
        CheckEqual(game.fen(), fen, "and comes back out unchanged");
        Check(game.board().enPassantTarget() == Sq('d', 6), "the en passant square survives");
        Check(game.board().castlingRights() == (WHITE_KING_SIDE | BLACK_QUEEN_SIDE),
              "partial castling rights survive");
        Check(game.board().halfmoveClock() == 7, "the halfmove clock survives");
        Check(game.fullmoveNumber() == 23, "the move number survives");

        GameState untouched;
        Check(!untouched.setFen("not a position"), "nonsense is refused");
        CheckEqual(untouched.fen(), INITIAL_FEN, "and leaves the game alone");
    }

    // The orientation check that matters: both colours' pawns must run the
    // right way on one and the same board.
    void TestPawnDirections() {
        std::printf("pawn directions\n");
        GameState game;

        Check(Has(game, 'a', 2, 'a', 3), "white pawn pushes one up");
        Check(Has(game, 'a', 2, 'a', 4), "white pawn pushes two up");
        Check(!Has(game, 'a', 2, 'a', 1), "white pawn never moves down");

        Check(Play(game, 'a', 2, 'a', 3), "white opens a3");
        Check(game.colorToPlay() == color_t::BLACK, "the turn passes to black");
        Check(CountMoves(game.availableMoves()) == 20, "black has 20 opening moves");

        Check(Has(game, 'a', 7, 'a', 6), "black pawn pushes one down");
        Check(Has(game, 'a', 7, 'a', 5), "black pawn pushes two down");
        Check(!Has(game, 'a', 7, 'a', 8), "black pawn never moves up");
    }

    void TestOnlyYourOwnLegalMoves() {
        std::printf("refusing moves\n");
        GameState game;

        // The property the old peer to peer build lacked entirely: a move that
        // is not yours is refused instead of being replayed on trust.
        Check(!Play(game, 'a', 7, 'a', 6), "white cannot move a black pawn");
        Check(!Play(game, 'a', 1, 'a', 5), "a rook cannot pass through its own pawn");
        Check(!Play(game, 'd', 4, 'd', 5), "an empty square cannot move");
        Check(!game.applyMove(action_t(100, 3)), "a square off the board is refused");
        Check(!game.applyMove(action_t()), "a move that was never filled in is refused");
        CheckEqual(game.fen(), INITIAL_FEN, "none of that changed the position");
        Check(game.colorToPlay() == color_t::WHITE, "nor passed the turn");
    }

    void TestCastlingMovesBothPieces() {
        std::printf("castling\n");
        GameState game;
        Check(game.setFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"), "bare castling position");

        Check(Has(game, 'e', 1, 'g', 1), "white may castle king side");
        Check(Has(game, 'e', 1, 'c', 1), "white may castle queen side");

        Check(Play(game, 'e', 1, 'g', 1), "white castles king side");
        Check(PieceAt(game, 'g', 1) == figure_types_t::KING, "the king lands on g1");
        // The move the player makes is the king's; the rook coming with it is
        // the part a client cannot be trusted to work out for itself.
        Check(PieceAt(game, 'f', 1) == figure_types_t::ROOK, "the rook jumps to f1");
        Check(IsEmpty(game, 'e', 1) && IsEmpty(game, 'h', 1), "and both home squares are empty");
        Check((game.board().castlingRights() & (WHITE_KING_SIDE | WHITE_QUEEN_SIDE)) == 0,
              "white has no castling rights left");
        Check((game.board().castlingRights() & (BLACK_KING_SIDE | BLACK_QUEEN_SIDE)) != 0,
              "black's are untouched");

        Check(Play(game, 'e', 8, 'c', 8), "black castles queen side");
        Check(PieceAt(game, 'c', 8) == figure_types_t::KING, "the king lands on c8");
        Check(PieceAt(game, 'd', 8) == figure_types_t::ROOK, "the rook jumps to d8");
        Check(IsEmpty(game, 'e', 8) && IsEmpty(game, 'a', 8), "and both home squares are empty");
    }

    void TestCastlingRightsAreLost() {
        std::printf("castling rights\n");

        GameState moved_king;
        moved_king.setFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
        Check(Play(moved_king, 'e', 1, 'e', 2), "the white king steps out");
        Check(Play(moved_king, 'a', 8, 'b', 8), "black moves a rook");
        Check(Play(moved_king, 'e', 2, 'e', 1), "the white king steps back");
        Check(Play(moved_king, 'b', 8, 'a', 8), "and black's rook returns too");
        Check(moved_king.colorToPlay() == color_t::WHITE, "leaving white to move");
        Check(!Has(moved_king, 'e', 1, 'g', 1), "a king that has moved may not castle again");
        Check(!Has(moved_king, 'e', 1, 'c', 1), "either way");

        GameState moved_rook;
        moved_rook.setFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
        Check(Play(moved_rook, 'h', 1, 'h', 2), "the king side rook steps out");
        Check(Play(moved_rook, 'a', 8, 'b', 8), "black moves a rook");
        Check(Play(moved_rook, 'h', 2, 'h', 1), "the rook steps back");
        Check(Play(moved_rook, 'b', 8, 'a', 8), "and black's rook returns too");
        Check(moved_rook.colorToPlay() == color_t::WHITE, "leaving white to move");
        Check(!Has(moved_rook, 'e', 1, 'g', 1), "that side is gone for good");
        Check(Has(moved_rook, 'e', 1, 'c', 1), "the other side still stands");

        GameState captured_rook;
        captured_rook.setFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
        Check(Play(captured_rook, 'a', 1, 'a', 8), "white takes the black queen side rook");
        Check((captured_rook.board().castlingRights() & BLACK_QUEEN_SIDE) == 0,
              "black loses that side without having moved anything");
        Check((captured_rook.board().castlingRights() & BLACK_KING_SIDE) != 0,
              "but keeps the other");
    }

    void TestCastlingThroughAndIntoCheck() {
        std::printf("castling and check\n");

        GameState out_of_check;
        out_of_check.setFen("k3r3/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        Check(out_of_check.inCheck(), "the king is in check down the e file");
        Check(!Has(out_of_check, 'e', 1, 'g', 1), "a king in check may not castle out of it");
        Check(!Has(out_of_check, 'e', 1, 'c', 1), "either way");

        GameState through_check;
        through_check.setFen("k4r2/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        Check(!through_check.inCheck(), "the king is not in check");
        Check(!Has(through_check, 'e', 1, 'g', 1),
              "but may not castle across an attacked f1");
        Check(Has(through_check, 'e', 1, 'c', 1), "the other side is unaffected");

        GameState into_check;
        into_check.setFen("k5r1/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        Check(!Has(into_check, 'e', 1, 'g', 1), "nor onto an attacked g1");
        Check(Has(into_check, 'e', 1, 'c', 1), "the other side is unaffected");

        // b1 is the one square on the queen side the king never stands on, so
        // it has to be empty but need not be safe.
        GameState attacked_b_file;
        attacked_b_file.setFen("k7/1r6/8/8/8/8/8/R3K2R w KQ - 0 1");
        Check(Has(attacked_b_file, 'e', 1, 'c', 1),
              "an attacked b1 does not prevent queen side castling");

        GameState blocked;
        blocked.setFen("k7/8/8/8/8/8/8/RN2K2R w KQ - 0 1");
        Check(!Has(blocked, 'e', 1, 'c', 1), "a piece on b1 does prevent it");
    }

    void TestEnPassant() {
        std::printf("en passant\n");
        GameState game;
        Check(game.setFen("4k3/8/8/8/4p3/8/3P4/4K3 w - - 0 1"), "pawns set up for en passant");

        Check(game.board().enPassantTarget() == -1, "no en passant square to begin with");
        Check(Play(game, 'd', 2, 'd', 4), "white pushes two squares past the black pawn");
        Check(game.board().enPassantTarget() == Sq('d', 3), "which leaves an en passant square");

        Check(Has(game, 'e', 4, 'd', 3), "the black pawn may capture onto the empty square");
        Check(Play(game, 'e', 4, 'd', 3), "and does");
        Check(PieceAt(game, 'd', 3) == figure_types_t::PAWN, "the capturer lands on d3");
        // The captured pawn is on a square the capturer never visits, which is
        // the whole reason this needs handling of its own.
        Check(IsEmpty(game, 'd', 4), "and the pawn it took is gone from d4");
        Check(game.board().enPassantTarget() == -1, "the opportunity is spent");
    }

    void TestEnPassantExpires() {
        std::printf("en passant expires\n");
        GameState game;
        game.setFen("4k3/7p/8/8/4p3/8/3P4/4K3 w - - 0 1");
        Check(Play(game, 'd', 2, 'd', 4), "white pushes two squares");
        Check(game.board().enPassantTarget() == Sq('d', 3), "the square is there");
        Check(Play(game, 'h', 7, 'h', 6), "black plays elsewhere instead");
        Check(game.board().enPassantTarget() == -1, "the square is gone");
        Check(Play(game, 'e', 1, 'd', 1), "white waits");
        Check(!Has(game, 'e', 4, 'd', 3), "and the capture is no longer on offer");
    }

    void TestPromotionHonoursTheChoice() {
        std::printf("promotion\n");

        // Four different moves to the same square, and the player's pick has
        // to be the one that happens.
        const struct {
            unsigned char piece;
            const char *name;
        } choices[] = {{figure_types_t::QUEEN,  "queen"},
                       {figure_types_t::ROOK,   "rook"},
                       {figure_types_t::BISHOP, "bishop"},
                       {figure_types_t::KNIGHT, "knight"}};

        for (const auto &choice: choices) {
            GameState game;
            game.setFen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
            Check(Has(game, 'a', 7, 'a', 8, choice.piece),
                  std::string("promoting to a ") + choice.name + " is offered");
            Check(Play(game, 'a', 7, 'a', 8, choice.piece),
                  std::string("promoting to a ") + choice.name + " is played");
            Check(PieceAt(game, 'a', 8) == choice.piece,
                  std::string("and a ") + choice.name + " is what appears on a8");
            Check(game.board().getFigure(Sq('a', 8)).color_ == color_t::WHITE,
                  "in the right colour");
        }

        GameState game;
        game.setFen("4k3/P7/8/8/8/8/8/4K3 w - - 0 1");
        Check(game.legalMovesFrom(Sq('a', 7)).size() == 4, "a promotion is exactly four moves");
        Check(!Play(game, 'a', 7, 'a', 8), "and one without a choice is not one of them");

        GameState black_game;
        black_game.setFen("4k3/8/8/8/8/8/p7/4K3 b - - 0 1");
        Check(Play(black_game, 'a', 2, 'a', 1, figure_types_t::KNIGHT), "black promotes too");
        Check(PieceAt(black_game, 'a', 1) == figure_types_t::KNIGHT, "to a knight");
        Check(black_game.board().getFigure(Sq('a', 1)).color_ == color_t::BLACK, "and stays black");
    }

    void TestCheckmate() {
        std::printf("checkmate\n");
        GameState game;
        Check(Play(game, 'f', 2, 'f', 3), "1. f3");
        Check(Play(game, 'e', 7, 'e', 5), "1... e5");
        Check(Play(game, 'g', 2, 'g', 4), "2. g4");
        Check(game.status() == GameStatus::IN_PROGRESS, "not over before the mate");
        Check(Play(game, 'd', 8, 'h', 4), "2... Qh4#");

        Check(game.status() == GameStatus::CHECKMATE, "white is mated");
        Check(game.colorToPlay() == color_t::WHITE, "white is the side to play when mated");
        Check(game.inCheck(), "and is in check");
        Check(!Play(game, 'g', 1, 'h', 3), "no move is accepted after mate");
        CheckEqual(game.lastMoveNotation(), "Qh4#", "the mate is written with a hash");
    }

    void TestCheckMustBeAnswered() {
        std::printf("check evasion\n");
        GameState game;
        Check(Play(game, 'f', 2, 'f', 3), "1. f3");
        Check(Play(game, 'e', 7, 'e', 5), "1... e5");
        Check(Play(game, 'a', 2, 'a', 3), "2. a3");
        Check(Play(game, 'd', 8, 'h', 4), "2... Qh4+");

        Check(game.inCheck(), "white is in check");
        Check(game.status() == GameStatus::IN_PROGRESS, "check is not mate");
        Check(!Play(game, 'b', 2, 'b', 3), "a move that ignores the check is refused");
        Check(Play(game, 'g', 2, 'g', 3), "g3 interposes");
        Check(!game.inCheck(), "and the king is safe again");
    }

    void TestStalemate() {
        std::printf("stalemate\n");
        GameState game;
        // Black king a8, white queen b6, white king c1 - a8, a7, b7 and b8 are
        // all covered by the queen, so black has no move and is not in check.
        Check(game.setFen("k7/8/1Q6/8/8/8/8/2K5 b - - 0 1"), "stalemate position");
        Check(game.status() == GameStatus::STALEMATE, "black is stalemated");
        Check(IsDraw(game.status()), "which is a draw");
        Check(!game.inCheck(), "a stalemated king is not in check");
    }

    void TestFiftyMoveRule() {
        std::printf("the fifty move rule\n");
        GameState game;
        // Ninety-nine half-moves already gone, and a rook on the board so this
        // is not decided by insufficient material instead.
        game.setFen("4k3/8/8/8/8/8/8/R3K3 w - - 99 60");
        Check(game.status() == GameStatus::IN_PROGRESS, "ninety-nine is not yet fifty moves");
        Check(Play(game, 'a', 1, 'a', 2), "a quiet rook move");
        Check(game.board().halfmoveClock() == 100, "takes the clock to a hundred half-moves");
        Check(game.status() == GameStatus::DRAW_FIFTY_MOVE, "and the game is drawn");

        GameState reset_by_capture;
        reset_by_capture.setFen("4k3/8/8/8/8/8/p7/R3K3 w - - 99 60");
        Check(Play(reset_by_capture, 'a', 1, 'a', 2), "a capture instead");
        Check(reset_by_capture.board().halfmoveClock() == 0, "puts the clock back to zero");
        Check(reset_by_capture.status() == GameStatus::IN_PROGRESS, "and the game goes on");
    }

    void TestThreefoldRepetition() {
        std::printf("threefold repetition\n");
        GameState game;
        // Knights out and back twice over. Nothing else changes, so the
        // opening position occurs a third time.
        for (int round = 0; round < 2; round++) {
            Check(Play(game, 'g', 1, 'f', 3), "Nf3");
            Check(Play(game, 'g', 8, 'f', 6), "Nf6");
            Check(Play(game, 'f', 3, 'g', 1), "Ng1");
            Check(Play(game, 'f', 6, 'g', 8), "Ng8");
            if (round == 0) {
                Check(game.status() == GameStatus::IN_PROGRESS,
                      "a second occurrence is not yet a draw");
            }
        }
        Check(game.status() == GameStatus::DRAW_REPETITION, "the third occurrence draws");
        Check(IsDraw(game.status()), "which is a draw");
    }

    void TestInsufficientMaterial() {
        std::printf("insufficient material\n");
        const struct {
            const char *fen;
            bool drawn;
            const char *description;
        } positions[] = {
                {"4k3/8/8/8/8/8/8/4K3 w - - 0 1",   true,  "king against king"},
                {"4k3/8/8/8/8/8/8/3BK3 w - - 0 1",  true,  "a lone bishop cannot mate"},
                {"4k3/8/8/8/8/8/8/3NK3 w - - 0 1",  true,  "nor a lone knight"},
                {"3bk3/8/8/8/8/8/8/3BK3 w - - 0 1", true,  "nor one each"},
                {"4k3/8/8/8/8/8/8/3RK3 w - - 0 1",  false, "a rook can"},
                {"4k3/8/8/8/8/8/P7/4K3 w - - 0 1",  false, "and so can a pawn, eventually"},
                {"4k3/8/8/8/8/8/8/2NNK3 w - - 0 1", false, "two knights are not hopeless"},
        };
        for (const auto &position: positions) {
            GameState game;
            Check(game.setFen(position.fen), std::string("set up: ") + position.description);
            const bool drawn = game.status() == GameStatus::DRAW_INSUFFICIENT_MATERIAL;
            Check(drawn == position.drawn, position.description);
        }
    }

    void TestNotation() {
        std::printf("algebraic notation\n");
        GameState game;
        Check(Play(game, 'e', 2, 'e', 4), "1. e4");
        CheckEqual(game.lastMoveNotation(), "e4", "a quiet pawn move is just its square");
        Check(Play(game, 'd', 7, 'd', 5), "1... d5");
        Check(Play(game, 'e', 4, 'd', 5), "2. exd5");
        CheckEqual(game.lastMoveNotation(), "exd5", "a pawn capture names the file it came from");
        Check(Play(game, 'd', 8, 'd', 5), "2... Qxd5");
        CheckEqual(game.lastMoveNotation(), "Qxd5", "a piece capture uses its letter");
        Check(Play(game, 'g', 1, 'f', 3), "3. Nf3");
        CheckEqual(game.lastMoveNotation(), "Nf3", "a quiet piece move names the square");

        Check(game.history().size() == 5, "every move is recorded");
        CheckEqual(game.history()[0], "e4", "in order");

        GameState castling;
        castling.setFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
        Check(Play(castling, 'e', 1, 'g', 1), "white castles short");
        CheckEqual(castling.lastMoveNotation(), "O-O", "which is written O-O");
        Check(Play(castling, 'e', 8, 'c', 8), "black castles long");
        CheckEqual(castling.lastMoveNotation(), "O-O-O", "which is written O-O-O");

        GameState promotion;
        // The king is kept off the eighth rank so that the promotion is not
        // also a check, which would earn it a suffix.
        promotion.setFen("8/P7/4k3/8/8/8/8/4K3 w - - 0 1");
        Check(Play(promotion, 'a', 7, 'a', 8, figure_types_t::ROOK), "a rook promotion");
        CheckEqual(promotion.lastMoveNotation(), "a8=R", "names the piece chosen");

        // Two knights that can both reach the square: the notation has to say
        // which one moved, or the move cannot be replayed from the record.
        GameState two_knights;
        two_knights.setFen("4k3/8/8/8/8/2N1N3/8/4K3 w - - 0 1");
        Check(Play(two_knights, 'c', 3, 'd', 5), "the c3 knight moves");
        CheckEqual(two_knights.lastMoveNotation(), "Ncd5", "the file tells them apart");

        GameState same_file;
        same_file.setFen("7k/2N5/8/8/8/2N5/8/4K3 w - - 0 1");
        Check(Play(same_file, 'c', 3, 'd', 5), "the lower of two knights on the c file moves");
        CheckEqual(same_file.lastMoveNotation(), "N3d5",
                   "when they share a file the rank tells them apart");

        GameState checking;
        checking.setFen("4k3/8/8/8/8/8/8/R3K3 w - - 0 1");
        Check(Play(checking, 'a', 1, 'a', 8), "a rook gives check");
        CheckEqual(checking.lastMoveNotation(), "Ra8+", "which is marked with a plus");
    }

    void TestCoordinateNotation() {
        std::printf("coordinate notation\n");
        CheckEqual(ToCoordinateNotation(action_t(Sq('e', 2), Sq('e', 4))), "e2e4", "a plain move");
        CheckEqual(ToCoordinateNotation(action_t(Sq('a', 7), Sq('a', 8), figure_types_t::KNIGHT)),
                   "a7a8n", "a promotion names the piece");
    }

    // Two games in one process must not influence each other. This is the
    // whole point of the state object, and what a server depends on.
    void TestGamesAreIndependent() {
        std::printf("game independence\n");
        GameState first;
        GameState second;

        Check(Play(first, 'e', 2, 'e', 4), "the first game plays e4");
        Check(first.colorToPlay() == color_t::BLACK, "the first game is on black");
        Check(second.colorToPlay() == color_t::WHITE, "the second game is still on white");
        CheckEqual(second.fen(), INITIAL_FEN, "and holds the opening position");
        Check(second.history().empty(), "with no history of its own");
        Check(Play(second, 'e', 2, 'e', 4), "the second game plays e4 too");
    }

    void TestLegalMovesAreWhatIsOffered() {
        std::printf("offered moves\n");
        // A pinned piece generates moves that cannot be played. A board that
        // highlighted them would be offering moves the server will refuse.
        GameState game;
        Check(game.setFen("4k3/4r3/8/8/8/8/4R3/4K3 w - - 0 1"), "a rook shielding its king");
        Check(!game.legalMovesFrom(Sq('e', 2)).empty(), "the rook can move along the file");
        for (const action_t &move: game.legalMovesFrom(Sq('e', 2))) {
            Check(FileOf(move.move_to) == FileOf(Sq('e', 2)),
                  "a piece shielding its king may only move along the line of the attack");
        }
    }

}

int main() {
    TestOpeningPosition();
    TestFenRoundTrip();
    TestPawnDirections();
    TestOnlyYourOwnLegalMoves();
    TestCastlingMovesBothPieces();
    TestCastlingRightsAreLost();
    TestCastlingThroughAndIntoCheck();
    TestEnPassant();
    TestEnPassantExpires();
    TestPromotionHonoursTheChoice();
    TestCheckmate();
    TestCheckMustBeAnswered();
    TestStalemate();
    TestFiftyMoveRule();
    TestThreefoldRepetition();
    TestInsufficientMaterial();
    TestNotation();
    TestCoordinateNotation();
    TestGamesAreIndependent();
    TestLegalMovesAreWhatIsOffered();

    if (failures == 0) {
        std::printf("\nall checks passed\n");
        return 0;
    }
    std::printf("\n%d check(s) failed\n", failures);
    return 1;
}
