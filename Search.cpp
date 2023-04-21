//
// Created by Joe Chrisman on 4/11/23.
//

#include "Search.h"
#include "Notation.h"

namespace
{
    Score captureScores[13][13];

    void initCaptureScores()
    {
        static constexpr Score attackerScores[13] = {
                0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0
        };
        static constexpr Score victimScores[13] = {
                0, 100, 200, 300, 400, 500, 0, 100, 200, 300, 400, 500, 0
        };

        for (Piece attacker = NULL_PIECE; attacker <= BLACK_KING; attacker++)
        {
            for (Piece victim = NULL_PIECE; victim <= BLACK_KING; victim++)
            {
                captureScores[attacker][victim] = victimScores[victim] - attackerScores[attacker];
            }
        }
    }

    inline void orderMove(Move moves[256], int numMoves, int moveNum)
    {
        Score bestScore = Eval::MIN_SCORE;
        int bestMoveIndex = -1;
        for (int i = moveNum; i < numMoves; i++)
        {
            Move move = moves[i];
            Score score = captureScores[Moves::getMoved(move)][Moves::getCaptured(move)];
            if (score > bestScore)
            {
                bestScore = score;
                bestMoveIndex = i;
            }
        }

        Move bestMove = moves[bestMoveIndex];
        moves[bestMoveIndex] = moves[moveNum];
        moves[moveNum] = bestMove;
    }

    inline bool isRepetition()
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
        for (int moveNum = 0; moveNum < numMoves; moveNum++)
        {
            orderMove(moves, numMoves, moveNum);
            Move move = moves[moveNum];
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

void Search::init()
{
    initCaptureScores();
}

Move Search::getBestMove()
{
    Score bestScore = Eval::MIN_SCORE;
    std::vector<Move> bestMoves;

    Gen::genMoves();
    const int numMoves = Gen::numMoves;
    Move moves[256];
    std::memcpy(moves, Gen::moveList, sizeof(Gen::moveList));
    const Position::Irreversibles state = Position::irreversibles;
    for (int i = 0; i < numMoves; i++)
    {
        Move move = moves[i];

        Position::makeMove(move);
        Score score = -negamax(Position::isWhiteToMove ? WHITE : BLACK, 6, Eval::MIN_SCORE, Eval::MAX_SCORE);
        //std::cout << Notation::moveToStr(move) << ": " << score << "\n";
        if (score > bestScore)
        {
            bestMoves.clear();
            bestMoves.push_back(move);
            bestScore = score;
        }
        else if (score == bestScore)
        {
            bestMoves.push_back(move);
        }
        Position::unMakeMove(move, state);
    }
    // add some variance during the game because when carl goes against other
    // engines the same game often happens over and over again
    return bestMoves[rand() % bestMoves.size()];
}

