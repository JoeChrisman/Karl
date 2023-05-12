//
// Created by Joe Chrisman on 4/18/23.
//

#include "Zobrist.h"

Hash Zobrist::getRandomHash()
{
    return ((Hash)(rand()) << 32) | rand();
}

Zobrist::Zobrist()
: PIECES{0}, CASTLING{0}, EN_PASSANT{0}
{
    WHITE_TO_MOVE = getRandomHash();

    for (int castlingFlag = 0; castlingFlag < 16; castlingFlag++)
    {
        CASTLING[castlingFlag] = getRandomHash();
    }

    for (int file = A_FILE; file < H_FILE; file++)
    {
        EN_PASSANT[file] = getRandomHash();
    }

    for (Square square = A8; square <= H1; square++)
    {
        for (Piece piece = NULL_PIECE; piece <= BLACK_KING; piece++)
        {
            PIECES[square][piece] = getRandomHash();
        }
    }
}
