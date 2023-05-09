//
// Created by Joe Chrisman on 4/11/23.
//

#include "Search.h"
#include "Notation.h"

bool Search::isOutOfTime = false;
long Search::endTime = 0;

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

    struct SearchInfo
    {
        U64 negaNodes;
        U64 quietNodes;
        U64 leafNodes;
    };

    SearchInfo info;

    Score quiescence(Score alpha, Score beta, int color)
    {
        info.quietNodes++;
        Score score = Eval::evaluate(Position::materialScore, Position::midgamePlacementScore) * color;
        if (score >= beta)
        {
            return beta;
        }
        if (score > alpha)
        {
            alpha = score;
        }

        Gen::genCaptures();
        Move captures[256];
        std::memcpy(captures, Gen::moveList, sizeof(Gen::moveList));
        const int numCaptures = Gen::numMoves;
        const Position::Irreversibles state = Position::irreversibles;
        for (int captureNum = 0; captureNum < numCaptures; captureNum++)
        {
            orderMove(captures, numCaptures, captureNum);
            Move move = captures[captureNum];
            Position::makeMove(move);
            Score score = -quiescence(-beta, -alpha, -color);
            Position::unMakeMove(move, state);
            if (score >= beta)
            {
                return beta;
            }
            if (score > alpha)
            {
                alpha = score;
            }
        }
        return alpha;
    }

    Score negamax(int color, int depth, Score alpha, Score beta)
    {
        if (getEpochMilis() > Search::endTime)
        {
            Search::isOutOfTime = true;
            return 0;
        }

        if (isRepetition() || Position::irreversibles.reversiblePlies >= 50)
        {
            info.leafNodes++;
            return Eval::CONTEMPT;
        }
        if (!depth)
        {
            info.leafNodes++;
            //return quiescence(alpha, beta, color);
            return Eval::evaluate(Position::materialScore, Position::midgamePlacementScore) * color;
        }
        info.negaNodes++;

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
            Score score = -negamax(-color, depth - 1, -beta, -alpha);
            Position::unMakeMove(move, state);
            if (score >= beta)
            {
                return beta;
            }
            if (score > alpha)
            {
                alpha = score;
            }
        }
        return alpha;
    }
}

void Search::init()
{
    initCaptureScores();
}

Move Search::searchByDepth(const int depth)
{
    isOutOfTime = false;

    timespec start = {};
    timespec end = {};
    clock_gettime(CLOCK_REALTIME, &start);

    info = {};

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
        Score score = -negamax(Position::isWhiteToMove ? WHITE : BLACK, depth, Eval::MIN_SCORE, Eval::MAX_SCORE);
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
        if (isOutOfTime)
        {
            return Moves::NULL_MOVE;
        }
    }
    // add some variance during the game because when carl goes against other
    // engines the same game often happens over and over again
    Move bestMove = bestMoves[rand() % bestMoves.size()];

    clock_gettime(CLOCK_REALTIME, &end);
    /*
    double startMillis = (start.tv_sec * 1000.0) + (start.tv_nsec / 1000000.0);
    double endMillis = (end.tv_sec * 1000.0) + (end.tv_nsec / 1000000.0);
    double msElapsed = endMillis - startMillis;

    double branchingFactor = (double)(info.negaNodes + info.leafNodes - 1) / (double)info.negaNodes;

    std::cout << "\t\t ~ Depth: " << depth << ", Time: " << msElapsed << "ms, Score: " << bestScore;
    std::cout << ", Move: " << Notation::moveToStr(bestMove) << ", Negamax nodes: " << info.negaNodes;
    std::cout << ", Quiet nodes: " << info.quietNodes << ", Leaf nodes: " << info.leafNodes;
    std::cout << ", kN/S: " << (double)(info.negaNodes + info.quietNodes) / msElapsed;
    std::cout << ", Average branching factor: " << branchingFactor << "\n";
     */

    return bestMove;
}

Move Search::searchByTime(const int millis)
{
    long startTime = getEpochMilis();
    endTime = startTime + millis;

    Gen::genMoves();
    const int numMoves = Gen::numMoves;
    if (numMoves == 1)
    {
        //std::cout << "\t~ Target elapsed: " << millis << ", Actual elapsed: " << getEpochMilis() - startTime << "\n";
        return Gen::moveList[0];
    }

    int depth = 0;
    Move best;
    long lastSearchTime = 0;
    while (++depth <= MAX_DEPTH)
    {
        // if we estimate the next search will take too much time
        if (endTime < getEpochMilis() + 7 * lastSearchTime)
        {
            // give up and use the best move from the previous depth
            break;
        }

        long searchStartTime = getEpochMilis();
        const Move bestForDepth = searchByDepth(depth);
        lastSearchTime = getEpochMilis() - searchStartTime;
        // if we ran out of time while searching
        if (bestForDepth == Moves::NULL_MOVE)
        {
            // give up and use the best move from the previous depth
            break;
        }
        best = bestForDepth;
    }

    //std::cout << "\t~ Target elapsed: " << millis << ", Actual elapsed: " << getEpochMilis() - startTime << "\n";

    return best;
}

Move Search::searchByTimeControl(
        const int whiteRemaining,
        const int blackRemaining,
        const int whiteIncrement,
        const int blackIncrement)
{
    int bestTime = (whiteRemaining + whiteIncrement * 29) / 30;
    return searchByTime(bestTime);
}


