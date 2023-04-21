//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_SEARCH_H
#define KARL_SEARCH_H

#include "Eval.h"
#include "Gen.h"

namespace Search
{
    void init();
    Move getBestMove();

    inline constexpr int MAX_DEPTH = 100;

};

#endif //KARL_SEARCH_H
