//
// Created by Joe Chrisman on 4/11/23.
//

#ifndef KARL_EVAL_H
#define KARL_EVAL_H

#include "Defs.h"

typedef short Score;

inline Score evaluate(const Score material, const Score midgamePlacement)
{
    return material + midgamePlacement;
}

inline constexpr Score MAX_SCORE = 30000;
inline constexpr Score MIN_SCORE = -30000;
inline constexpr Score CONTEMPT = 250;

inline constexpr Score PIECE_SCORES[13] = {
        0,    // NULL_PIECE
        100,  // WHITE_PAWN
        310,  // WHITE_KNIGHT
        380,  // WHITE_BISHOP
        500,  // WHITE_ROOK
        940,  // WHITE_QUEEN
        0,    // WHITE_KING
        -100, // BLACK_PAWN
        -310, // BLACK_KNIGHT
        -380, // BLACK_BISHOP
        -500, // BLACK_ROOK
        -940, // BLACK_QUEEN
        0     // BLACK_KING
};

inline constexpr Score MIDGAME_PLACEMENT_SCORES[13][64] = {
        // NULL_PIECE
        {
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
        },
        // WHITE_PAWN
        {
                 0,   0,   0,   0,   0,   0,   0,   0,
                15,  15,  20,  20,  20,  20,  15,  15,
                 5,  10,  10,  15,  15,  10,  10,   5,
                 5,   5,  10,  15,  15,  10,   5,   5,
                 0,   0,   5,  20,  20,   5,   0,   0,
                 1,   1, -10,   2,   2, -10,   1,   1,
                 1,   2,   2, -20, -20,   2,   2,   1,
                 0,   0,   0,   0,   0,   0,   0,   0
        },
        // WHITE_KNIGHT
        {
                -10, -10,  -5,  -5,  -5,  -5, -10, -10,
                -10,   0,   0,   5,   5,   0,   0, -10,
                 -5,   0,  15,  10,  10,  15,   0,  -5,
                 -5,   0,  15,  20,  20,  15,   0,  -5,
                 -5,   0,  15,  20,  20,  15,   0,  -5,
                 -5,   0,  15,  15,  15,  15,   0,  -5,
                -10,   0,   0,   0,   0,   0,   0, -10,
                -10, -10,  -5,  -5,  -5,  -5, -10, -10
        },
        // WHITE_BISHOP
        {
                 0,   0,   0,   0,   0,   0,   0,   0,
                 0,   0,   2,   2,   2,   2,   0,   0,
                 0,   0,  15,  15,  15,  15,   0,   0,
                 0,   0,  10,  10,  10,  10,   0,   0,
                 0,   5,  15,   5,   5,  15,   5,   0,
                 0,   5,  10,   0,   0,  10,   5,   0,
                 0,  10,   0,   5,   5,   0,  10,   0,
                -5,  -5, -10,  -5,  -5, -10,  -5,  -5
        },
        // WHITE_ROOK
        {
                  0,   0,   0,   0,   0,   0,   0,   0,
                 30,  30,  40,  40,  40,  40,  30,  30,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
                -10,   0,  30,  35,  35,  30,   0, -10,
        },
        // WHITE_QUEEN
        {
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,  30,   0,   0,   0,   0,
        },
        // WHITE_KING
        {
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0, -50, -50, -50,   0,   0,
                0,  99,  80, -50,   0, -50,  99,   0,
        },
        // BLACK_PAWN
        {
                  0,   0,   0,   0,   0,   0,   0,   0,
                 -1,  -2,  -2,  20,  20,  -2,  -2,  -1,
                 -1,  -1,  10,  -2,  -2,  10,  -1,  -1,
                  0,   0,  -5, -20, -20,  -5,   0,   0,
                 -5,  -5, -10, -15, -15, -10,  -5,  -5,
                 -5, -10, -10, -15, -15, -10, -10,  -5,
                -15, -15, -20, -20, -20, -20, -15, -15,
                  0,   0,   0,   0,   0,   0,   0,   0
        },
        // BLACK_KNIGHT
        {
                10,  10,   5,   5,   5,   5,  10,  10,
                10,   0,   0,   0,   0,   0,   0,  10,
                 5,   0, -15, -15, -15, -15,   0,   5,
                 5,   0, -15, -20, -20, -15,   0,   5,
                 5,   0, -15, -20, -20, -15,   0,   5,
                 5,   0, -15, -10, -10, -15,   0,   5,
                10,   0,   0,  -5,  -5,   0,   0,  10,
                10,  10,   5,   5,   5,   5,  10,  10,
        },
        // BLACK_BISHOP
        {
                5,   5,  10,   5,   5,  10,   5,   5,
                0, -10,   0,  -5,  -5,   0, -10,   0,
                0,  -5, -10,   0,   0, -10,  -5,   0,
                0,  -5, -15,  -5,  -5, -15,  -5,   0,
                0,   0, -10, -10, -10, -10,   0,   0,
                0,   0, -15, -15, -15, -15,   0,   0,
                0,   0,  -2,  -2,  -2,  -2,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0
        },
        // BLACK_ROOK
        {
                 10,   0, -30, -35, -35, -30,   0,  10,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
                  0,   0,   0,   0,   0,   0,   0,   0,
                -30, -30, -40, -40, -40, -40, -30, -30,
                  0,   0,   0,   0,   0,   0,   0,   0,
        },
        // BLACK_QUEEN
        {
                0,   0,   0, -30,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
        },
        // BLACK_KING
        {
                0, -99, -80,  50,   0,  50, -99,   0,
                0,   0,   0,  50,  50,  50,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0,
                0,   0,   0,   0,   0,   0,   0,   0
        },
};


#endif //KARL_EVAL_H
