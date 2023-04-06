//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_POSITION_H
#define KARL_POSITION_H

#include <string>
#include <iostream>
#include <vector>
#include "Definitions.h"

class Position
{
public:
    Position();
    explicit Position(const std::string& fen);

    std::vector<U64> bitboards = std::vector<U64>(12, EMPTY_BOARD);
    std::vector<Piece> pieces = std::vector<Piece>(64, NULL_PIECE);

    void printPosition() const;
    void makeMove();
    void unMakeMove();

private:

    Piece getPieceByChar(const char piece) const;
    std::string getUnicodePiece(const Piece piece) const;

};


#endif //KARL_POSITION_H
