//
// Created by Joe Chrisman on 4/6/23.
//

#include "MoveGenerator.h"

MoveGenerator::MoveGenerator(Position& _position) :
        position(_position)
{
    initKingMoves();
    initKnightMoves();
}

void MoveGenerator::initKnightMoves()
{
    for (Square square = A8; square <= H1; square++)
    {
        int rank = getRank(square);
        int file = getFile(square);
        if (rank + 2 <= EIGHTH_RANK)
        {
            if (file > A_FILE)
            {
                knightMoves[square] |= getBoard(north<2>(west<1>(square)));
            }
            if (file < H_FILE)
            {
                knightMoves[square] |= getBoard(north<2>(east<1>(square)));
            }
        }
        if (file + 2 <= H_FILE)
        {
            if (rank < EIGHTH_RANK)
            {
                knightMoves[square] |= getBoard(east<2>(north<1>(square)));
            }
            if (rank > FIRST_RANK)
            {
                knightMoves[square] |= getBoard(east<2>(south<1>(square)));
            }
        }
        if (rank - 2 >= FIRST_RANK)
        {
            if (file > A_FILE)
            {
                knightMoves[square] |= getBoard(south<2>(west<1>(square)));
            }
            if (file < H_FILE)
            {
                knightMoves[square] |= getBoard(south<2>(east<1>(square)));

            }
        }
        if (file - 2 >= A_FILE)
        {
            if (rank < EIGHTH_RANK)
            {
                knightMoves[square] |= getBoard(west<2>(north<1>(square)));
            }
            if (rank > FIRST_RANK)
            {
                knightMoves[square] |= getBoard(west<2>(south<1>(square)));
            }
        }
    }
}


void MoveGenerator::initKingMoves()
{
    for (Square square = A8; square <= H1; square++)
    {
        int rank = getRank(square);
        int file = getFile(square);
        if (rank < EIGHTH_RANK)
        {
            if (file > A_FILE)
            {
                kingMoves[square] |= getBoard(north<1>(west<1>(square)));
            }
            kingMoves[square] |= getBoard(north<1>(square));
            if (file < H_FILE)
            {
                kingMoves[square] |= getBoard(north<1>(east<1>(square)));
            }
        }
        if (file > A_FILE)
        {
            kingMoves[square] |= getBoard(west<1>(square));
        }
        if (file < H_FILE)
        {
            kingMoves[square] |= getBoard(east<1>(square));
        }
        if (rank > FIRST_RANK)
        {
            if (file > A_FILE)
            {
                kingMoves[square] |= getBoard(south<1>(west<1>(square)));
            }
            kingMoves[square] |= getBoard(south<1>(square));
            if (file < H_FILE)
            {
                kingMoves[square] |= getBoard(south<1>(east<1>(square)));
            }
        }
    }
}



