//
// Created by Joe Chrisman on 4/12/23.
//

#include <string>
#include "Notation.h"

int Notation::charToFile(const char fileChar)
{
    return (int)(fileChar - 'a');
}

int Notation::charToRank(const char rankChar)
{
    return (int)(rankChar - '1');
}

std::string Notation::fileToStr(const int file)
{
    return std::string{char(file + 'a')};
}

std::string Notation::rankToStr(const int rank)
{
    return std::string{char('1' + rank)};
}

Square Notation::strToSquare(const std::string& notation)
{
    int rank = charToRank(notation[1]);
    int file = charToFile(notation[0]);
    return getSquare(rank, file);
}

std::string Notation::squareToStr(const Square square)
{
    return fileToStr(getFile(square)) + rankToStr((getRank(square)));
}

std::string Notation::moveToStr(const Move move)
{
    return squareToStr(Moves::getFrom(move)) + squareToStr(Moves::getTo(move));
}