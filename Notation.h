//
// Created by Joe Chrisman on 4/12/23.
//

#ifndef KARL_NOTATION_H
#define KARL_NOTATION_H

#include <string>
#include "Moves.h"

namespace Notation
{
    char pieceToChar(const Piece piece);
    Piece charToPiece(const char letter);
    std::string pieceToUnicode(const Piece piece);

    int charToFile(const char fileChar);
    int charToRank(const char rankChar);

    Square strToSquare(const std::string& notation);

    std::string fileToStr(const int file);
    std::string  rankToStr(const int rank);
    std::string squareToStr(const Square square);
    std::string moveToStr(const Move move);
};


#endif //KARL_NOTATION_H
