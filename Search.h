//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_SEARCH_H
#define KARL_SEARCH_H

#include "Eval.h"
#include "Gen.h"

struct ScoredMove
{
    Move move;
    Score score;
};

class Search
{
public:
    Search(Position& position, Gen& generator);
    static constexpr int MAX_DEPTH = 100;

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

    U64 branchNodes;
    U64 quietNodes;
    U64 leafNodes;

    long endTime;
    bool isOutOfTime;
    Score captureScores[13][13];
    //Move principalVariation[(MAX_DEPTH * (MAX_DEPTH + 1)) / 2];

    Score quiescence(Score alpha, const Score beta, const int color);
    Score negamax(int color, int depth, Score alpha, Score beta);

    inline void orderMove(Move moves[256], int numMoves, int moveNum);
    inline bool isRepetition();
};

#endif //KARL_SEARCH_H
