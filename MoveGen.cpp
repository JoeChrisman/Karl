//
// Created by Joe Chrisman on 4/6/23.
//

#include "MoveGen.h"


MoveGen::MoveGen(Position& position, const Magics& magics)
: magics(magics), position(position), moveList{0}, knightMoves{0}, kingMoves{0}
{
    std::memset(moveList, NULL_MOVE, sizeof(moveList));
    numMoves = 0;
    initKnightMoves();
    initKingMoves();

    resolverSquares = EMPTY_BOARD;
    safeSquares = EMPTY_BOARD;
    cardinalPins = EMPTY_BOARD;
    ordinalPins = EMPTY_BOARD;
}

void MoveGen::initKnightMoves()
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

void MoveGen::initKingMoves()
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

template<bool isCardinal>
U64 MoveGen::getSlidingMoves(Square from)
{
    if constexpr (isCardinal)
    {
        const MagicSquare square = magics.cardinalMagics[from];
        U64 blockers = square.blockers & position.occupiedSquares;
        return magics.cardinalAttacks[from][blockers * square.magic >> 52];
    }
    else
    {
        const MagicSquare square = magics.ordinalMagics[from];
        U64 blockers = square.blockers & position.occupiedSquares;
        return magics.ordinalAttacks[from][blockers * square.magic >> 55];
    }
}

template<bool isWhite>
void MoveGen::genPromotions(const Square from, const Square to, const Piece captured)
{
    static constexpr Piece pieceMoving = isWhite ? WHITE_PAWN : BLACK_PAWN;

    for (int promoted = isWhite ? WHITE_KNIGHT : BLACK_KNIGHT;
         promoted <= (isWhite ? WHITE_QUEEN : BLACK_QUEEN);
         promoted++)
    {
        moveList[numMoves++] = createMove(from, to, pieceMoving, captured, promoted);
    }
}

template<bool isWhite>
bool MoveGen::isEnPassantHorizontallyPinned(const Square from, const Square to)
{
    static constexpr U64 enPassantRank = RANKS[isWhite ? FIFTH_RANK : FOURTH_RANK];
    const U64 captured = getBoard(isWhite ? south(to) : north(to));

    const U64 horizontalAttackerMask =
            position.bitboards[isWhite ? WHITE_KING : BLACK_KING]
            | position.bitboards[isWhite ? BLACK_ROOK : WHITE_ROOK]
            | position.bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];

    position.occupiedSquares ^= captured;
    const U64 horizontalAttacks = getSlidingMoves<true>(from)
            & horizontalAttackerMask
            & enPassantRank;
    position.occupiedSquares |= captured;

    return getNumPieces(horizontalAttacks) == 2;
}

template<bool isWhite, bool isEast>
U64 MoveGen::getPawnCaptures(const U64 pawns)
{
    static constexpr U64 eastCaptureMask = ~FILES[A_FILE];
    static constexpr U64 westCaptureMask = ~FILES[H_FILE];
    if constexpr (isWhite)
    {
        if constexpr (isEast)
        {
            return northEast(pawns)
                & position.blackPieces
                & eastCaptureMask
                & resolverSquares;
        }
        else
        {
            return northWest(pawns)
                & position.blackPieces
                & westCaptureMask
                & resolverSquares;
        }
    }
    else
    {
        if constexpr (isEast)
        {
            return southEast(pawns)
                & position.whitePieces
                & eastCaptureMask
                & resolverSquares;
        }
        else
        {
            return southWest(pawns)
                & position.whitePieces
                & westCaptureMask
                & resolverSquares;
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGen::genPawnMoves()
{
    static constexpr Piece pieceMoving = isWhite ? WHITE_PAWN : BLACK_PAWN;
    static constexpr U64 beforePromotionRank = RANKS[isWhite ? SEVENTH_RANK : SECOND_RANK];
    static constexpr U64 promotionRank = RANKS[isWhite ? EIGHTH_RANK : FIRST_RANK];

    const U64 pawns = position.bitboards[pieceMoving];
    const U64 unpinnedPawns = pawns & ~(cardinalPins | ordinalPins);
    const U64 cardinalPinnedPawns = pawns & cardinalPins;
    const U64 ordinalPinnedPawns = pawns & ordinalPins;

    const U64 unpinnedEastCaptures = getPawnCaptures<isWhite, true>(unpinnedPawns);
    const U64 unpinnedWestCaptures = getPawnCaptures<isWhite, false>(unpinnedPawns);
    const U64 pinnedEastCaptures = getPawnCaptures<isWhite, true>(ordinalPinnedPawns) & ordinalPins;
    const U64 pinnedWestCaptures = getPawnCaptures<isWhite, false>(ordinalPinnedPawns) & ordinalPins;

    U64 eastCaptures = unpinnedEastCaptures | pinnedEastCaptures;
    U64 westCaptures = unpinnedWestCaptures | pinnedWestCaptures;

    const U64 unpinnedPawnPushes = (isWhite ? north(unpinnedPawns) : south(unpinnedPawns)) & position.emptySquares;

    // calculate pawn promotions
    if (pawns & beforePromotionRank)
    {
        U64 eastCapturePromotions = eastCaptures & promotionRank;
        U64 westCapturePromotions = westCaptures & promotionRank;

        while (eastCapturePromotions)
        {
            const Square to = popFirstPiece(eastCapturePromotions);
            const Square from = isWhite ? southWest(to) : northWest(to);
            genPromotions<isWhite>(from, to, position.pieces[to]);
        }
        while (westCapturePromotions)
        {
            const Square to = popFirstPiece(westCapturePromotions);
            const Square from = isWhite ? southEast(to) : northEast(to);
            genPromotions<isWhite>(from, to, position.pieces[to]);
        }
        U64 pushPromotions = unpinnedPawnPushes & promotionRank & resolverSquares;
        while (pushPromotions)
        {
            const Square to = popFirstPiece(pushPromotions);
            const Square from = isWhite ? south(to) : north(to);
            genPromotions<isWhite>(from, to, NULL_PIECE);
        }
    }
    westCaptures &= ~promotionRank;
    eastCaptures &= ~promotionRank;
    while (eastCaptures)
    {
        const Square to = popFirstPiece(eastCaptures);
        const Square from = isWhite ? southWest(to) : northWest(to);
        moveList[numMoves++] = createMove(from, to, pieceMoving, position.pieces[to]);
    }
    while (westCaptures)
    {
        const Square to = popFirstPiece(westCaptures);
        const Square from = isWhite ? southEast(to) : northEast(to);
        moveList[numMoves++] = createMove(from, to, pieceMoving, position.pieces[to]);
    }

    const int enPassantFile = position.irreversibles.enPassantFile;
    if (enPassantFile > -1)
    {
        static constexpr U64 eastCaptureMask = ~FILES[A_FILE];
        static constexpr U64 westCaptureMask = ~FILES[H_FILE];
        static constexpr Piece pawnCapturing = isWhite ? BLACK_PAWN : WHITE_PAWN;

        const U64 enPassantSquare = FILES[position.irreversibles.enPassantFile] &
                                    RANKS[isWhite ? SIXTH_RANK : THIRD_RANK];
        const U64 shiftedResolvers = isWhite ? north(resolverSquares) : south(resolverSquares);
        const U64 enPassantMask = enPassantSquare & shiftedResolvers;

        const U64 unpinnedWestCapture = isWhite ? northWest(unpinnedPawns) : southWest(unpinnedPawns);
        const U64 unpinnedEastCapture = isWhite ? northEast(unpinnedPawns) : southEast(unpinnedPawns);

        const U64 pinnedWestCapture = (isWhite ? northWest(ordinalPinnedPawns) : southWest(ordinalPinnedPawns))
                & ordinalPins;
        const U64 pinnedEastCapture = (isWhite ? northEast(ordinalPinnedPawns) : southEast(ordinalPinnedPawns))
                & ordinalPins;

        U64 westCapture = (unpinnedWestCapture | pinnedWestCapture) & enPassantMask & westCaptureMask;
        U64 eastCapture = (unpinnedEastCapture | pinnedEastCapture) & enPassantMask & eastCaptureMask;

        if (eastCapture)
        {
            const Square to = getSquare(eastCapture);
            const Square from = isWhite ? southWest(to) : northWest(to);
            if (!isEnPassantHorizontallyPinned<isWhite>(from, to))
            {
                moveList[numMoves++] = EN_PASSANT | createMove(from, to, pieceMoving, pawnCapturing);
            }
        }
        if (westCapture)
        {
            const Square to = getSquare(westCapture);
            const Square from = isWhite ? southEast(to) : northEast(to);
            if (!isEnPassantHorizontallyPinned<isWhite>(from, to))
            {
                moveList[numMoves++] = EN_PASSANT | createMove(from, to, pieceMoving, pawnCapturing);
            }
        }
    }
    if constexpr (quiets)
    {
        const U64 pinnedPawnPushes = (isWhite ? north(cardinalPinnedPawns) : south(cardinalPinnedPawns))
             & position.emptySquares
             & cardinalPins;

        const U64 pushed1 = (pinnedPawnPushes | unpinnedPawnPushes) & ~promotionRank;
        U64 singlePawnPushes = pushed1 & resolverSquares;
        while (singlePawnPushes)
        {
            const Square to = popFirstPiece(singlePawnPushes);
            const Square from = isWhite ? south(to) : north(to);
            moveList[numMoves++] = createMove(from, to, pieceMoving, NULL_PIECE);
        }

        U64 doublePawnPushes = (isWhite ? north(pushed1) : south(pushed1))
            & RANKS[isWhite ? FOURTH_RANK : FIFTH_RANK]
            & position.emptySquares
            & resolverSquares;
        while (doublePawnPushes)
        {
            const Square to = popFirstPiece(doublePawnPushes);
            const Square from = isWhite ? south<2>(to) : north<2>(to);
            moveList[numMoves++] = DOUBLE_PAWN_PUSH | createMove(from, to, pieceMoving, NULL_PIECE);
        }

    }
}

template<bool isWhite, bool quiets>
void MoveGen::genKnightMoves()
{
    static constexpr Piece pieceMoving = isWhite ? WHITE_KNIGHT : BLACK_KNIGHT;

    U64 knights = position.bitboards[isWhite ? WHITE_KNIGHT : BLACK_KNIGHT];
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
            moveList[numMoves++] = createMove(from, to, pieceMoving, position.pieces[to]);
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGen::genBishopMoves()
{
    static constexpr Piece pieceMoving = isWhite ? WHITE_BISHOP : BLACK_BISHOP;

    U64 bishops = position.bitboards[pieceMoving] & ~cardinalPins;
    while (bishops)
    {
        const Square from = popFirstPiece(bishops);
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
            const Square to = popFirstPiece(moves);
            moveList[numMoves++] = createMove(from, to, pieceMoving, position.pieces[to]);
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGen::genRookMoves()
{
    static constexpr Piece pieceMoving = isWhite ? WHITE_ROOK : BLACK_ROOK;

    U64 rooks = position.bitboards[pieceMoving] & ~ordinalPins;
    while (rooks)
    {
        const Square from = popFirstPiece(rooks);
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
            const Square to = popFirstPiece(moves);
            moveList[numMoves++] = createMove(from, to, pieceMoving, position.pieces[to]);
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGen::genQueenMoves()
{
    static constexpr Piece pieceMoving = isWhite ? WHITE_QUEEN : BLACK_QUEEN;
    U64 queens = position.bitboards[pieceMoving];
    U64 moves = EMPTY_BOARD;
    while (queens)
    {
        const Square from = popFirstPiece(queens);
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
            const Square to = popFirstPiece(moves);
            moveList[numMoves++] = createMove(from, to, pieceMoving, position.pieces[to]);
        }
    }
}

template<bool isWhite, bool quiets>
void MoveGen::genKingMoves()
{
    static constexpr Piece pieceMoving = isWhite ? WHITE_KING : BLACK_KING;

    const Square from = getSquare(position.bitboards[pieceMoving]);
    U64 moves = kingMoves[from] & safeSquares;
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
        const Square to = popFirstPiece(moves);
        moveList[numMoves++] = createMove(from, to, pieceMoving, position.pieces[to]);
    }

    if constexpr (quiets)
    {
        static constexpr int castleShort = isWhite ? WHITE_CASTLE_SHORT : BLACK_CASTLE_SHORT;
        static constexpr int castleLong = isWhite ? WHITE_CASTLE_LONG : BLACK_CASTLE_LONG;

        if (position.irreversibles.castlingFlags & castleShort)
        {
            static constexpr U64 shortSafeSquares = isWhite ? 0x7000000000000000 : 0x70;
            static constexpr U64 shortEmptySquares = isWhite ? 0x6000000000000000 : 0x60;
            if ((shortSafeSquares & safeSquares) == shortSafeSquares &&
                (shortEmptySquares & position.emptySquares) == shortEmptySquares)
            {
                const int to = isWhite ? G1 : G8;
                moveList[numMoves++] = SHORT_CASTLE | createMove(from, to, pieceMoving, NULL_PIECE);
            }
        }
        if (position.irreversibles.castlingFlags & castleLong)
        {
            static constexpr U64 longSafeSquares = isWhite ? 0x1c00000000000000 : 0x1c;
            static constexpr U64 longEmptySquares = isWhite ? 0xe00000000000000 : 0xe;
            if ((longSafeSquares & safeSquares) == longSafeSquares &&
                (longEmptySquares & position.emptySquares) == longEmptySquares)
            {
                const int to = isWhite ? C1 : C8;
                moveList[numMoves++] = LONG_CASTLE | createMove(from, to, pieceMoving, NULL_PIECE);
            }
        }
    }
}

template<bool isWhite>
void MoveGen::updateSafeSquares()
{
    U64 attackedSquares = EMPTY_BOARD;

    const U64 king = position.bitboards[isWhite ? WHITE_KING : BLACK_KING];
    const U64 enemyKing = position.bitboards[isWhite ? BLACK_KING : WHITE_KING];
    position.occupiedSquares ^= king;

    U64 cardinalAttackers = position.bitboards[isWhite ? BLACK_ROOK : WHITE_ROOK];
    U64 ordinalAttackers = position.bitboards[isWhite ? BLACK_BISHOP : WHITE_BISHOP];
    cardinalAttackers |= position.bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];
    ordinalAttackers |= position.bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];
    while (cardinalAttackers)
    {
        attackedSquares |= getSlidingMoves<true>(popFirstPiece(cardinalAttackers));
    }
    while (ordinalAttackers)
    {
        attackedSquares |= getSlidingMoves<false>(popFirstPiece(ordinalAttackers));
    }

    position.occupiedSquares ^= king;

    const U64 enemyPawns = position.bitboards[isWhite ? BLACK_PAWN : WHITE_PAWN];
    U64 eastAttacks = isWhite ? southEast(enemyPawns) : northEast(enemyPawns);
    eastAttacks &= ~FILES[A_FILE];
    U64 westAttacks = isWhite ? southWest(enemyPawns) : northWest(enemyPawns);
    westAttacks &= ~FILES[H_FILE];
    attackedSquares |= eastAttacks | westAttacks;

    U64 enemyKnights = position.bitboards[isWhite ? BLACK_KNIGHT : WHITE_KNIGHT];
    while (enemyKnights)
    {
        attackedSquares |= knightMoves[popFirstPiece(enemyKnights)];
    }

    attackedSquares |= kingMoves[getSquare(enemyKing)];

    safeSquares = ~attackedSquares;
}

template<bool isWhite>
void MoveGen::updateResolverSquares()
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

    U64 eastAttacks = getBoard(isWhite ? northEast(king) : southEast(king));
    eastAttacks &= ~FILES[A_FILE];
    U64 westAttacks = getBoard(isWhite ? northWest(king) : southWest(king));
    westAttacks &= ~FILES[H_FILE];
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
void MoveGen::updatePins()
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

    const U64 pins = getSlidingMoves<isCardinal>(king);

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

template<bool isWhite>
bool MoveGen::isInCheck()
{
    const U64 king = position.bitboards[isWhite ? WHITE_KING : BLACK_KING];
    const U64 enemyRooks = position.bitboards[isWhite ? BLACK_ROOK : WHITE_ROOK];
    const U64 enemyBishops = position.bitboards[isWhite ? BLACK_BISHOP : WHITE_BISHOP];
    const U64 enemyQueens = position.bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];

    U64 attackers = EMPTY_BOARD;
    const Square kingSquare = getSquare(king);
    attackers |= getSlidingMoves<true>(kingSquare) & (enemyRooks | enemyQueens);
    attackers |= getSlidingMoves<false>(kingSquare) & (enemyBishops | enemyQueens);
    attackers |= knightMoves[kingSquare] & position.bitboards[isWhite ? BLACK_KNIGHT : WHITE_KNIGHT];

    const U64 pawnAttackRank = isWhite ? north(king) : south(king);
    U64 pawnAttacks = (east(pawnAttackRank) & ~FILES[A_FILE]) | (west(pawnAttackRank) & ~FILES[A_FILE]);
    pawnAttacks &= position.bitboards[isWhite ? BLACK_PAWN : WHITE_PAWN];
    attackers |= pawnAttacks;
    return static_cast<bool>(attackers);
}

// generate fully legal moves
template<bool isWhite, bool quiets>
void MoveGen::genLegalMoves()
{
    numMoves = 0;

    std::memset(moveList, NULL_MOVE, sizeof(moveList));
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

void MoveGen::genMoves()
{
    if (position.isWhiteToMove)
    {
        genLegalMoves<true, true>();
    }
    else
    {
        genLegalMoves<false, true>();
    }
}

void MoveGen::genCaptures()
{
    if (position.isWhiteToMove)
    {
        genLegalMoves<true, false>();
    }
    else
    {
        genLegalMoves<false, false>();
    }
}

bool MoveGen::isInCheck(const int color)
{
    return color == 1 ? isInCheck<true>() : isInCheck<false>();
}