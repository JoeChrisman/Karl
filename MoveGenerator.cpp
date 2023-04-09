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

    // create lookup tables for kings and knights
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


// generate quiet moves and captures
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

// generate captures only
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

// generate fully legal moves
template<bool isWhite, bool quiets>
void MoveGenerator::generate()
{
    moveList.clear();

    updateSafeSquares<isWhite>();
    updateResolverSquares<isWhite>();
    updatePins<isWhite, true>();
    updatePins<isWhite, false>();
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
    // push the pawns one square
    U64 pushed1 = (isWhite ? north(pawns) : south(pawns));
    // calculate left captures
    U64 leftCaptures = (isWhite ? west(pushed1 & leftCaptureMask)
                                : east(pushed1 & leftCaptureMask));
    // calculate right captures
    U64 rightCaptures = (isWhite ? east(pushed1 & rightCaptureMask)
                                 : west(pushed1 & rightCaptureMask));
    // make sure the captures actually capture something
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

    knights &= ~(ordinalPins | cardinalPins);

    while (knights)
    {
        Square from = popFirstPiece(knights);
        U64 moves = knightMoves[from];
        if constexpr (quiets)
        {
            moves &= (isWhite ? position.blackOrEmpty : position.whiteOrEmpty);
        }
        else
        {
            moves &= (isWhite ? position.blackPieces : position.whitePieces);
        }
        moves &= resolverSquares;
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

    bishops &= ~cardinalPins;

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
        moves &= resolverSquares;
        if (getBoard(from) & ordinalPins)
        {
            moves &= ordinalPins;
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

    rooks &= ~ordinalPins;

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
        moves &= resolverSquares;
        if (getBoard(from) & cardinalPins)
        {
            moves &= cardinalPins;
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
    U64 moves = EMPTY_BOARD;
    while (queens)
    {
        Square from = popFirstPiece(queens);
        U64 queen = getBoard(from);
        if (queen & ~cardinalPins)
        {
            moves |= getSlidingMoves<false>(from);
            if (queen & ordinalPins)
            {
                moves &= ordinalPins;
            }
        }
        if (queen & ~ordinalPins)
        {
            moves |= getSlidingMoves<true>(from);
            if (queen & cardinalPins)
            {
                moves &= cardinalPins;
            }
        }
        if constexpr (quiets)
        {
            moves &= isWhite ? position.blackOrEmpty : position.whiteOrEmpty;
        }
        else
        {
            moves &= isWhite ? position.blackPieces : position.whitePieces;
        }
        moves &= resolverSquares;
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
    moves &= safeSquares;
    if constexpr (quiets)
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

template<bool isWhite>
void MoveGenerator::updateSafeSquares()
{
    U64 attackedSquares = EMPTY_BOARD;

    const U64 king = position.bitboards[isWhite ? WHITE_KING : BLACK_KING];
    position.occupiedSquares ^= king;

    U64 cardinalAttackers = position.bitboards[isWhite ? BLACK_ROOK : WHITE_ROOK] |
                            position.bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];
    while (cardinalAttackers)
    {
        attackedSquares |= getSlidingMoves<true>(popFirstPiece(cardinalAttackers));
    }
    U64 ordinalAttackers = position.bitboards[isWhite ? BLACK_BISHOP : WHITE_BISHOP] |
                           position.bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];
    while (ordinalAttackers)
    {
        attackedSquares |= getSlidingMoves<false>(popFirstPiece(ordinalAttackers));
    }

    position.occupiedSquares ^= king;

    const U64 enemyPawns = position.bitboards[isWhite ? BLACK_PAWN : WHITE_PAWN];
    U64 eastAttacks = isWhite ? southEast(enemyPawns) : northEast(enemyPawns);
    eastAttacks &= ~FILE_MASKS[A_FILE];
    U64 westAttacks = isWhite ? southWest(enemyPawns) : northWest(enemyPawns);
    westAttacks &= ~FILE_MASKS[H_FILE];
    attackedSquares |= eastAttacks | westAttacks;

    U64 enemyKnights = position.bitboards[isWhite ? BLACK_KNIGHT : WHITE_KNIGHT];
    while (enemyKnights)
    {
        attackedSquares |= knightMoves[popFirstPiece(enemyKnights)];
    }

    U64 enemyKing = position.bitboards[isWhite ? BLACK_KING : WHITE_KING];
    attackedSquares |= kingMoves[getSquare(enemyKing)];

    safeSquares = ~attackedSquares;
    printBitboard(attackedSquares);
}

template<bool isWhite>
void MoveGenerator::updateResolverSquares()
{
    const Square king = getSquare(position.bitboards[isWhite ? WHITE_KING : BLACK_KING]);

    const U64 cardinalAttacks = getSlidingMoves<true>(king);
    const U64 ordinalAttacks = getSlidingMoves<false>(king);

    U64 cardinalAttackers = isWhite ? position.bitboards[BLACK_ROOK] : position.bitboards[WHITE_ROOK];
    U64 ordinalAttackers = isWhite ? position.bitboards[BLACK_BISHOP] : position.bitboards[WHITE_BISHOP];
    cardinalAttackers |= isWhite ? position.bitboards[BLACK_QUEEN] : position.bitboards[WHITE_QUEEN];
    ordinalAttackers |= isWhite ? position.bitboards[BLACK_QUEEN] : position.bitboards[WHITE_QUEEN];
    cardinalAttackers &= cardinalAttacks;
    ordinalAttackers &= ordinalAttacks;

    U64 attackers = cardinalAttackers | ordinalAttackers;
    attackers |= knightMoves[king] & position.bitboards[isWhite ? BLACK_KNIGHT : WHITE_KNIGHT];

    U64 eastAttacks = isWhite ? northEast(king) : southEast(king);
    eastAttacks &= ~FILE_MASKS[A_FILE];
    U64 westAttacks = isWhite ? northWest(king) : southWest(king);
    westAttacks &= ~FILE_MASKS[H_FILE];
    attackers |= position.bitboards[isWhite ? BLACK_PAWN : WHITE_PAWN] & (eastAttacks | westAttacks);

    if (attackers)
    {
        if (getNumPieces(attackers) == 1)
        {
            if (cardinalAttackers)
            {
                resolverSquares = cardinalAttacks & getSlidingMoves<true>(getSquare(attackers));
                resolverSquares |= attackers;
            }
            else if (ordinalAttackers)
            {
                resolverSquares = ordinalAttacks & getSlidingMoves<false>(getSquare(attackers));
                resolverSquares |= attackers;
            }
            else
            {
                resolverSquares = attackers;
            }
        }
        else
        {
            resolverSquares = EMPTY_BOARD;
        }
    }
    else
    {
        resolverSquares = FULL_BOARD;
    }
}

template<bool isWhite, bool isCardinal>
void MoveGenerator::updatePins()
{
    if constexpr (isCardinal)
    {
        cardinalPins = EMPTY_BOARD;
    }
    else
    {
        ordinalPins = EMPTY_BOARD;
    }

    const Square king = getSquare(position.bitboards[isWhite ? WHITE_KING : BLACK_KING]);

    U64 pinned = getSlidingMoves<isCardinal>(king);
    pinned &= isWhite ? position.whitePieces : position.blackPieces;

    position.occupiedSquares ^= pinned;

    U64 pins = getSlidingMoves<isCardinal>(king);

    U64 pinners = position.bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];
    if constexpr (isCardinal)
    {
        pinners |=  position.bitboards[isWhite ? BLACK_ROOK : WHITE_ROOK];
    }
    else
    {
        pinners |=  position.bitboards[isWhite ? BLACK_BISHOP : WHITE_BISHOP];
    }
    pinners &= pins;

    while (pinners)
    {
        const Square pinner = popFirstPiece(pinners);
        U64 pin = getSlidingMoves<isCardinal>(pinner);
        pin &= pins;
        pin |= getBoard(pinner);

        if constexpr (isCardinal)
        {
            cardinalPins |= pin;
        }
        else
        {
            ordinalPins |= pin;
        }
    }
    position.occupiedSquares ^= pinned;
}