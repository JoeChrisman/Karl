//
// Created by Joe Chrisman on 4/7/23.
//

#ifndef KARL_CLI_H
#define KARL_CLI_H

#include "Search.h"

class Cli
{

public:
    Cli(const Zobrist& zobrist, const Magics& magics);
    int runCli();

private:
    bool isWhiteOnBottom;

    Position position;
    Search search;
    Gen generator;

    struct PerftInfo
    {
        U64 totalNodes;
        U64 totalSplit;

        U64 nodes;
        U64 captures;
        U64 enPassants;
        U64 promotions;
        U64 castles;
    };

    void showReady();
    int runUci();

    void runPerftSuite();
    void perft(int depth, PerftInfo &info, int splitDepth = -1);
    void printPerftInfo(const PerftInfo& info, const int depth, const double msElapsed);
};


#endif //KARL_CLI_H
