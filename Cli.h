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

    extern Position* position;
    extern MoveGenerator* moveGenerator;

    void showReady();
    int notationToFile(const char fileChar);
    int notationToRank(const char rankChar);
    char fileToNotation(const int file);
    char rankToNotation(const int rank);
    Square notationToSquare(const std::string& notation);
    std::string squareToNotation(const Square square);
    std::string moveToNotation(const Move move);

};


#endif //KARL_CLI_H
