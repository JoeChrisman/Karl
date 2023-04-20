//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_SEARCH_H
#define KARL_SEARCH_H

#include "Eval.h"
#include "Gen.h"

namespace Search
{
    inline constexpr int MAX_DEPTH = 100;
    inline constexpr int MAX_MOVES = 1024;


    Move getBestMove();
};

#endif //KARL_SEARCH_H
