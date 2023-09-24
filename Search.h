//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_SEARCH_H
#define KARL_SEARCH_H

#include "Eval.h"
#include "Gen.h"

inline static constexpr int MAX_DEPTH = 64;

struct ScoredMove
{
    Move move;
    Score score;
};

class Search
{
public:
    Search(Position& position, Gen& generator);

    ScoredMove searchByDepth(const int depth);
    Move searchByTime(const int millis);
    Move searchByTimeControl(
            const int whiteRemaining,
            const int blackRemaining,
            const int whiteIncrement,
            const int blackIncrement);

private:
    Position& position;
    Gen& generator;

    Score quiescence(Score alpha, const Score beta, const int color);
    Score negamax(const int color, const short depth, Score alpha, Score beta);

    template<bool isQuiescent>
    void orderMove(Move moves[256], const int numMoves, const int moveNum, const int depth);

    inline bool isRepetition();

    void printSearchInfo(
            const long msElapsed,
            const int depth,
            const ScoredMove& bestMove);

    void printSearchTime(
            const long targetElapsed,
            const long startTime);

    U64 branchNodes;
    U64 quietNodes;
    U64 leafNodes;

    long endTime;
    bool isOutOfTime;

    Score captureScores[13][13];
    Move killerMoves[MAX_DEPTH][2];
};

#endif //KARL_SEARCH_H
