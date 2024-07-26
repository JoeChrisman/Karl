//
// Created by Joe Chrisman on 4/18/23.
//

#include "Zobrist.h"

Hash Zobrist::getRandomBits(const int size)
{
    const Hash random = static_cast<Hash>(
            (static_cast<Hash>(rand()))) |
            (static_cast<Hash>(rand()) << 32);

    return random & (0xffffffffffffffff >> (64 - size));
}

Zobrist::Zobrist()
: PIECES{0}, CASTLING{0}, EN_PASSANT{0}
{
    WHITE_TO_MOVE = getRandomBits(64);

    for (int castlingFlag = 0; castlingFlag < 16; castlingFlag++)
    {
        CASTLING[castlingFlag] = getRandomBits(64);
    }

    for (int file = A_FILE; file < H_FILE; file++)
    {
        EN_PASSANT[file] = getRandomBits(64);
    }

    for (Square square = A8; square <= H1; square++)
    {
        for (Piece piece = NULL_PIECE; piece <= BLACK_KING; piece++)
        {
            if (piece == WHITE_PAWN || piece == BLACK_PAWN)
            {
                PIECES[square][piece] = getRandomBits(64);
            }
            else
            {
                PIECES[square][piece] = getRandomBits(64);
            }
        }
    }
}
