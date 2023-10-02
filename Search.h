//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_SEARCH_H
#define KARL_SEARCH_H

#include "Eval.h"
#include "MoveGen.h"

inline constexpr int MAX_DEPTH = 64;

struct ScoredMove
{
    Move move;
    Score score;
};

class Search
{
public:
    Search(Position& position, MoveGen& moveGen, Evaluator& evaluator);

    ScoredMove searchByDepth(const int depth);
    Move searchByTime(const int msTargetElapsed);
    Move searchByTimeControl(const int msRemaining, const int msIncrement);

private:
    Evaluator& evaluator;
    Position& position;
    MoveGen& moveGen;

    Score quiescence(Score alpha, const Score beta, const int color);
    Score negamax(const int color, const int depth, Score alpha, Score beta);

    template<bool isQuiescent>
    void orderMove(Move moves[256], const int numMoves, const int moveNum, const int depth, const Move principalMove);

    inline bool isRepetition();

    void printSearchInfo(
            const long msElapsed,
            const int depth,
            const ScoredMove& bestMove);

    void printPrincipalVariation(const Hash zobristHash, const int depth);

    void printSearchTime(
            const long msTargetElapsed,
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
