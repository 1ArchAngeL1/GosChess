//
// Created by Leo Gogichaishvili on 23.11.21.
//

#include "GameObjects.h"


const std::map<char, unsigned char> GosChess::Board::FEN_TO_FIG = {{'p', FigureTypes::PAWN},
                                                                   {'q', FigureTypes::QUEEN},
                                                                   {'k', FigureTypes::KING},
                                                                   {'b', FigureTypes::BISHOP},
                                                                   {'r', FigureTypes::ROOK},
                                                                   {'n', FigureTypes::KNIGHT}};


const std::map<unsigned char, char> GosChess::Board::FIG_TO_FEN = {{Figure(Color::WHITE, 1).full_type, 'R'},
                                                                   {Figure(Color::WHITE, 2).full_type, 'B'},
                                                                   {Figure(Color::WHITE, 3).full_type, 'P'},
                                                                   {Figure(Color::WHITE, 4).full_type, 'N'},
                                                                   {Figure(Color::WHITE, 5).full_type, 'K'},
                                                                   {Figure(Color::WHITE, 6).full_type, 'Q'},
                                                                   {Figure(Color::BLACK, 1).full_type, 'r'},
                                                                   {Figure(Color::BLACK, 2).full_type, 'b'},
                                                                   {Figure(Color::BLACK, 3).full_type, 'p'},
                                                                   {Figure(Color::BLACK, 4).full_type, 'n'},
                                                                   {Figure(Color::BLACK, 5).full_type, 'k'},
                                                                   {Figure(Color::BLACK, 6).full_type, 'q'}};


GosChess::Figure::Figure(const unsigned char &color, const unsigned char &type) {
    this->full_type = 0;
    this->color = color;
    this->type = type;
}


bool isCharacter(const char &character_figure) {
    return (character_figure >= 'a' && character_figure <= 'z') ||
           (character_figure >= 'A' && character_figure <= 'Z');
}


//upper case white
//lower case black
unsigned char *GosChess::Board::DecodeFen(std::string fen_str) {
    unsigned char *board_demo = new unsigned char[Board::BOARD_SIZE]{0};
    short curr_pos = 0;
    for (int i = 0; i < fen_str.length(); i++) {
        if (fen_str[i] == '/')continue;
        if (isCharacter(fen_str[i])) {
            Figure fig = Board::FigureFromChar(fen_str[i]);
            board_demo[curr_pos] = fig.full_type;
            curr_pos++;
        } else {
            int num_jump = fen_str[i] - '0';
            curr_pos += num_jump;
        }
    }
    return board_demo;
}

GosChess::Figure GosChess::Board::FigureFromChar(const unsigned char &figure_rep) {
    if (isCharacter(figure_rep)) {
        Figure figure;
        if (std::isupper(figure_rep))figure.color = Color::WHITE;
        else figure.color = Color::BLACK;
        figure.type = FEN_TO_FIG.at(std::tolower(figure_rep));
        return figure;
    } else {
        throw new std::exception();
    }
}

GosChess::Board::Board(std::string initial_state) {
    this->board = Board::DecodeFen(initial_state);
}

GosChess::Figure GosChess::Board::GetPosition(const int &grid_num) {
    return Board::FigureFromChar(this->board[grid_num]);
}

void GosChess::Board::SetPosition(const int &grid_num, unsigned char figure) {
    this->board[grid_num] = figure;
}

std::string GosChess::Board::BoardStateToFen() {
    unsigned char *target_board = this->board;
    std::string fen_str = "";
    int empty = 0;
    for (int i = 0; i < Board::BOARD_SIZE; i++) {
        if (i % 8 == 0) {
            fen_str += ((empty != 0) ? std::string() + (char) ('0' + empty) : "") + "/";
            empty = 0;
        }
        std::string to_add = (target_board[i] == 0) ? "" : Board::FIG_TO_FEN.at(target_board[i]) + std::string();
        (to_add == "") ? (std::string() + (char) empty++) : (empty == 0 ? fen_str += to_add : fen_str += (char) ('0' +
                                                                                                                 empty) +
                                                                                                         std::string() +
                                                                                                         to_add);
        if (to_add != "")empty = 0;
    }
    return fen_str;
}





