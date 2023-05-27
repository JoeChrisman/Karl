//
// Created by Joe Chrisman on 4/18/23.
//

#ifndef KARL_ZOBRIST_H
#define KARL_ZOBRIST_H

#include <random>
#include "Defs.h"

typedef long long Hash;

class Zobrist
{
public:
    Zobrist();

    Hash PIECES[64][13];
    Hash CASTLING[16];
    Hash EN_PASSANT[8];
    Hash WHITE_TO_MOVE;

private:
    static Hash getRandomHash();

};


#endif //KARL_ZOBRIST_H
