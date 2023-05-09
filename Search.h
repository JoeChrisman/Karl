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
    Move searchByDepth(const int depth);
    Move searchByTime(const int millis);
    Move searchByTimeControl(
            const int whiteRemaining,
            const int blackRemaining,
            const int whiteIncrement,
            const int blackIncrement);

    extern long endTime;
    extern bool isOutOfTime;

    inline constexpr int MAX_DEPTH = 100;

};

#endif //KARL_SEARCH_H
