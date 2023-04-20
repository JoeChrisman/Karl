//
// Created by Joe Chrisman on 4/18/23.
//

#include "Zobrist.h"

Hash Zobrist::PIECES[64][13];
Hash Zobrist::CASTLING[16];
Hash Zobrist::EN_PASSANT[8];

Hash Zobrist::WHITE_TO_MOVE;

namespace
{
    Hash getRandomHash()
    {
        return ((Hash)(rand()) << 32) | rand();
    }
}

void Zobrist::init()
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
