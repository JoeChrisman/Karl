//
// Created by Joe Chrisman on 4/11/23.
//

#include "Search.h"
#include "Notation.h"

Move Search::getBestMove()
{
    Score bestScore = Eval::MIN_SCORE;

    // moves equal to the best move
    std::vector<Move> equals;

    Gen::genMoves();
    const int numMoves = Gen::numMoves;
    Move moves[256];
    std::memcpy(moves, Gen::moveList, sizeof(Gen::moveList));
    const Position::Rights rights = Position::rights;
    for (int i = 0; i < numMoves; i++)
    {
        Move move = moves[i];

        Position::makeMove(move);
        Score score = -negamax(rights.isWhiteToMove ? BLACK : WHITE, 4);
        if (score > bestScore)
        {
            equals.clear();
            equals.push_back(move);
            bestScore = score;
        }
        else if (score == bestScore)
        {
            equals.push_back(move);
        }
        Position::unMakeMove(move, rights);
    }
    // add some variance during the game because when carl goes against other
    // engines the same game often happens over and over again
    return equals[rand() % equals.size()];
}

Score Search::negamax(Color color, int depth)
{
    if (!depth)
    {
        return Eval::evaluate(Position::materialScore) * color;
    }

    Score best = Eval::MIN_SCORE;
    Gen::genMoves();
    const int numMoves = Gen::numMoves;
    if (!numMoves)
    {
        if (~Gen::safeSquares & Position::bitboards[color == WHITE ? WHITE_KING : BLACK_KING])
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
        Score score = -negamax((Color)-color, depth - 1);
        if (score > best)
        {
            best = score;
        }
        Position::unMakeMove(move, rights);
    }

    return best;
}