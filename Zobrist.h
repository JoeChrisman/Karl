//
// Created by Joe Chrisman on 4/18/23.
//

#ifndef KARL_ZOBRIST_H
#define KARL_ZOBRIST_H

#include <random>
#include "Defs.h"

typedef unsigned long long Hash;

namespace Zobrist
{
    void init();

    extern Hash PIECES[64][13];
    extern Hash CASTLING[16];
    extern Hash EN_PASSANT[8];

    extern Hash WHITE_TO_MOVE;

};


#endif //KARL_ZOBRIST_H
