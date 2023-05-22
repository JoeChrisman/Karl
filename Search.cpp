//
// Created by Joe Chrisman on 4/11/23.
//

#include "Search.h"
#include "Notation.h"
#include <iomanip>

Search::Search(Position& position, Gen& generator)
: position(position), generator(generator), captureScores{0}, killerMoves{{NULL_MOVE}}
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

template<bool isQuiescent>
void Search::orderMove(Move moves[256], const int numMoves, const int moveNum, const int depth)
{
    Score bestScore = MIN_SCORE;
    int bestMoveIndex = -1;
    for (int i = moveNum; i < numMoves; i++)
    {
        Score score;
        const Move move = moves[i];
        // during quiescence search
        if constexpr (isQuiescent)
        {
            // order the move based on capture value and piece value
            score = captureScores[getMoved(move)][getCaptured(move)];
        }
        // during negamax search
        else
        {
            // if this move is a primary killer move
            if (move == killerMoves[depth][0] || move == killerMoves[depth][1])
            {
                // put killer moves after captures
                score = 94;
            }
            // if this move is not a killer move
            else
            {
                // order the move based on capture value and piece value
                score = captureScores[getMoved(move)][getCaptured(move)];
            }
        }

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
        orderMove<true>(captures, numCaptures, captureNum, -1);
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

Score Search::negamax(const int color, const int depth, Score alpha, Score beta)
{
    if (isOutOfTime)
    {
        return 0;
    }
    // if we have broken the threefold repetition rule or the fifty move rule
    if (isRepetition() || position.irreversibles.reversiblePlies >= 100)
    {
        leafNodes++;
        return CONTEMPT;
    }
    if (!depth)
    {
        // every few thousand leaf nodes
        if (++leafNodes & 4095)
        {
            // check if we ran out of time
            if (getEpochMillis() > endTime)
            {
                isOutOfTime = true;
                return 0;
            }
        }
        return quiescence(alpha, beta, color);
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
        orderMove<false>(moves, numMoves, moveNum, depth);
        Move move = moves[moveNum];
        position.makeMove(move);
        Score score = -negamax(-color, depth - 1, -beta, -alpha);
        position.unMakeMove(move, state);
        // if the score caused a beta cutoff
        if (score >= beta)
        {
            // a quiet move that caused a beta cutoff is a killer move
            if (getCaptured(move) == NULL_PIECE)
            {
                // store the killer move
                killerMoves[depth][1] = killerMoves[depth][0];
                killerMoves[depth][0] = move;
            }
            return beta;
        }
        // if the score raised alpha
        if (score > alpha)
        {
            alpha = score;
        }
    }
    return alpha;
}


ScoredMove Search::searchByDepth(const int depth)
{
    isOutOfTime = false;
    branchNodes = 0;
    leafNodes = 0;
    quietNodes = 0;

    const long startMillis = getEpochMillis();

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
            return ScoredMove{0, NULL_MOVE};
        }
    }
    // add some variance during the game because when carl goes against other
    // engines the same game often happens over and over again
    ScoredMove bestMove = ScoredMove{bestMoves[rand() % bestMoves.size()], bestScore};
    printSearchInfo(getEpochMillis() - startMillis, depth, bestMove);

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
        printSearchTime(millis, startTime);
        return generator.moveList[0];
    }

    int depth = 0;
    ScoredMove best = {};
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
        const ScoredMove bestForDepth = searchByDepth(depth);

        lastSearchTime = getEpochMillis() - searchStartTime;
        // if we ran out of time while searching
        if (bestForDepth.move == NULL_MOVE)
        {
            // give up and use the best move from the previous depth
            break;
        }
        // if we found a mating line while searching
        if (bestForDepth.score >= MAX_SCORE - MAX_DEPTH)
        {
            // play the move right away
            return bestForDepth.move;
        }

        best = bestForDepth;
    }

    printSearchTime(millis, startTime);

    return best.move;
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

void Search::printSearchInfo(
        const long msElapsed,
        const int depth,
        const ScoredMove& bestMove)
{
    const double branchingFactor = (double)(branchNodes + leafNodes - 1) / (double)branchNodes;
    const std::string kNodesPerSec = msElapsed ? std::to_string((branchNodes + quietNodes) / msElapsed) : "NAN";
    const std::string elapsed = std::to_string(msElapsed) + "ms";

    std::cout << std::left;

    std::cout << "info string | Depth: " << std::setw(2) << depth;
    std::cout << std::setw(7) << "| Time: " << std::setw(10) << elapsed;
    std::cout << std::setw(8) << "| Score: " << std::setw(6) << bestMove.score;
    std::cout << std::setw(7) << "| Move: " << std::setw(6) << moveToStr(bestMove.move);
    std::cout << std::setw(15) << "| Branch nodes: " << std::setw(10) << branchNodes;
    std::cout << std::setw(14) << "| Quiet nodes: " << std::setw(10) << quietNodes;
    std::cout << std::setw(13) << "| Leaf nodes: " << std::setw(10) << leafNodes;
    std::cout << std::setw(7) << "| kN/S: " << std::setw(6) << kNodesPerSec;
    std::cout << std::setw(6) << "| ABF: " << std::setw(10) << branchingFactor << "\n";
}

void Search::printSearchTime(
        const long targetElapsed,
        const long startTime)
{
    std::cout << "info string Target elapsed: " << targetElapsed << ", Actual elapsed: " << getEpochMillis() - startTime << "\n";
}