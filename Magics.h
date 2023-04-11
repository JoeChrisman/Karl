//
// Created by Joe Chrisman on 4/8/23.
//

#ifndef KARL_MAGICS_H
#define KARL_MAGICS_H

#include "Defs.h"

namespace Magics
{
    void init();

    struct MagicSquare
    {
        U64 blockers;
        U64 magic;
    };

    extern MagicSquare ordinalMagics[64];
    extern MagicSquare cardinalMagics[64];

    extern U64 cardinalAttacks[64][4096];
    extern U64 ordinalAttacks[64][512];
}


#endif //KARL_MAGICS_H
