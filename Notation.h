//
// Created by Joe Chrisman on 4/12/23.
//

#ifndef KARL_NOTATION_H
#define KARL_NOTATION_H

#include "Moves.h"

namespace Notation
{

    int charToFile(const char fileChar);
    int charToRank(const char rankChar);

    Square strToSquare(const std::string& notation);

    std::string fileToStr(const int file);
    std::string  rankToStr(const int rank);
    std::string squareToStr(const Square square);
    std::string moveToStr(const Move move);
};


#endif //KARL_NOTATION_H
