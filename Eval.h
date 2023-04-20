//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_EVAL_H
#define KARL_EVAL_H

#include "Defs.h"

typedef short Score;

namespace Eval
{
    Score evaluate(Score material);

    inline constexpr Score MAX_SCORE = 30000;
    inline constexpr Score MIN_SCORE = -30000;
    inline constexpr Score DRAW_SCORE = 0;
    inline constexpr Score CONTEMPT = 400;

    inline constexpr Score PIECE_SCORES[13] = {
            0,    // NULL_PIECE
            100,  // WHITE_PAWN
            310,  // WHITE_KNIGHT
            380,  // WHITE_BISHOP
            500,  // WHITE_ROOK
            860,  // WHITE_QUEEN
            0,    // WHITE_KING
            -100, // BLACK_PAWN
            -310, // BLACK_KNIGHT
            -380, // BLACK_BISHOP
            -500, // BLACK_ROOK
            -860, // BLACK_QUEEN
            0     // BLACK_KING
    };
};


#endif //KARL_EVAL_H
