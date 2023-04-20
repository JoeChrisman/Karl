//
// Created by Joe Chrisman on 4/11/23.
//

#include "Search.h"
#include "Notation.h"

namespace
{
    bool isRepetition()
    {
        int repetitions = 1;
        int index = Position::totalPlies - Position::irreversibles.reversiblePlies;
        while (index <= Position::totalPlies)
        {
            if (Position::hash == Position::history[index++])
            {
                repetitions++;
            }
        }
        return repetitions >= 3;
    }

    Score negamax(Color color, int depth, int alpha, int beta)
    {
        if (isRepetition() || Position::irreversibles.reversiblePlies >= 50)
        {
            return Eval::CONTEMPT;
        }
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
                return Eval::MIN_SCORE + Search::MAX_DEPTH - depth;
            }
            else
            {
                return Eval::CONTEMPT;
            }
        }
        Move moves[256];
        std::memcpy(moves, Gen::moveList, sizeof(Gen::moveList));
        const Position::Irreversibles state = Position::irreversibles;
        for (int i = 0; i < numMoves; i++)
        {
            Move move = moves[i];
            Position::makeMove(move);
            Score score = -negamax((Color)-color, depth - 1, -beta, -alpha);
            Position::unMakeMove(move, state);
            if (score > best)
            {
                best = score;
            }
            if (best > alpha)
            {
                alpha = best;
            }
            if (alpha >= beta)
            {
                break;
            }
        }

        return best;
    }
}

Move Search::getBestMove()
{
    Score bestScore = Eval::MIN_SCORE;

    // moves equal to the best move
    std::vector<Move> equals;

    Gen::genMoves();
    const int numMoves = Gen::numMoves;
    Move moves[256];
    std::memcpy(moves, Gen::moveList, sizeof(Gen::moveList));
    const Position::Irreversibles state = Position::irreversibles;
    for (int i = 0; i < numMoves; i++)
    {
        Move move = moves[i];

        Position::makeMove(move);
        Score score = -negamax(Position::isWhiteToMove ? WHITE : BLACK, 4, Eval::MIN_SCORE, Eval::MAX_SCORE);
        //std::cout << Notation::moveToStr(move) << ": " << score << "\n";
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
        Position::unMakeMove(move, state);
    }
    // add some variance during the game because when carl goes against other
    // engines the same game often happens over and over again
    return equals[rand() % equals.size()];
}
