//
// Created by Joe Chrisman on 4/8/23.
//

#ifndef KARL_MAGICS_H
#define KARL_MAGICS_H

#include "Defs.h"

struct MagicSquare
{
    U64 blockers;
    U64 magic;
};

class Magics
{
public:
    Magics();

    MagicSquare ordinalMagics[64];
    MagicSquare cardinalMagics[64];

    U64 cardinalAttacks[64][4096];
    U64 ordinalAttacks[64][512];

private:
    U64 random64();

    U64 getMagicNumber(Square square, bool isCardinal);

    U64 getBishopBlockers(Square from);
    U64 getRookBlockers(Square from);
    U64 getBishopAttacks(Square from, U64 blockers, bool captures);
    U64 getRookAttacks(Square from, U64 blockers, bool captures);

};


#endif //KARL_MAGICS_H
