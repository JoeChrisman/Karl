//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_POSITION_H
#define KARL_POSITION_H

#include <string>
#include <iostream>
#include "Definitions.h"

class Position
{
public:
    Position(const std::string& fen);

    U64 bitboards[12];
    Piece pieces[64];

    void printPosition() const;
    void makeMove();
    void unMakeMove();

private:

    Piece getPieceByChar(const char piece) const;
    std::string getUnicodePiece(const Piece piece) const;

};


#endif //KARL_POSITION_H
