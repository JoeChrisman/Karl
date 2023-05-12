//
// Created by Joe Chrisman on 4/11/23.
//

#include "Search.h"
#include "Notation.h"

Search::Search(Position& position, Gen& generator)
: position(position), generator(generator), captureScores{0}
{
    branchNodes = 0;
    leafNodes = 0;
    quietNodes = 0;

    endTime = 0;
    isOutOfTime = false;

    // initialize the capture scores
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

void Search::orderMove(Move moves[256], int numMoves, int moveNum)
{
    Score bestScore = MIN_SCORE;
    int bestMoveIndex = -1;
    for (int i = moveNum; i < numMoves; i++)
    {
        Move move = moves[i];
        Score score = captureScores[getMoved(move)][getCaptured(move)];
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

bool Search::isRepetition()
{
    int repetitions = 1;
    int index = position.totalPlies - position.irreversibles.reversiblePlies;
    while (index <= position.totalPlies)
    {
        if (position.hash == position.history[index++])
        {
            repetitions++;
        }
    }
    return repetitions >= 3;
}

Score Search::quiescence(Score alpha, const Score beta, const int color)
{
    quietNodes++;
    Score score = evaluate(position.materialScore, position.midgamePlacementScore) * color;
    if (score >= beta)
    {
        return beta;
    }
    if (score > alpha)
    {
        alpha = score;
    }

    generator.genCaptures();
    Move captures[256];
    std::memcpy(captures, generator.moveList, sizeof(Gen::moveList));
    const int numCaptures = generator.numMoves;
    const Position::Irreversibles state = position.irreversibles;
    for (int captureNum = 0; captureNum < numCaptures; captureNum++)
    {
        orderMove(captures, numCaptures, captureNum);
        Move move = captures[captureNum];
        position.makeMove(move);
        Score score = -quiescence(-beta, -alpha, -color);
        position.unMakeMove(move, state);
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

Score Search::negamax(int color, int depth, Score alpha, Score beta)
{
    if (isOutOfTime)
    {
        return 0;
    }
    if (leafNodes & 2047)
    {
        // check if we ran out of time
        if (getEpochMillis() > endTime)
        {
            isOutOfTime = true;
            return 0;
        }
    }
    // if we have broken the threefold repetition rule or the fifty move rule
    if (isRepetition() || position.irreversibles.reversiblePlies >= 100)
    {
        leafNodes++;
        return CONTEMPT;
    }
    if (!depth)
    {
        leafNodes++;
        //return quiescence(alpha, beta, color);
        return evaluate(position.materialScore, position.midgamePlacementScore) * color;
    }
    branchNodes++;

    generator.genMoves();
    const int numMoves = generator.numMoves;
    if (!numMoves)
    {
        if (generator.isInCheck(color))
        {
            return MIN_SCORE + MAX_DEPTH - depth;
        }
        else
        {
            return CONTEMPT;
        }
    }
    Move moves[256];
    std::memcpy(moves, generator.moveList, sizeof(Gen::moveList));
    const Position::Irreversibles state = position.irreversibles;
    for (int moveNum = 0; moveNum < numMoves; moveNum++)
    {
        orderMove(moves, numMoves, moveNum);
        Move move = moves[moveNum];
        position.makeMove(move);
        Score score = -negamax(-color, depth - 1, -beta, -alpha);
        position.unMakeMove(move, state);
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


Move Search::searchByDepth(const int depth)
{
    isOutOfTime = false;
    branchNodes = 0;
    leafNodes = 0;
    quietNodes = 0;

    timespec start = {};
    timespec end = {};
    clock_gettime(CLOCK_REALTIME, &start);

    Score bestScore = MIN_SCORE;
    std::vector<Move> bestMoves;

    generator.genMoves();
    const int numMoves = generator.numMoves;
    Move moves[256];
    std::memcpy(moves, generator.moveList, sizeof(Gen::moveList));
    const Position::Irreversibles state = position.irreversibles;
    for (int i = 0; i < numMoves; i++)
    {
        Move move = moves[i];

        position.makeMove(move);
        Score score = -negamax(position.isWhiteToMove ? WHITE : BLACK, depth, MIN_SCORE, MAX_SCORE);
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
        position.unMakeMove(move, state);
        if (isOutOfTime)
        {
            return NULL_MOVE;
        }
    }
    // add some variance during the game because when carl goes against other
    // engines the same game often happens over and over again
    Move bestMove = bestMoves[rand() % bestMoves.size()];

    clock_gettime(CLOCK_REALTIME, &end);

    double startMillis = (start.tv_sec * 1000.0) + (start.tv_nsec / 1000000.0);
    double endMillis = (end.tv_sec * 1000.0) + (end.tv_nsec / 1000000.0);
    double msElapsed = endMillis - startMillis;

    double branchingFactor = (double)(branchNodes + leafNodes - 1) / (double)branchNodes;

    std::cout << "info string Depth: " << depth << ", Time: " << msElapsed << "ms, Score: " << bestScore;
    std::cout << ", Move: " << moveToStr(bestMove) << ", branch nodes: " << branchNodes;
    std::cout << ", Quiet nodes: " << quietNodes << ", Leaf nodes: " << leafNodes;
    std::cout << ", kN/S: " << (double)(branchNodes + quietNodes) / msElapsed;
    std::cout << ", ABF: " << branchingFactor << "\n";

    return bestMove;
}

Move Search::searchByTime(const int millis)
{
    long startTime = getEpochMillis();
    endTime = startTime + millis;

    generator.genMoves();
    const int numMoves = generator.numMoves;
    if (numMoves == 1)
    {
        std::cout << "info string Target elapsed: " << millis << ", Actual elapsed: " << getEpochMillis() - startTime << "\n";
        return generator.moveList[0];
    }

    int depth = 0;
    Move best;
    long lastSearchTime = 0;
    while (++depth <= MAX_DEPTH)
    {
        // if we estimate the next search will take too much time
        if (endTime < getEpochMillis() + 7 * lastSearchTime)
        {
            // give up and use the best move from the previous depth
            break;
        }

        long searchStartTime = getEpochMillis();
        const Move bestForDepth = searchByDepth(depth);
        lastSearchTime = getEpochMillis() - searchStartTime;
        // if we ran out of time while searching
        if (bestForDepth == NULL_MOVE)
        {
            // give up and use the best move from the previous depth
            break;
        }
        best = bestForDepth;
    }

    std::cout << "info string Target elapsed: " << millis << ", Actual elapsed: " << getEpochMillis() - startTime << "\n";

    return best;
}

Move Search::searchByTimeControl(
        const int whiteRemaining,
        const int blackRemaining,
        const int whiteIncrement,
        const int blackIncrement)
{
    int estimatedRemaining = whiteRemaining + whiteIncrement * 19;
    int bestTime = estimatedRemaining / 20;

    return searchByTime(bestTime);
}


