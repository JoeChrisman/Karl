//
// Created by Joe Chrisman on 4/6/23.
//

#include "MoveGenerator.h"

MoveGenerator::MoveGenerator() = default;

MoveGenerator::MoveGenerator(Position& _position) :
        position(_position)
{
    this->initKingMoves();
    this->initKnightMoves();
}

template<bool quiets>
void MoveGenerator::generateMoves()
{
    this->moveList.clear();
    if (position.whiteToMove)
    {
        this->generateMoves<true, quiets>();
    }
    else
    {
        this->generateMoves<false, quiets>();
    }
}

template<bool isWhite, bool quiets>
void MoveGenerator::generateMoves()
{
    this->updateSafeSquares<isWhite>();
    this->updateResolverSquares<isWhite>();
    this->updatePins<isWhite, true>();
    this->updatePins<isWhite, false>();

    this->genPawnMoves<isWhite, quiets>();
    this->genKnightMoves<isWhite, quiets>();
    this->genKingMoves<isWhite, quiets>();
    this->genRookMoves<isWhite, quiets>();
    this->genBishopMoves<isWhite, quiets>();
    this->genQueenMoves<isWhite, quiets>();
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
                knightMoves[square] |= toBoard(north<2>(west<1>(square)));
            }
            if (file < H_FILE)
            {
                knightMoves[square] |= toBoard(north<2>(east<1>(square)));
            }
        }
        if (file + 2 <= H_FILE)
        {
            if (rank < EIGHTH_RANK)
            {
                knightMoves[square] |= toBoard(east<2>(north<1>(square)));
            }
            if (rank > FIRST_RANK)
            {
                knightMoves[square] |= toBoard(east<2>(south<1>(square)));
            }
        }
        if (rank - 2 >= FIRST_RANK)
        {
            if (file > A_FILE)
            {
                knightMoves[square] |= toBoard(south<2>(west<1>(square)));
            }
            if (file < H_FILE)
            {
                knightMoves[square] |= toBoard(south<2>(east<1>(square)));

            }
        }
        if (file - 2 >= A_FILE)
        {
            if (rank < EIGHTH_RANK)
            {
                knightMoves[square] |= toBoard(west<2>(north<1>(square)));
            }
            if (rank > FIRST_RANK)
            {
                knightMoves[square] |= toBoard(west<2>(south<1>(square)));
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
                kingMoves[square] |= toBoard(north<1>(west<1>(square)));
            }
            kingMoves[square] |= toBoard(north<1>(square));
            if (file < H_FILE)
            {
                kingMoves[square] |= toBoard(north<1>(east<1>(square)));
            }
        }
        if (file > A_FILE)
        {
            kingMoves[square] |= toBoard(west<1>(square));
        }
        if (file < H_FILE)
        {
            kingMoves[square] |= toBoard(east<1>(square));
        }
        if (rank > FIRST_RANK)
        {
            if (file > A_FILE)
            {
                kingMoves[square] |= toBoard(south<1>(west<1>(square)));
            }
            kingMoves[square] |= toBoard(south<1>(square));
            if (file < H_FILE)
            {
                kingMoves[square] |= toBoard(south<1>(east<1>(square)));
            }
        }
    }
}



