//
// Created by Joe Chrisman on 4/6/23.
//

#include "MoveGenerator.h"

MoveGenerator::MoveGenerator(Position& _position) :
        position(_position)
{
    resolverSquares = FULL_BOARD;
    cardinalPins = EMPTY_BOARD;
    ordinalPins = EMPTY_BOARD;

    initKingMoves();
    initKnightMoves();
}

void MoveGenerator::generateMoves()
{
    if (position.isWhiteToMove)
    {
        generate<true, true>();
    }
    else
    {
        generate<false, true>();
    }
}

void MoveGenerator::generateCaptures()
{
    if (position.isWhiteToMove)
    {
        generate<true, false>();
    }
    else
    {
        generate<false, false>();
    }
}

template<bool isWhite, bool quiets>
void MoveGenerator::generate()
{
    moveList.clear();

    //updateSafeSquares<isWhite>();
    //updateResolverSquares<isWhite>();
    //updatePins<isWhite, true>();
    //updatePins<isWhite, false>();
    genPawnMoves<isWhite, quiets>();
    genKnightMoves<isWhite, quiets>();
    genKingMoves<isWhite, quiets>();
    genRookMoves<isWhite, quiets>();
    genBishopMoves<isWhite, quiets>();
    genQueenMoves<isWhite, quiets>();
}

template<bool isWhite, bool quiets>
void MoveGenerator::genPawnMoves()
{
    constexpr Piece pieceMoving = isWhite ? WHITE_PAWN : BLACK_PAWN;
    constexpr U64 leftCaptureMask = ~FILE_MASKS[isWhite ? A_FILE : H_FILE];
    constexpr U64 rightCaptureMask = ~FILE_MASKS[isWhite ? H_FILE : F_FILE];
    constexpr U64 promotionMask = RANK_MASKS[isWhite ? EIGHTH_RANK : FIRST_RANK];
    constexpr U64 notPromotionMask = ~promotionMask;

    const U64 pawns = position.bitboards[pieceMoving];

    U64 pushed1 = (isWhite ? north(pawns) : south(pawns));

    U64 leftCaptures = (isWhite ? west(pushed1 & leftCaptureMask)
                                : east(pushed1 & leftCaptureMask));
    U64 rightCaptures = (isWhite ? east(pushed1 & rightCaptureMask)
                                 : west(pushed1 & rightCaptureMask));
    leftCaptures &= (isWhite ? position.blackPieces : position.whitePieces);
    rightCaptures &= (isWhite ? position.blackPieces : position.whitePieces);

    uint64_t rightCapturePromotions = rightCaptures & promotionMask;
    uint64_t leftCapturePromotions = leftCaptures & promotionMask;

    rightCaptures &= notPromotionMask;
    leftCaptures &= notPromotionMask;

    while (leftCaptures)
    {
        Square to = popFirstPiece(leftCaptures);
        Square from = isWhite ? southEast(to) : northWest(to);
        moveList.emplace_back(createMove(
                NORMAL,
                pieceMoving,
                position.pieces[to],
                from,
                to
        ));
    }
    while (rightCaptures)
    {
        Square to = popFirstPiece(rightCaptures);
        Square from = isWhite ? southWest(to) : northEast(to);
        moveList.emplace_back(createMove(
                NORMAL,
                pieceMoving,
                position.pieces[to],
                from,
                to
        ));
    }
    while (leftCapturePromotions)
    {
        Square to = popFirstPiece(leftCapturePromotions);
        Square from = isWhite ? southEast(to) : northWest(to);
        genPromotions<isWhite>(from, to, position.pieces[to]);
    }
    while (rightCapturePromotions)
    {
        Square to = popFirstPiece(rightCapturePromotions);
        Square from = isWhite ? southWest(to) : northEast(to);
        genPromotions<isWhite>(from, to, position.pieces[to]);
    }

    pushed1 &= position.emptySquares;
    U64 pushPromotions = pushed1 & promotionMask;
    pushed1 &= notPromotionMask;

    while (pushPromotions)
    {
        Square to = popFirstPiece(pushPromotions);
        Square from = isWhite ? south(to) : north(to);
        genPromotions<isWhite>(from, to, NULL_PIECE);
    }

    // generate non-captures and non-promotions
    if constexpr (quiets)
    {

        constexpr U64 doublePushMask = RANK_MASKS[isWhite ? THIRD_RANK : SIXTH_RANK];
        // calculate two square pawn pushes
        // we can only push a pawn two squares if it has not been moved yet
        U64 pushed2 = pushed1 & doublePushMask;
        pushed2 = isWhite ? north(pushed2) : south(pushed2);
        pushed2 &= position.emptySquares;

        while (pushed1)
        {
            Square to = popFirstPiece(pushed1);
            Square from = isWhite ? south(to) : north(to);
            moveList.emplace_back(createMove(
                    NORMAL,
                    pieceMoving,
                    NULL_PIECE,
                    from,
                    to
            ));
        }

        while (pushed2)
        {
            Square to = popFirstPiece(pushed2);
            Square from = isWhite ? south<2>(to) : north<2>(to);
            moveList.emplace_back(createMove(
                    NORMAL,
                    pieceMoving,
                    NULL_PIECE,
                    from,
                    to
            ));
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGenerator::genKnightMoves()
{
    constexpr Piece pieceMoving = isWhite ? WHITE_KNIGHT : BLACK_KNIGHT;

    U64 knights = position.bitboards[pieceMoving];
    while (knights)
    {
        Square from = popFirstPiece(knights);
        U64 moves = knightMoves[from];
        if constexpr (quiets)
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
                    pieceMoving,
                    position.pieces[to],
                    from,
                    to
            ));
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGenerator::genBishopMoves()
{
    constexpr Piece pieceMoving = isWhite ? WHITE_BISHOP : BLACK_BISHOP;
    U64 bishops = position.bitboards[pieceMoving];
    while (bishops)
    {
        Square from = popFirstPiece(bishops);
        U64 moves = getSlidingMoves<false>(from);
        if constexpr (quiets)
        {
            moves &= isWhite ? position.blackOrEmpty : position.whiteOrEmpty;
        }
        else
        {
            moves &= isWhite ? position.blackPieces : position.whitePieces;
        }
        while (moves)
        {
            Square to = popFirstPiece(moves);
            moveList.emplace_back(createMove(
                    NORMAL,
                    pieceMoving,
                    position.pieces[to],
                    from,
                    to
            ));
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGenerator::genRookMoves()
{
    constexpr Piece pieceMoving = isWhite ? WHITE_ROOK : BLACK_ROOK;
    U64 rooks = position.bitboards[pieceMoving];
    while (rooks)
    {
        Square from = popFirstPiece(rooks);
        U64 moves = getSlidingMoves<true>(from);
        if constexpr (quiets)
        {
            moves &= isWhite ? position.blackOrEmpty : position.whiteOrEmpty;
        }
        else
        {
            moves &= isWhite ? position.blackPieces : position.whitePieces;
        }
        while (moves)
        {
            Square to = popFirstPiece(moves);
            moveList.emplace_back(createMove(
                    NORMAL,
                    pieceMoving,
                    position.pieces[to],
                    from,
                    to
            ));
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGenerator::genQueenMoves()
{
    constexpr Piece pieceMoving = isWhite ? WHITE_QUEEN : BLACK_QUEEN;

    U64 queens = position.bitboards[pieceMoving];
    while (queens)
    {
        Square from = popFirstPiece(queens);
        U64 moves = getSlidingMoves<true>(from) |
                    getSlidingMoves<false>(from);
        if constexpr (quiets)
        {
            moves &= isWhite ? position.blackOrEmpty : position.whiteOrEmpty;
        }
        else
        {
            moves &= isWhite ? position.blackPieces : position.whitePieces;
        }
        while (moves)
        {
            Square to = popFirstPiece(moves);
            moveList.push_back(createMove(
                    NORMAL,
                    pieceMoving,
                    position.pieces[to],
                    from,
                    to
            ));
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGenerator::genKingMoves()
{
    Square from = getSquare(position.bitboards[isWhite ? WHITE_KING : BLACK_KING]);
    U64 moves = kingMoves[from];
    if (quiets)
    {
        moves &= (isWhite ? position.blackOrEmpty : position.whiteOrEmpty);
    }
    else
    {
        moves &= (isWhite ? position.blackPieces : position.whitePieces);
    }
    while (moves)
    {
        Square to = popFirstPiece(moves);
        moveList.push_back(createMove(
                NORMAL,
                isWhite ? WHITE_KING : BLACK_KING,
                position.pieces[to],
                from,
                to
        ));
    }
}


template<bool isWhite>
void MoveGenerator::genPromotions(const Square from, const Square to, const Piece captured)
{
    for (int type = KNIGHT_PROMOTION; type <= QUEEN_PROMOTION; type++)
    {
        moveList.emplace_back(createMove(
                (MoveType)type,
                isWhite ? WHITE_PAWN : BLACK_PAWN,
                captured,
                from,
                to
        ));
    }
}

template<bool isCardinal>
U64 MoveGenerator::getSlidingMoves(Square from)
{
    if constexpr (isCardinal)
    {
        MagicSliders::MagicSquare square = MagicSliders::cardinalMagics[from];
        U64 blockers = square.blockers & position.occupiedSquares;
        return MagicSliders::cardinalAttacks[from][blockers * square.magic >> 52];
    }
    else
    {
        MagicSliders::MagicSquare square = MagicSliders::ordinalMagics[from];
        U64 blockers = square.blockers & position.occupiedSquares;
        return MagicSliders::ordinalAttacks[from][blockers * square.magic >> 55];
    }
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
                knightMoves[square] |= getBoard(north<2>(west(square)));
            }
            if (file < H_FILE)
            {
                knightMoves[square] |= getBoard(north<2>(east(square)));
            }
        }
        if (file + 2 <= H_FILE)
        {
            if (rank < EIGHTH_RANK)
            {
                knightMoves[square] |= getBoard(east<2>(north(square)));
            }
            if (rank > FIRST_RANK)
            {
                knightMoves[square] |= getBoard(east<2>(south(square)));
            }
        }
        if (rank - 2 >= FIRST_RANK)
        {
            if (file > A_FILE)
            {
                knightMoves[square] |= getBoard(south<2>(west(square)));
            }
            if (file < H_FILE)
            {
                knightMoves[square] |= getBoard(south<2>(east(square)));

            }
        }
        if (file - 2 >= A_FILE)
        {
            if (rank < EIGHTH_RANK)
            {
                knightMoves[square] |= getBoard(west<2>(north(square)));
            }
            if (rank > FIRST_RANK)
            {
                knightMoves[square] |= getBoard(west<2>(south(square)));
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
                kingMoves[square] |= getBoard(northWest(square));
            }
            kingMoves[square] |= getBoard(north(square));
            if (file < H_FILE)
            {
                kingMoves[square] |= getBoard(northEast(square));
            }
        }
        if (file > A_FILE)
        {
            kingMoves[square] |= getBoard(west(square));
        }
        if (file < H_FILE)
        {
            kingMoves[square] |= getBoard(east(square));
        }
        if (rank > FIRST_RANK)
        {
            if (file > A_FILE)
            {
                kingMoves[square] |= getBoard(southWest(square));
            }
            kingMoves[square] |= getBoard(south(square));
            if (file < H_FILE)
            {
                kingMoves[square] |= getBoard(southEast(square));
            }
        }
    }
}



