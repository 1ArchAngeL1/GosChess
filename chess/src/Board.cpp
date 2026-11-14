#include "GosChess/Board.hh"

#include <cctype>
#include <cstring>
#include <stdexcept>

namespace GosChess {
    static bool IsCharacter(const char &input) {
        const char upper_input = static_cast<char>(std::toupper(input));
        return upper_input >= 'A' && upper_input <= 'Z';
    }


    static figure_t FigureFromChar(const char &figure_rep) {
        const unsigned char type = FigureTypeFromFenChar(figure_rep);
        if (type == 0) {
            throw std::invalid_argument("given figure representation doesn't exist");
        }
        GosChess::figure_t figure;
        figure.color_ = std::isupper(figure_rep) ? GosChess::color_t::WHITE : GosChess::color_t::BLACK;
        figure.type_ = type;
        return figure;
    }


    unsigned char *decodeFen(const std::string &fen_str) {

        auto *result_board = new unsigned char[board_t::BOARD_SIZE]{0};
        uint32_t index = 0;

        for (const char &symbol: fen_str) {
            if (symbol == '/') continue;
            if (index >= board_t::BOARD_SIZE) break;
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

    board_t::board_t() : board_(new unsigned char[BOARD_SIZE]{0}) {}

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

    // Defined explicitly because the class owns a raw buffer: the compiler's
    // implicit assignment would copy the pointer and double free it.
    board_t &board_t::operator=(const board_t &board) {
        if (this == &board) return *this;
        if (board_ == nullptr) board_ = new unsigned char[BOARD_SIZE];
        memcpy(board_, board.board_, BOARD_SIZE * sizeof(unsigned char));
        game_rev_ = board.game_rev_;
        return *this;
    }

    board_t &board_t::operator=(board_t &&board) noexcept {
        if (this == &board) return *this;
        delete[] board_;
        board_ = board.board_;
        board.board_ = nullptr;
        game_rev_ = std::move(board.game_rev_);
        return *this;
    }

    board_t::~board_t() {
        delete[] board_;
    }

    void board_t::setState(const std::string &state) {
        delete[] board_;
        board_ = decodeFen(state);
        game_rev_ = {};
    }

    void board_t::saveState() {
        std::array<unsigned char, board_t::BOARD_SIZE> arr{};
        std::copy(board_t::board_, board_t::board_ + BOARD_SIZE, arr.begin());
        game_rev_.push(arr);
    }

    bool board_t::restoreState() {
        if (game_rev_.empty()) return false;
        std::copy(game_rev_.top().begin(), game_rev_.top().end(), board_);
        game_rev_.pop();
        return true;
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

    std::string board_t::boardStateToFen() const {
        std::string fen_str;
        int empty = 0;
        for (int i = 0; i < board_t::BOARD_SIZE; i++) {
            if (i % ROW_LENGTH == 0 && i != 0) {
                if (empty != 0) fen_str += static_cast<char>('0' + empty);
                fen_str += '/';
                empty = 0;
            }
            if (board_[i] == 0) {
                empty++;
                continue;
            }
            if (empty != 0) {
                fen_str += static_cast<char>('0' + empty);
                empty = 0;
            }
            fen_str += FenCharFromFigure(board_[i]);
        }
        if (empty != 0) fen_str += static_cast<char>('0' + empty);
        fen_str += '/';
        return fen_str;
    }

    figure_t::figure_t(const unsigned char &clr, const unsigned char &tp) {
        full_type ^= full_type;
        color_ = clr;
        type_ = tp;
    }

    board_node_t GetSquare(const int &num) {
        int x = num % GosChess::board_t::ROW_LENGTH;
        int y = num / GosChess::board_t::ROW_NUM;
        return {y, x};
    }

    int GetNumFromNode(const board_node_t &cell) {
        return cell.y_ * GosChess::board_t::ROW_NUM + cell.x_;
    }

}
