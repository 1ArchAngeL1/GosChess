//
// Created by Leo Gogichaishvili on 23.11.21.
//

#include "GameObjects.hh"

namespace GosChess {
    static bool IsCharacter(const char &input) {
        const char upper_input = static_cast<char>(std::toupper(input));
        return upper_input >= 'A' && upper_input <= 'Z';
    }


    static figure_t FigureFromChar(const char &figure_rep) {
        if (!IsCharacter(figure_rep)) {
            throw std::invalid_argument("given figure representation doesn't exist");
        }
        GosChess::figure_t figure;

        if (std::isupper(figure_rep)) figure.color_ = GosChess::color_t::WHITE;
        else figure.color_ = GosChess::color_t::BLACK;

        figure.type_ = GosChess::FEN_TO_FIG.at(figure_rep);

        return figure;
    }


    const std::map<char, unsigned char> FEN_TO_FIG = {{'p', figure_types_t::PAWN},
                                                      {'q', figure_types_t::QUEEN},
                                                      {'k', figure_types_t::KING},
                                                      {'b', figure_types_t::BISHOP},
                                                      {'r', figure_types_t::ROOK},
                                                      {'n', figure_types_t::KNIGHT},
                                                      {'P', figure_types_t::PAWN},
                                                      {'Q', figure_types_t::QUEEN},
                                                      {'K', figure_types_t::KING},
                                                      {'B', figure_types_t::BISHOP},
                                                      {'R', figure_types_t::ROOK},
                                                      {'N', figure_types_t::KNIGHT}};


    const std::map<unsigned char, char> FIG_TO_FEN = {{figure_t(color_t::WHITE,
                                                                figure_types_t::ROOK).full_type,   'R'},
                                                      {figure_t(color_t::WHITE,
                                                                figure_types_t::BISHOP).full_type, 'B'},
                                                      {figure_t(color_t::WHITE,
                                                                figure_types_t::PAWN).full_type,   'P'},
                                                      {figure_t(color_t::WHITE,
                                                                figure_types_t::KNIGHT).full_type, 'N'},
                                                      {figure_t(color_t::WHITE,
                                                                figure_types_t::KING).full_type,   'K'},
                                                      {figure_t(color_t::WHITE,
                                                                figure_types_t::QUEEN).full_type,  'Q'},
                                                      {figure_t(color_t::BLACK,
                                                                figure_types_t::ROOK).full_type,   'r'},
                                                      {figure_t(color_t::BLACK,
                                                                figure_types_t::BISHOP).full_type, 'b'},
                                                      {figure_t(color_t::BLACK,
                                                                figure_types_t::PAWN).full_type,   'p'},
                                                      {figure_t(color_t::BLACK,
                                                                figure_types_t::KNIGHT).full_type, 'n'},
                                                      {figure_t(color_t::BLACK,
                                                                figure_types_t::KING).full_type,   'k'},
                                                      {figure_t(color_t::BLACK,
                                                                figure_types_t::QUEEN).full_type,  'q'}};

    unsigned char *decodeFen(const std::string &fen_str) {

        auto *result_board = new unsigned char[board_t::BOARD_SIZE]{0};
        uint32_t index = 0;

        for (const char &symbol: fen_str) {
            if (symbol == '/') continue;
            if (IsCharacter(symbol)) {
                figure_t fig = FigureFromChar(symbol);
                result_board[index] = fig.full_type;
                index++;
                continue;
            }
            int num_jump = symbol - '0';
            index += num_jump;
        }

        return result_board;
    }

    board_t::board_t(const std::string &initial_state) {
        board_ = decodeFen(initial_state);
    }

    board_t::board_t() : board_(nullptr) {}

    board_t::board_t(const board_t &board) {
        board_ = new unsigned char[BOARD_SIZE];
        memcpy(board_, board.board_, BOARD_SIZE * sizeof(unsigned char));
        game_rev_ = board.game_rev_;
    }

    board_t::board_t(board_t &&board) noexcept {
        board_ = board.board_;
        board.board_ = nullptr;
        game_rev_ = std::move(board.game_rev_);
    }

    board_t::~board_t() {
        delete[] board_;
    }

    void board_t::setState(const std::string &state) {
        delete[] board_;
        board_ = decodeFen(state);
    }

    void board_t::saveState() {
        std::array<unsigned char, board_t::BOARD_SIZE> arr{};
        std::copy(board_t::board_, board_t::board_ + BOARD_SIZE, arr.begin());
        game_rev_.push(arr);
    }

    void board_t::setFigure(const int &node, const unsigned char &figure) {
        board_[node] = figure;
    }

    const unsigned char *board_t::getRawBoard() const {
        return board_;
    }

    figure_t board_t::getFigure(const int &index) const {
        unsigned char fig_char = board_[index];
        return figure_t(fig_char);
    }

// ignore this block of code, it was little challenge for my self :))) i know it looks hoooriibbleee :))
    std::string board_t::boardStateToFen() const {
        unsigned char *target_board = board_;
        std::string fen_str;
        int empty = 0;
        for (int i = 0; i < board_t::BOARD_SIZE; i++) {
            if (i % 8 == 0) {
                fen_str += ((empty != 0) ? std::string() + (char) ('0' + empty) : "") + "/";
                empty = 0;
            }
            std::string to_add = (target_board[i] == 0) ? "" : FIG_TO_FEN.at(target_board[i]) + std::string();
            (to_add.empty()) ? (std::string() + (char) empty++) : (empty == 0 ? fen_str += to_add : fen_str +=
                                                                                                            (char) ('0' +
                                                                                                                    empty) +
                                                                                                            std::string() +
                                                                                                            to_add);
            if (!to_add.empty())empty = 0;
        }
        return fen_str;
    }

    figure_t::figure_t(const unsigned char &clr, const unsigned char &tp) {
        full_type ^= full_type;
        color_ = clr;
        type_ = tp;
    }

}





