//
// Created by Joe Chrisman on 4/8/23.
//

#ifndef KARL_MAGICSLIDERS_H
#define KARL_MAGICSLIDERS_H

#include "Definitions.h"

namespace MagicSliders
{
    extern void init();

    struct MagicSquare
    {
        U64 blockers;
        U64 magic;
    };

    extern MagicSquare ordinalMagics[64];
    extern MagicSquare cardinalMagics[64];

    extern U64 cardinalAttacks[64][4096];
    extern U64 ordinalAttacks[64][512];

    extern U64 getBishopAttacks(Square from, U64 blockers, bool captures);
    extern U64 getRookAttacks(Square from, U64 blockers, bool captures);

    extern U64 getRookBlockers(Square from);
    extern U64 getBishopBlockers(Square from);

    extern U64 getMagicNumber(Square square, bool isCardinal);

    extern U64 random64();
}


#endif //KARL_MAGICSLIDERS_H
