//
// Created by Joe Chrisman on 4/6/23.
//

#ifndef KARL_GEN_H
#define KARL_GEN_H

#include "Position.h"
#include "Magics.h"

namespace Gen
{
    void init();

    void genMoves();
    void genCaptures();

    extern std::vector<Move> moveList;
};


#endif //KARL_GEN_H
