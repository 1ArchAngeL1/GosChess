
#ifndef GOSCHESS_INGAMECALCULATIONS_HH
#define GOSCHESS_INGAMECALCULATIONS_HH

#include "../global/MainOjbects/GameObjects.hh"
#include <SFML/Graphics.hpp>
#include <unordered_set>
#include "../global/GameGlobals.hh"
#include "ComputationGlobals/ComputationGlobals.hh"

namespace GosChess {

    GosChess::board_node_t GetSquare(const int &);

    GosChess::board_node_t GetNodeFromScreen(const float &, const float &);

    GosChess::action_t InvertMove(GosChess::action_t);

    int GetNumFromNode(const GosChess::board_node_t &);

    bool MakeMove(GosChess::action_t, GosChess::board_t &, GosChess::color_t, GosChess::actions_set_t &);

    bool UndoMove(GosChess::board_t &);

    bool CanMakeMove(GosChess::action_t, std::unordered_set<GosChess::action_t, GosChess::action_hash_t> &);

    bool CheckMate(GosChess::board_t &, GosChess::color_t);

    bool CheckForDraw(GosChess::board_t &, GosChess::color_t);

    bool CheckIndexForAttackers(const unsigned char *, const int &);

    void GenerateOffsets();

    void CalculateAvailableMoves(const unsigned char *, GosChess::color_t);

    void ChangeActiveColour(GosChess::board_t &);

    void MakeMoveForce(GosChess::action_t, GosChess::board_t &);

    void PromotePawns(GosChess::board_t &);
}


#endif
