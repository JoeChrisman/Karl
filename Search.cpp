//
// Created by Joe Chrisman on 4/11/23.
//

#include "Search.h"
#include "Notation.h"

Move Search::getBestMove()
{
    Move bestMove = Moves::NULL_MOVE;
    Score bestScore = Eval::MIN_SCORE;

    Gen::genMoves();
    const int numMoves = Gen::numMoves;
    Move moves[256];
    std::memcpy(moves, Gen::moveList, sizeof(Gen::moveList));
    const Position::Rights rights = Position::rights;
    for (int i = 0; i < numMoves; i++)
    {
        Move move = moves[i];

        Position::makeMove(move);
        Score score = -negamax(rights.isWhiteToMove ? -1 : 1, 4);
        if (score > bestScore)
        {
            bestScore = score;
            bestMove = move;
        }
        Position::unMakeMove(move, rights);
        //std::cout << Notation::moveToStr(move) << ": " << score << "\n";
    }
    return bestMove;
}

Score Search::negamax(int turn, int depth)
{
    if (!depth)
    {
        return Eval::evaluate(Position::materialScore) * turn;
    }

    Score best = Eval::MIN_SCORE;
    Gen::genMoves();
    const int numMoves = Gen::numMoves;
    if (!numMoves)
    {
        if (~Gen::safeSquares & Position::bitboards[turn == 1 ? WHITE_KING : BLACK_KING])
        {
            return Eval::MIN_SCORE + MAX_DEPTH - depth;
        }
        else
        {
            return Eval::DRAW_SCORE;
        }
    }
    Move moves[256];
    std::memcpy(moves, Gen::moveList, sizeof(Gen::moveList));
    const Position::Rights rights = Position::rights;
    for (int i = 0; i < numMoves; i++)
    {
        Move move = moves[i];
        Position::makeMove(move);
        Score score = -negamax(-turn, depth - 1);
        if (score > best)
        {
            best = score;
        }
        Position::unMakeMove(move, rights);
    }

    return best;
}