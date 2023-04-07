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

template<bool quiets>
void MoveGenerator::generateMoves()
{
    moveList.clear();
    if (position.whiteToMove)
    {
        generateMoves<true, quiets>();
    }
    else
    {
        generateMoves<false, quiets>();
    }
}

template<bool isWhite, bool quiets>
void MoveGenerator::generateMoves()
{
    updateSafeSquares<isWhite>();
    updateResolverSquares<isWhite>();
    updatePins<isWhite, true>();
    updatePins<isWhite, false>();

    //genPawnMoves<isWhite, quiets>();
    genKnightMoves<isWhite, quiets>();
    //genKingMoves<isWhite, quiets>();
    //genRookMoves<isWhite, quiets>();
    //genBishopMoves<isWhite, quiets>();
    //genQueenMoves<isWhite, quiets>();
}

template<const bool isWhite, const bool quiets>
void MoveGenerator::genKnightMoves()
{
    U64 knights = position.bitboards[isWhite ? WHITE_KNIGHT : BLACK_KNIGHT];
    while (knights)
    {
        Square from = popFirstPiece(knights);
        U64 moves = knightMoves[from];
        if (quiets)
        {
            // quiet moves and captures
            moves &= (isWhite ? position.blackOrEmpty : position.whiteOrEmpty);
        }
        else
        {
            // captures only
            moves &= (isWhite ? position.blackPieces : position.whitePieces);
        }

        while (moves)
        {
            Square to = popFirstPiece(moves);
            moveList.emplace_back(createMove(
                    NORMAL,
                    isWhite ? WHITE_KNIGHT : BLACK_KNIGHT,
                    position.pieces[to],
                    from,
                    to
            ));
        }
    }
}



template<bool isWhite>
void MoveGenerator::updateSafeSquares()
{

}

template<bool isWhite>
void MoveGenerator::updateResolverSquares()
{

}

template<bool isWhite, bool isCardinal>
void MoveGenerator::updatePins()
{

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



