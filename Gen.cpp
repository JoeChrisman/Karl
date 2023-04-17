//
// Created by Joe Chrisman on 4/6/23.
//

#include "Gen.h"

Move Gen::moveList[256];

U64 Gen::resolverSquares = EMPTY_BOARD;
U64 Gen::safeSquares = EMPTY_BOARD;
U64 Gen::cardinalPins = EMPTY_BOARD;
U64 Gen::ordinalPins = EMPTY_BOARD;

int Gen::numMoves = 0;

namespace
{
    U64 knightMoves[64];
    U64 kingMoves[64];

    void initKnightMoves()
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

    void initKingMoves()
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
    U64 getSlidingMoves(Square from)
    {
        if constexpr (isCardinal)
        {
            const Magics::MagicSquare square = Magics::cardinalMagics[from];
            U64 blockers = square.blockers & Position::occupiedSquares;
            return Magics::cardinalAttacks[from][blockers * square.magic >> 52];
        }
        else
        {
            const Magics::MagicSquare square = Magics::ordinalMagics[from];
            U64 blockers = square.blockers & Position::occupiedSquares;
            return Magics::ordinalAttacks[from][blockers * square.magic >> 55];
        }
    }

    template<bool isWhite>
    void genPromotions(const Square from, const Square to, const Piece captured)
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_PAWN : BLACK_PAWN;

        for (int promoted = isWhite ? WHITE_KNIGHT : BLACK_KNIGHT;
             promoted <= (isWhite ? WHITE_QUEEN : BLACK_QUEEN);
             promoted++)
        {
            Gen::moveList[Gen::numMoves++] =
                    Moves::createMove(from, to, pieceMoving, captured, promoted);
        }
    }

    template<bool isWhite>
    bool isEnPassantHorizontallyPinned(const Square from, const Square to)
    {
        static constexpr U64 enPassantRank = RANK_MASKS[isWhite ? FIFTH_RANK : FOURTH_RANK];
        const U64 captured = getBoard(isWhite ? south(to) : north(to));

        const U64 horizontalAttackerMask =
                Position::bitboards[isWhite ? WHITE_KING : BLACK_KING]
                | Position::bitboards[isWhite ? BLACK_ROOK : WHITE_ROOK]
                | Position::bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];

        Position::occupiedSquares ^= captured;
        const U64 horizontalAttacks = getSlidingMoves<true>(from)
                & horizontalAttackerMask
                & enPassantRank;
        Position::occupiedSquares |= captured;

        return getNumPieces(horizontalAttacks) == 2;
    }

    template<bool isWhite, bool isEast>
    U64 getPawnCaptures(const U64 pawns)
    {
        static constexpr U64 eastCaptureMask = ~FILE_MASKS[A_FILE];
        static constexpr U64 westCaptureMask = ~FILE_MASKS[H_FILE];
        if constexpr (isWhite)
        {
            if constexpr (isEast)
            {
                return northEast(pawns)
                    & Position::blackPieces
                    & eastCaptureMask
                    & Gen::resolverSquares;
            }
            else
            {
                return northWest(pawns)
                    & Position::blackPieces
                    & westCaptureMask
                    & Gen::resolverSquares;
            }
        }
        else
        {
            if constexpr (isEast)
            {
                return southEast(pawns)
                    & Position::whitePieces
                    & eastCaptureMask
                    & Gen::resolverSquares;
            }
            else
            {
                return southWest(pawns)
                    & Position::whitePieces
                    & westCaptureMask
                    & Gen::resolverSquares;
            }
        }
    }

    template<bool isWhite, bool quiets>
    void genPawnMoves()
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_PAWN : BLACK_PAWN;
        static constexpr U64 beforePromotionRank = RANK_MASKS[isWhite ? SEVENTH_RANK : SECOND_RANK];
        static constexpr U64 promotionRank = RANK_MASKS[isWhite ? EIGHTH_RANK : FIRST_RANK];

        const U64 pawns = Position::bitboards[pieceMoving];
        const U64 unpinnedPawns = pawns & ~(Gen::cardinalPins | Gen::ordinalPins);
        const U64 cardinalPinnedPawns = pawns & Gen::cardinalPins;
        const U64 ordinalPinnedPawns = pawns & Gen::ordinalPins;

        const U64 unpinnedEastCaptures = getPawnCaptures<isWhite, true>(unpinnedPawns);
        const U64 unpinnedWestCaptures = getPawnCaptures<isWhite, false>(unpinnedPawns);
        const U64 pinnedEastCaptures = getPawnCaptures<isWhite, true>(ordinalPinnedPawns) & Gen::ordinalPins;
        const U64 pinnedWestCaptures = getPawnCaptures<isWhite, false>(ordinalPinnedPawns) & Gen::ordinalPins;

        U64 eastCaptures = unpinnedEastCaptures | pinnedEastCaptures;
        U64 westCaptures = unpinnedWestCaptures | pinnedWestCaptures;

        const U64 unpinnedPawnPushes = (isWhite ? north(unpinnedPawns) : south(unpinnedPawns)) & Position::emptySquares;

        // calculate pawn promotions
        if (pawns & beforePromotionRank)
        {
            U64 eastCapturePromotions = eastCaptures & promotionRank;
            U64 westCapturePromotions = westCaptures & promotionRank;

            while (eastCapturePromotions)
            {
                const Square to = popFirstPiece(eastCapturePromotions);
                const Square from = isWhite ? southWest(to) : northWest(to);
                genPromotions<isWhite>(from, to, Position::pieces[to]);
            }
            while (westCapturePromotions)
            {
                const Square to = popFirstPiece(westCapturePromotions);
                const Square from = isWhite ? southEast(to) : northEast(to);
                genPromotions<isWhite>(from, to, Position::pieces[to]);
            }
            U64 pushPromotions = unpinnedPawnPushes & promotionRank & Gen::resolverSquares;
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
            Gen::moveList[Gen::numMoves++] =
                    Moves::createMove(from, to, pieceMoving, Position::pieces[to]);
        }
        while (westCaptures)
        {
            const Square to = popFirstPiece(westCaptures);
            const Square from = isWhite ? southEast(to) : northEast(to);
            Gen::moveList[Gen::numMoves++] =
                    Moves::createMove(from, to, pieceMoving, Position::pieces[to]);
        }

        const int enPassantFile = Position::rights.enPassantFile;
        if (enPassantFile > -1)
        {
            static constexpr U64 eastCaptureMask = ~FILE_MASKS[A_FILE];
            static constexpr U64 westCaptureMask = ~FILE_MASKS[H_FILE];
            static constexpr Piece pawnCapturing = isWhite ? BLACK_PAWN : WHITE_PAWN;

            const U64 enPassantSquare = FILE_MASKS[Position::rights.enPassantFile] &
                                        RANK_MASKS[isWhite ? SIXTH_RANK : THIRD_RANK];
            const U64 shiftedResolvers = isWhite ? north(Gen::resolverSquares) : south(Gen::resolverSquares);
            const U64 enPassantMask = enPassantSquare & shiftedResolvers;

            const U64 unpinnedWestCapture = isWhite ? northWest(unpinnedPawns) : southWest(unpinnedPawns);
            const U64 unpinnedEastCapture = isWhite ? northEast(unpinnedPawns) : southEast(unpinnedPawns);

            const U64 pinnedWestCapture = (isWhite ? northWest(ordinalPinnedPawns) : southWest(ordinalPinnedPawns))
                    & Gen::ordinalPins;
            const U64 pinnedEastCapture = (isWhite ? northEast(ordinalPinnedPawns) : southEast(ordinalPinnedPawns))
                    & Gen::ordinalPins;

            U64 westCapture = (unpinnedWestCapture | pinnedWestCapture) & enPassantMask & westCaptureMask;
            U64 eastCapture = (unpinnedEastCapture | pinnedEastCapture) & enPassantMask & eastCaptureMask;

            if (eastCapture)
            {
                const Square to = getSquare(eastCapture);
                const Square from = isWhite ? southWest(to) : northWest(to);
                if (!isEnPassantHorizontallyPinned<isWhite>(from, to))
                {
                    Gen::moveList[Gen::numMoves++] =
                            Moves::EN_PASSANT | Moves::createMove(from, to, pieceMoving, pawnCapturing);
                }
            }
            if (westCapture)
            {
                const Square to = getSquare(westCapture);
                const Square from = isWhite ? southEast(to) : northEast(to);
                if (!isEnPassantHorizontallyPinned<isWhite>(from, to))
                {
                    Gen::moveList[Gen::numMoves++] =
                            Moves::EN_PASSANT | Moves::createMove(from, to, pieceMoving, pawnCapturing);
                }
            }
        }
        if constexpr (quiets)
        {
            const U64 pinnedPawnPushes = (isWhite ? north(cardinalPinnedPawns) : south(cardinalPinnedPawns))
                 & Position::emptySquares
                 & Gen::cardinalPins;

            const U64 pushed1 = (pinnedPawnPushes | unpinnedPawnPushes) & ~promotionRank;
            U64 singlePawnPushes = pushed1 & Gen::resolverSquares;
            while (singlePawnPushes)
            {
                const Square to = popFirstPiece(singlePawnPushes);
                const Square from = isWhite ? south(to) : north(to);
                Gen::moveList[Gen::numMoves++] =
                        Moves::createMove(from, to, pieceMoving, NULL_PIECE);
            }

            U64 doublePawnPushes = (isWhite ? north(pushed1) : south(pushed1))
                & RANK_MASKS[isWhite ? FOURTH_RANK : FIFTH_RANK]
                & Position::emptySquares
                & Gen::resolverSquares;
            while (doublePawnPushes)
            {
                const Square to = popFirstPiece(doublePawnPushes);
                const Square from = isWhite ? south<2>(to) : north<2>(to);
                Gen::moveList[Gen::numMoves++] =
                        Moves::DOUBLE_PAWN_PUSH | Moves::createMove(from, to, pieceMoving, NULL_PIECE);
            }

        }
    }

    template<bool isWhite, bool quiets>
    void genKnightMoves()
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_KNIGHT : BLACK_KNIGHT;

        U64 knights = Position::bitboards[isWhite ? WHITE_KNIGHT : BLACK_KNIGHT];
        knights &= ~(Gen::ordinalPins | Gen::cardinalPins);

        while (knights)
        {
            Square from = popFirstPiece(knights);
            U64 moves = knightMoves[from];
            if constexpr (quiets)
            {
                moves &= (isWhite ? Position::blackOrEmpty : Position::whiteOrEmpty);
            }
            else
            {
                moves &= (isWhite ? Position::blackPieces : Position::whitePieces);
            }
            moves &= Gen::resolverSquares;
            while (moves)
            {
                Square to = popFirstPiece(moves);
                Gen::moveList[Gen::numMoves++] =
                        Moves::createMove(from, to, pieceMoving, Position::pieces[to]);
            }
        }
    }

    template<bool isWhite, bool quiets>
    void genBishopMoves()
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_BISHOP : BLACK_BISHOP;

        U64 bishops = Position::bitboards[pieceMoving] & ~Gen::cardinalPins;
        while (bishops)
        {
            const Square from = popFirstPiece(bishops);
            U64 moves = getSlidingMoves<false>(from);
            if constexpr (quiets)
            {
                moves &= isWhite ? Position::blackOrEmpty : Position::whiteOrEmpty;
            }
            else
            {
                moves &= isWhite ? Position::blackPieces : Position::whitePieces;
            }
            moves &= Gen::resolverSquares;
            if (getBoard(from) & Gen::ordinalPins)
            {
                moves &= Gen::ordinalPins;
            }
            while (moves)
            {
                const Square to = popFirstPiece(moves);
                Gen::moveList[Gen::numMoves++] =
                        Moves::createMove(from, to, pieceMoving, Position::pieces[to]);
            }
        }
    }

    template<bool isWhite, bool quiets>
    void genRookMoves()
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_ROOK : BLACK_ROOK;

        U64 rooks = Position::bitboards[pieceMoving] & ~Gen::ordinalPins;
        while (rooks)
        {
            const Square from = popFirstPiece(rooks);
            U64 moves = getSlidingMoves<true>(from);
            if constexpr (quiets)
            {
                moves &= isWhite ? Position::blackOrEmpty : Position::whiteOrEmpty;
            }
            else
            {
                moves &= isWhite ? Position::blackPieces : Position::whitePieces;
            }
            moves &= Gen::resolverSquares;
            if (getBoard(from) & Gen::cardinalPins)
            {
                moves &= Gen::cardinalPins;
            }
            while (moves)
            {
                const Square to = popFirstPiece(moves);
                Gen::moveList[Gen::numMoves++] =
                        Moves::createMove(from, to, pieceMoving, Position::pieces[to]);
            }
        }
    }

    template<bool isWhite, bool quiets>
    void genQueenMoves()
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_QUEEN : BLACK_QUEEN;
        U64 queens = Position::bitboards[pieceMoving];
        U64 moves = EMPTY_BOARD;
        while (queens)
        {
            const Square from = popFirstPiece(queens);
            U64 queen = getBoard(from);
            if (queen & ~Gen::cardinalPins)
            {
                moves |= getSlidingMoves<false>(from);
                if (queen & Gen::ordinalPins)
                {
                    moves &= Gen::ordinalPins;
                }
            }
            if (queen & ~Gen::ordinalPins)
            {
                moves |= getSlidingMoves<true>(from);
                if (queen & Gen::cardinalPins)
                {
                    moves &= Gen::cardinalPins;
                }
            }
            if constexpr (quiets)
            {
                moves &= isWhite ? Position::blackOrEmpty : Position::whiteOrEmpty;
            }
            else
            {
                moves &= isWhite ? Position::blackPieces : Position::whitePieces;
            }
            moves &= Gen::resolverSquares;
            while (moves)
            {
                const Square to = popFirstPiece(moves);
                Gen::moveList[Gen::numMoves++] =
                        Moves::createMove(from, to, pieceMoving, Position::pieces[to]);
            }
        }
    }

    template<bool isWhite, bool quiets>
    void genKingMoves()
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_KING : BLACK_KING;

        const Square from = getSquare(Position::bitboards[pieceMoving]);
        U64 moves = kingMoves[from] & Gen::safeSquares;
        if constexpr (quiets)
        {
            moves &= (isWhite ? Position::blackOrEmpty : Position::whiteOrEmpty);
        }
        else
        {
            moves &= (isWhite ? Position::blackPieces : Position::whitePieces);
        }
        while (moves)
        {
            const Square to = popFirstPiece(moves);
            Gen::moveList[Gen::numMoves++] =
                    Moves::createMove(from, to, pieceMoving, Position::pieces[to]);
        }

        if constexpr (quiets)
        {
            static constexpr int castleShort = isWhite ? Position::WHITE_CASTLE_SHORT : Position::BLACK_CASTLE_SHORT;
            static constexpr int castleLong = isWhite ? Position::WHITE_CASTLE_LONG : Position::BLACK_CASTLE_LONG;

            if (Position::rights.castlingFlags & castleShort)
            {
                static constexpr U64 shortSafeSquares = isWhite ? 0x7000000000000000 : 0x70;
                static constexpr U64 shortEmptySquares = isWhite ? 0x6000000000000000 : 0x60;
                if ((shortSafeSquares & Gen::safeSquares) == shortSafeSquares &&
                    (shortEmptySquares & Position::emptySquares) == shortEmptySquares)
                {
                    const int to = isWhite ? G1 : G8;
                    Gen::moveList[Gen::numMoves++] =
                            Moves::SHORT_CASTLE | Moves::createMove(from, to, pieceMoving, NULL_PIECE);
                }
            }
            if (Position::rights.castlingFlags & castleLong)
            {
                static constexpr U64 longSafeSquares = isWhite ? 0x1c00000000000000 : 0x1c;
                static constexpr U64 longEmptySquares = isWhite ? 0xe00000000000000 : 0xe;
                if ((longSafeSquares & Gen::safeSquares) == longSafeSquares &&
                    (longEmptySquares & Position::emptySquares) == longEmptySquares)
                {
                    const int to = isWhite ? C1 : C8;
                    Gen::moveList[Gen::numMoves++] =
                            Moves::LONG_CASTLE | Moves::createMove(from, to, pieceMoving, NULL_PIECE);
                }
            }
        }
    }

    template<bool isWhite>
    void updateSafeSquares()
    {
        U64 attackedSquares = EMPTY_BOARD;

        const U64 king = Position::bitboards[isWhite ? WHITE_KING : BLACK_KING];
        const U64 enemyKing = Position::bitboards[isWhite ? BLACK_KING : WHITE_KING];
        Position::occupiedSquares ^= king;

        U64 cardinalAttackers = Position::bitboards[isWhite ? BLACK_ROOK : WHITE_ROOK];
        U64 ordinalAttackers = Position::bitboards[isWhite ? BLACK_BISHOP : WHITE_BISHOP];
        cardinalAttackers |= Position::bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];
        ordinalAttackers |= Position::bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];
        while (cardinalAttackers)
        {
            attackedSquares |= getSlidingMoves<true>(popFirstPiece(cardinalAttackers));
        }
        while (ordinalAttackers)
        {
            attackedSquares |= getSlidingMoves<false>(popFirstPiece(ordinalAttackers));
        }

        Position::occupiedSquares ^= king;

        const U64 enemyPawns = Position::bitboards[isWhite ? BLACK_PAWN : WHITE_PAWN];
        U64 eastAttacks = isWhite ? southEast(enemyPawns) : northEast(enemyPawns);
        eastAttacks &= ~FILE_MASKS[A_FILE];
        U64 westAttacks = isWhite ? southWest(enemyPawns) : northWest(enemyPawns);
        westAttacks &= ~FILE_MASKS[H_FILE];
        attackedSquares |= eastAttacks | westAttacks;

        U64 enemyKnights = Position::bitboards[isWhite ? BLACK_KNIGHT : WHITE_KNIGHT];
        while (enemyKnights)
        {
            attackedSquares |= knightMoves[popFirstPiece(enemyKnights)];
        }

        attackedSquares |= kingMoves[getSquare(enemyKing)];

        Gen::safeSquares = ~attackedSquares;
    }

    template<bool isWhite>
    void updateResolverSquares()
    {
        const Square king = getSquare(Position::bitboards[isWhite ? WHITE_KING : BLACK_KING]);

        const U64 cardinalAttacks = getSlidingMoves<true>(king);
        const U64 ordinalAttacks = getSlidingMoves<false>(king);

        U64 cardinalAttackers = isWhite ? Position::bitboards[BLACK_ROOK] : Position::bitboards[WHITE_ROOK];
        U64 ordinalAttackers = isWhite ? Position::bitboards[BLACK_BISHOP] : Position::bitboards[WHITE_BISHOP];
        cardinalAttackers |= isWhite ? Position::bitboards[BLACK_QUEEN] : Position::bitboards[WHITE_QUEEN];
        ordinalAttackers |= isWhite ? Position::bitboards[BLACK_QUEEN] : Position::bitboards[WHITE_QUEEN];
        cardinalAttackers &= cardinalAttacks;
        ordinalAttackers &= ordinalAttacks;

        U64 attackers = cardinalAttackers | ordinalAttackers;
        attackers |= knightMoves[king] & Position::bitboards[isWhite ? BLACK_KNIGHT : WHITE_KNIGHT];

        U64 eastAttacks = getBoard(isWhite ? northEast(king) : southEast(king));
        eastAttacks &= ~FILE_MASKS[A_FILE];
        U64 westAttacks = getBoard(isWhite ? northWest(king) : southWest(king));
        westAttacks &= ~FILE_MASKS[H_FILE];
        attackers |= Position::bitboards[isWhite ? BLACK_PAWN : WHITE_PAWN] & (eastAttacks | westAttacks);

        if (attackers)
        {
            if (getNumPieces(attackers) == 1)
            {
                if (cardinalAttackers)
                {
                    Gen::resolverSquares = cardinalAttacks & getSlidingMoves<true>(getSquare(attackers));
                    Gen::resolverSquares |= attackers;
                }
                else if (ordinalAttackers)
                {
                    Gen::resolverSquares = ordinalAttacks & getSlidingMoves<false>(getSquare(attackers));
                    Gen::resolverSquares |= attackers;
                }
                else
                {
                    Gen::resolverSquares = attackers;
                }
            }
            else
            {
                Gen::resolverSquares = EMPTY_BOARD;
            }
        }
        else
        {
            Gen::resolverSquares = FULL_BOARD;
        }
    }

    template<bool isWhite, bool isCardinal>
    void updatePins()
    {
        if constexpr (isCardinal)
        {
            Gen::cardinalPins = EMPTY_BOARD;
        }
        else
        {
            Gen::ordinalPins = EMPTY_BOARD;
        }

        const Square king = getSquare(Position::bitboards[isWhite ? WHITE_KING : BLACK_KING]);

        U64 pinned = getSlidingMoves<isCardinal>(king);
        pinned &= isWhite ? Position::whitePieces : Position::blackPieces;

        Position::occupiedSquares ^= pinned;

        const U64 pins = getSlidingMoves<isCardinal>(king);

        U64 pinners = Position::bitboards[isWhite ? BLACK_QUEEN : WHITE_QUEEN];
        if constexpr (isCardinal)
        {
            pinners |=  Position::bitboards[isWhite ? BLACK_ROOK : WHITE_ROOK];
        }
        else
        {
            pinners |=  Position::bitboards[isWhite ? BLACK_BISHOP : WHITE_BISHOP];
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
                Gen::cardinalPins |= pin;
            }
            else
            {
                Gen::ordinalPins |= pin;
            }
        }
        Position::occupiedSquares ^= pinned;
    }

    // generate fully legal moves
    template<bool isWhite, bool quiets>
    void genLegalMoves()
    {
        Gen::numMoves = 0;

        std::memset(Gen::moveList, Moves::NULL_MOVE, sizeof(Gen::moveList));
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
}

void Gen::genMoves()
{
    if (Position::rights.isWhiteToMove)
    {
        genLegalMoves<true, true>();
    }
    else
    {
        genLegalMoves<false, true>();
    }
}

void Gen::init()
{
    std::memset(Gen::moveList, Moves::NULL_MOVE, sizeof(Gen::moveList));
    numMoves = 0;
    initKnightMoves();
    initKingMoves();
}


void Gen::genCaptures()
{
    if (Position::rights.isWhiteToMove)
    {
        genLegalMoves<true, false>();
    }
    else
    {
        genLegalMoves<false, false>();
    }
}