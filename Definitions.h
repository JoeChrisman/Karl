//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_DEFINITIONS_H
#define KARL_DEFINITIONS_H

#include <string>

typedef unsigned long long U64;
typedef int Piece;
typedef int Square;

const U64 EMPTY_BOARD = 0;
const U64 FULL_BOARD = ~0;


const Piece NULL_PIECE = -1;
const Square NULL_SQUARE = -1;


const U64 FILE_MASKS[8] = {
        0x0101010101010101,
        0x0202020202020202,
        0x0404040404040404,
        0x0808080808080808,
        0x1010101010101010,
        0x2020202020202020,
        0x4040404040404040,
        0x8080808080808080,
};

enum
{
    A_FILE,
    B_FILE,
    C_FILE,
    D_FILE,
    E_FILE,
    F_FILE,
    G_FILE,
    H_FILE
};

enum
{
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING
};

enum
{
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1
};

inline U64 toBoard(Square square)
{
    return (U64)1 << square;
}

#endif //KARL_DEFINITIONS_H
