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
    constexpr U64 promotionRank = RANK_MASKS[isWhite ? EIGHTH_RANK : FIRST_RANK];
    constexpr U64 beforePromotionRank = RANK_MASKS[isWhite ? SEVENTH_RANK : SECOND_RANK];

    const U64 pawns = position.bitboards[pieceMoving];
    const U64 unPinnedPawns = pawns & ~(cardinalPins | ordinalPins);
    // TODO: test if putting all promotion generation in a branch will be faster
    // generate unpinned push-promotions
    U64 unpinnedPushPromotions = isWhite ? north(unPinnedPawns) : south(unPinnedPawns);
    unpinnedPushPromotions &= promotionRank;
    unpinnedPushPromotions &= position.emptySquares;
    unpinnedPushPromotions &= resolverSquares;
    while (unpinnedPushPromotions)
    {
        const Square to = popFirstPiece(unpinnedPushPromotions);
        const Square from = isWhite ? south(to) : north(to);
        genPromotions<isWhite>(from, to, NULL_PIECE);
    }

    // calculate unpinned east and west captures
    U64 unpinnedEastCaptures = isWhite ? northEast(unPinnedPawns) : southEast(unPinnedPawns);
    unpinnedEastCaptures &= ~FILE_MASKS[A_FILE];
    U64 unpinnedWestCaptures = isWhite ? northWest(unPinnedPawns) : southWest(unPinnedPawns);
    unpinnedWestCaptures &= ~FILE_MASKS[H_FILE];

    // make sure the captures are actually capturing
    unpinnedEastCaptures &= isWhite ? position.blackPieces : position.whitePieces;
    unpinnedWestCaptures &= isWhite ? position.blackPieces : position.whitePieces;

    // the captures must capture a checking piece
    unpinnedEastCaptures &= resolverSquares;
    unpinnedWestCaptures &= resolverSquares;

    // calculate unpinned capture-promotions
    U64 unpinnedEastCapturePromotions = unpinnedEastCaptures & promotionRank;
    U64 unpinnedWestCapturePromotions = unpinnedWestCaptures & promotionRank;

    // the normal captures must not be capture-promotions
    unpinnedEastCaptures &= ~promotionRank;
    unpinnedWestCaptures &= ~promotionRank;

    // add unpinned captures
    while (unpinnedEastCaptures)
    {
        const Square to = popFirstPiece(unpinnedEastCaptures);
        moveList.emplace_back(createMove(
                NORMAL,
                pieceMoving,
                position.pieces[to],
                isWhite ? southWest(to) : northWest(to),
                to
        ));
    }
    while (unpinnedWestCaptures)
    {
        const Square to = popFirstPiece(unpinnedWestCaptures);
        moveList.emplace_back(createMove(
                NORMAL,
                pieceMoving,
                position.pieces[to],
                isWhite ? southEast(to) : northEast(to),
                to
        ));
    }

    // add unpinned capture-promotions
    while (unpinnedEastCapturePromotions)
    {
        const Square to = popFirstPiece(unpinnedEastCapturePromotions);
        const Square from = isWhite ? southWest(to) : northWest(to);
        genPromotions<isWhite>(from, to, position.pieces[to]);
    }
    while (unpinnedWestCapturePromotions)
    {
        const Square to = popFirstPiece(unpinnedWestCapturePromotions);
        const Square from = isWhite ? southEast(to) : northEast(to);
        genPromotions<isWhite>(from, to, position.pieces[to]);
    }

    // TODO: test if putting pinned pawn capture generation in a branch will be faster
    const U64 ordinalPinnedPawns = pawns & ordinalPins & ~cardinalPins;
    // calculate pinned east and west captures
    U64 pinnedEastCaptures = isWhite ? northEast(ordinalPinnedPawns) : southEast(ordinalPinnedPawns);
    pinnedEastCaptures &= ~FILE_MASKS[A_FILE];
    U64 pinnedWestCaptures = isWhite ? northWest(ordinalPinnedPawns) : southWest(ordinalPinnedPawns);
    pinnedWestCaptures &= ~FILE_MASKS[H_FILE];

    // we can only capture along the pin
    pinnedEastCaptures &= ordinalPins;
    pinnedWestCaptures &= ordinalPins;

    // make sure the pinned pawns are capturing the pinning piece
    pinnedEastCaptures &= isWhite ? position.blackPieces : position.whitePieces;
    pinnedWestCaptures &= isWhite ? position.blackPieces : position.whitePieces;

    // if there is a checking piece, no pinned capture can resolve it
    pinnedEastCaptures &= resolverSquares;
    pinnedWestCaptures &= resolverSquares;

    // isolate pinned capture-promotions
    U64 pinnedEastCapturePromotions = pinnedEastCaptures & promotionRank;
    U64 pinnedWestCapturePromotions = pinnedWestCaptures & promotionRank;

    // add pinned capture-promotions
    while (pinnedEastCapturePromotions)
    {
        const Square to = popFirstPiece(pinnedEastCapturePromotions);
        const Square from = isWhite ? southWest(to) : northWest(to);
        genPromotions<isWhite>(from, to, position.pieces[to]);
    }
    while (pinnedWestCapturePromotions)
    {
        const Square to = popFirstPiece(pinnedWestCapturePromotions);
        const Square from = isWhite ? southEast(to) : northEast(to);
        genPromotions<isWhite>(from, to, position.pieces[to]);
    }

    // normal pinned captures cannot be pinned capture-promotions
    pinnedEastCaptures &= ~promotionRank;
    pinnedWestCaptures &= ~promotionRank;

    // add pinned captures
    while (pinnedEastCaptures)
    {
        const Square to = popFirstPiece(pinnedEastCaptures);
        moveList.emplace_back(createMove(
                NORMAL,
                pieceMoving,
                position.pieces[to],
                isWhite ? southWest(to) : northWest(to),
                to
        ));
    }
    while (pinnedWestCaptures)
    {
        const Square to = popFirstPiece(pinnedWestCaptures);
        moveList.emplace_back(createMove(
                NORMAL,
                pieceMoving,
                position.pieces[to],
                isWhite ? southEast(to) : northEast(to),
                to
        ));
    }

    // generate non-captures and non-promotions
    if constexpr (quiets)
    {
        const U64 movablePawns = pawns & ~ordinalPins & ~beforePromotionRank;

        // calculate cardinal pinned pawns
        U64 pinnedPush1 = movablePawns & cardinalPins;
        // push the pinned pawns one square
        pinnedPush1 = isWhite ? north(pinnedPush1) : south(pinnedPush1);
        // we can only push onto empty squares
        pinnedPush1 &= position.emptySquares;
        // don't break the pin
        pinnedPush1 &= cardinalPins;

        // calculate unpinned pawns
        U64 unpinnedPush1 = movablePawns & ~cardinalPins;
        // push the unpinned pawns one square
        unpinnedPush1 = isWhite ? north(unpinnedPush1) : south(unpinnedPush1);
        // we can only push onto empty squares
        unpinnedPush1 &= position.emptySquares;

        // calculate all one square pawn pushes
        U64 pushed1 = unpinnedPush1 | pinnedPush1;
        // we must resolve a check
        pushed1 &= resolverSquares;

        // add one square pawn pushes
        while (pushed1)
        {
            const Square to = popFirstPiece(pushed1);
            moveList.push_back(createMove(
                    NORMAL,
                    pieceMoving,
                    NULL_PIECE,
                    isWhite ? south(to) : north(to),
                    to
            ));
        }

        constexpr U64 pushRank = RANK_MASKS[isWhite ? THIRD_RANK : SIXTH_RANK];
        // calculate two square cardinal pinned pawn pushes
        U64 pinnedPush2 = pinnedPush1 & pushRank;
        // push the pawns
        pinnedPush2 = isWhite ? north(pinnedPush2) : south(pinnedPush2);
        // don't break the pin
        pinnedPush2 &= cardinalPins;

        // calculate two square pawn pushes
        U64 unpinnedPush2 = unpinnedPush1 & pushRank;
        // push the pawns
        unpinnedPush2 = isWhite ? north(unpinnedPush2) : south(unpinnedPush2);

        U64 pushed2 = unpinnedPush2 | pinnedPush2;
        // we can only push 2 squares if the second square is empty
        pushed2 &= position.emptySquares;
        // we must resolve a check
        pushed2 &= resolverSquares;

        // add initial two square pawn pushes
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

template<bool isWhite, bool quiets>
void MoveGenerator::genBishopMoves()
{
    U64 bishops = position.bitboards[isWhite ? WHITE_BISHOP : BLACK_BISHOP];

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
                    isWhite ? WHITE_BISHOP : BLACK_BISHOP,
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
    U64 rooks = position.bitboards[isWhite ? WHITE_ROOK : BLACK_ROOK];

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
                    isWhite ? WHITE_ROOK : BLACK_ROOK,
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
    U64 queens = position.bitboards[isWhite ? WHITE_QUEEN : BLACK_QUEEN];
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
                    isWhite ? WHITE_QUEEN : BLACK_QUEEN,
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
    const Square from = getSquare(position.bitboards[isWhite ? WHITE_KING : BLACK_KING]);
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
        const MagicSliders::MagicSquare square = MagicSliders::cardinalMagics[from];
        U64 blockers = square.blockers & position.occupiedSquares;
        return MagicSliders::cardinalAttacks[from][blockers * square.magic >> 52];
    }
    else
    {
        const MagicSliders::MagicSquare square = MagicSliders::ordinalMagics[from];
        U64 blockers = square.blockers & position.occupiedSquares;
        return MagicSliders::ordinalAttacks[from][blockers * square.magic >> 55];
    }
}

template<bool isWhite>
void MoveGenerator::updateSafeSquares()
{
    U64 attackedSquares = EMPTY_BOARD;

    const U64 king = position.bitboards[isWhite ? WHITE_KING : BLACK_KING];
    const U64 enemyKing = position.bitboards[isWhite ? BLACK_KING : WHITE_KING];
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

    attackedSquares |= kingMoves[getSquare(enemyKing)];

    safeSquares = ~attackedSquares;
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

    U64 eastAttacks = getBoard(isWhite ? northEast(king) : southEast(king));
    eastAttacks &= ~FILE_MASKS[A_FILE];
    U64 westAttacks = getBoard(isWhite ? northWest(king) : southWest(king));
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