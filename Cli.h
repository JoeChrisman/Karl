//
// Created by Joe Chrisman on 4/7/23.
//

#ifndef KARL_CLI_H
#define KARL_CLI_H

#include <string>
#include <iostream>
#include "MoveGenerator.h"

namespace Cli
{
    int run();
    int runUci();

    extern bool isWhiteOnBottom;

    extern Position position;
    extern MoveGenerator moveGenerator;

    void showReady();
    int notationToFile(const char fileChar);
    int notationToRank(const char rankChar);
    char fileToNotation(const int file);
    char rankToNotation(const int rank);
    Square notationToSquare(const std::string& notation);
    std::string squareToNotation(const Square square);
    std::string moveToNotation(const Move move);

    struct PerftInfo
    {
        U64 leafNodes;
        U64 totalNodes;
        U64 totalCaptures;
        U64 totalEnPassant;
        U64 totalPromotions;
    };
    void perft(int depth, PerftInfo& info);

};


#endif //KARL_CLI_H
