//
// Created by Joe Chrisman on 4/11/23.
//

#include "Search.h"

Move Search::getBestMove()
{
    Gen::genMoves();
    Move moves[256];
    std::memcpy(moves, Gen::moveList, sizeof(Gen::moveList));
    int numMoves = 0;
    for (Move move : moves)
    {
        if (move == Moves::NULL_MOVE)
        {
            break;
        }
        numMoves++;
    }
    return numMoves ? moves[rand() % numMoves] : Moves::NULL_MOVE;
}