#ifndef GOSCHESS_MOVE_HH
#define GOSCHESS_MOVE_HH

#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

#include "Board.hh"

namespace GosChess {

    // How far a square is from the edge of the board in each of the eight ray
    // directions, indexed the same way as direction_offsets below.
    struct offset_t {
        constexpr offset_t(int8_t north, int8_t south, int8_t west, int8_t east) :
                n(north), s(south), w(west), e(east),
                ne(std::min(north, east)),
                se(std::min(south, east)),
                nw(std::min(north, west)),
                sw(std::min(south, west)) {}

        constexpr offset_t() = default;

        constexpr int operator[](const int &index) const {
            if (index < 0 || index >= 8) {
                throw std::invalid_argument("given index is out of range");
            }
            return ((const std::int8_t *) this)[index];
        }

        int8_t n{};
        int8_t s{};
        int8_t w{};
        int8_t e{};
        int8_t ne{};
        int8_t se{};
        int8_t nw{};
        int8_t sw{};
    };


    // A move is a pair of absolute square indices plus, for a pawn reaching
    // the far rank, what it becomes. It carries no orientation of its own and
    // so can be put on the wire as-is.
    //
    // The promotion piece is part of the move's identity, not a detail: a pawn
    // on e7 has four different legal moves to e8, and a player who wanted a
    // knight must not be given a queen.
    struct action_t {
        std::int8_t move_from;
        std::int8_t move_to;
        // One of figure_types_t, or 0 when the move is not a promotion.
        unsigned char promotion;

        // Defaults to a square that cannot exist, so an action that was never
        // filled in is rejected by the rules instead of indexing the board
        // with whatever happened to be on the stack.
        constexpr action_t() : move_from(-1), move_to(-1), promotion(0) {}

        constexpr action_t(std::int8_t move_from, std::int8_t move_to, unsigned char promotion = 0)
                : move_from(move_from), move_to(move_to), promotion(promotion) {}

        constexpr bool operator==(const GosChess::action_t &rhm) const {
            return this->move_from == rhm.move_from && this->move_to == rhm.move_to &&
                   this->promotion == rhm.promotion;
        }

        [[nodiscard]] constexpr bool isPromotion() const { return promotion != 0; }
    };


    class action_hash_t {
    public:
        std::size_t operator()(const GosChess::action_t &move) const {
            std::size_t hash = static_cast<std::size_t>(move.move_from) * board_t::BOARD_SIZE +
                               static_cast<std::size_t>(move.move_to);
            return hash * 8u + move.promotion;
        }
    };

    typedef std::unordered_set<GosChess::action_t, GosChess::action_hash_t> actions_set_t;

    typedef std::unordered_map<std::int8_t, actions_set_t> action_map_t;

    // Ray directions: N, S, W, E, NE, SE, NW, SW. The first four are the rook
    // rays and the last four the bishop rays, which the generator relies on.
    extern const std::int8_t direction_offsets[8];

    extern constinit GosChess::offset_t precalculated_offsets[GosChess::board_t::BOARD_SIZE];

    // Fills precalculated_offsets. Idempotent, and called automatically by the
    // rules entry points, so callers no longer have to remember it.
    void GenerateOffsets();

}

#endif
