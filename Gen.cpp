//
// Created by Joe Chrisman on 4/6/23.
//

#include "Gen.h"

std::vector<Move> Gen::moveList;

U64 Gen::resolverSquares = EMPTY_BOARD;
U64 Gen::safeSquares = EMPTY_BOARD;
U64 Gen::cardinalPins = EMPTY_BOARD;
U64 Gen::ordinalPins = EMPTY_BOARD;

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

    template<bool isWhite>
    void genPromotions(const Square from, const Square to, const Piece captured)
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_PAWN : BLACK_PAWN;

        for (int promoted = isWhite ? WHITE_KNIGHT : BLACK_KNIGHT;
             promoted <= (isWhite ? WHITE_QUEEN : BLACK_QUEEN);
             promoted++)
        {
            Gen::moveList.emplace_back(
                    Moves::createMove(from, to, pieceMoving, captured, promoted));
        }
    }

    template<bool isWhite, bool quiets>
    void genPawnMoves()
    {
        static constexpr Piece pieceMoving = isWhite ? WHITE_PAWN : BLACK_PAWN;
        static constexpr U64 promotionRank = RANK_MASKS[isWhite ? EIGHTH_RANK : FIRST_RANK];
        static constexpr U64 beforePromotionRank = RANK_MASKS[isWhite ? SEVENTH_RANK : SECOND_RANK];

        const U64 pawns = Position::bitboards[pieceMoving];
        const U64 unPinnedPawns = pawns & ~(Gen::cardinalPins | Gen::ordinalPins);
        // TODO: test if putting all promotion generation in a branch will be faster
        // generate unpinned push-promotions
        U64 unpinnedPushPromotions = isWhite ? north(unPinnedPawns) : south(unPinnedPawns);
        unpinnedPushPromotions &= promotionRank;
        unpinnedPushPromotions &= Position::emptySquares;
        unpinnedPushPromotions &= Gen::resolverSquares;
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
        unpinnedEastCaptures &= isWhite ? Position::blackPieces : Position::whitePieces;
        unpinnedWestCaptures &= isWhite ? Position::blackPieces : Position::whitePieces;

        // the captures must capture a checking piece
        unpinnedEastCaptures &= Gen::resolverSquares;
        unpinnedWestCaptures &= Gen::resolverSquares;

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
            const Square from = isWhite ? southWest(to) : northWest(to);
            Gen::moveList.emplace_back(
                    Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
        }
        while (unpinnedWestCaptures)
        {
            const Square to = popFirstPiece(unpinnedWestCaptures);
            const Square from = isWhite ? southEast(to) : northEast(to);
            Gen::moveList.emplace_back(
                    Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
        }

        // add unpinned capture-promotions
        while (unpinnedEastCapturePromotions)
        {
            const Square to = popFirstPiece(unpinnedEastCapturePromotions);
            const Square from = isWhite ? southWest(to) : northWest(to);
            genPromotions<isWhite>(from, to, Position::pieces[to]);
        }
        while (unpinnedWestCapturePromotions)
        {
            const Square to = popFirstPiece(unpinnedWestCapturePromotions);
            const Square from = isWhite ? southEast(to) : northEast(to);
            genPromotions<isWhite>(from, to, Position::pieces[to]);
        }

        // TODO: test if putting pinned pawn capture generation in a branch will be faster
        const U64 ordinalPinnedPawns = pawns & Gen::ordinalPins & ~Gen::cardinalPins;
        // calculate pinned east and west captures
        U64 pinnedEastCaptures = isWhite ? northEast(ordinalPinnedPawns) : southEast(ordinalPinnedPawns);
        pinnedEastCaptures &= ~FILE_MASKS[A_FILE];
        U64 pinnedWestCaptures = isWhite ? northWest(ordinalPinnedPawns) : southWest(ordinalPinnedPawns);
        pinnedWestCaptures &= ~FILE_MASKS[H_FILE];

        // we can only capture along the pin
        pinnedEastCaptures &= Gen::ordinalPins;
        pinnedWestCaptures &= Gen::ordinalPins;

        // make sure the pinned pawns are capturing the pinning piece
        pinnedEastCaptures &= isWhite ? Position::blackPieces : Position::whitePieces;
        pinnedWestCaptures &= isWhite ? Position::blackPieces : Position::whitePieces;

        // if there is a checking piece, no pinned capture can resolve it
        pinnedEastCaptures &= Gen::resolverSquares;
        pinnedWestCaptures &= Gen::resolverSquares;

        // isolate pinned capture-promotions
        U64 pinnedEastCapturePromotions = pinnedEastCaptures & promotionRank;
        U64 pinnedWestCapturePromotions = pinnedWestCaptures & promotionRank;

        // add pinned capture-promotions
        while (pinnedEastCapturePromotions)
        {
            const Square to = popFirstPiece(pinnedEastCapturePromotions);
            const Square from = isWhite ? southWest(to) : northWest(to);
            genPromotions<isWhite>(from, to, Position::pieces[to]);
        }
        while (pinnedWestCapturePromotions)
        {
            const Square to = popFirstPiece(pinnedWestCapturePromotions);
            const Square from = isWhite ? southEast(to) : northEast(to);
            genPromotions<isWhite>(from, to, Position::pieces[to]);
        }

        // normal pinned captures cannot be pinned capture-promotions
        pinnedEastCaptures &= ~promotionRank;
        pinnedWestCaptures &= ~promotionRank;

        // add pinned captures
        while (pinnedEastCaptures)
        {
            const Square to = popFirstPiece(pinnedEastCaptures);
            const Square from = isWhite ? southWest(to) : northWest(to);
            Gen::moveList.emplace_back(
                    Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
        }
        while (pinnedWestCaptures)
        {
            const Square to = popFirstPiece(pinnedWestCaptures);
            const Square from = isWhite ? southEast(to) : northEast(to);
            Gen::moveList.emplace_back(
                    Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
        }

        if (Position::rights.enPassantFile != -1)
        {
            static constexpr Piece captured = isWhite ? BLACK_PAWN : WHITE_PAWN;
            const U64 enPassantSquare = FILE_MASKS[Position::rights.enPassantFile] &
                                        RANK_MASKS[isWhite ? SIXTH_RANK : THIRD_RANK];

            U64 eastEnPassant = isWhite ? northEast(unPinnedPawns) : southEast(unPinnedPawns);
            eastEnPassant &= ~FILE_MASKS[A_FILE];
            eastEnPassant &= enPassantSquare;

            U64 westEnPassant = isWhite ? northWest(unPinnedPawns) : southWest(unPinnedPawns);
            westEnPassant &= ~FILE_MASKS[H_FILE];
            westEnPassant &= enPassantSquare;

            U64 eastPinnedEnPassant = isWhite ? northEast(ordinalPinnedPawns) : southEast(ordinalPinnedPawns);
            eastPinnedEnPassant &= ~FILE_MASKS[A_FILE];
            eastPinnedEnPassant &= enPassantSquare;
            eastPinnedEnPassant &= Gen::ordinalPins;
            eastEnPassant |= eastPinnedEnPassant;

            U64 westPinnedEnPassant = isWhite ? northWest(ordinalPinnedPawns) : southWest(ordinalPinnedPawns);
            westPinnedEnPassant &= ~FILE_MASKS[H_FILE];
            westPinnedEnPassant &= enPassantSquare;
            westPinnedEnPassant &= Gen::ordinalPins;
            westEnPassant |= westPinnedEnPassant;

            const U64 enPassantResolvers = isWhite ? north(Gen::resolverSquares) : south(Gen::resolverSquares);
            eastEnPassant &= enPassantResolvers;
            westEnPassant &= enPassantResolvers;

            if (eastEnPassant)
            {
                const Square to = getSquare(eastEnPassant);
                const Square from = isWhite ? southWest(to) : northWest(to);
                Gen::moveList.emplace_back(
                        Moves::EN_PASSANT | Moves::createMove(from, to, pieceMoving, captured));
            }
            if (westEnPassant)
            {
                const Square to = getSquare(westEnPassant);
                const Square from = isWhite ? southEast(to) : northEast(to);
                Gen::moveList.emplace_back(
                        Moves::EN_PASSANT | Moves::createMove(from, to, pieceMoving, captured));
            }
        }

        // generate non-captures and non-promotions
        if constexpr (quiets)
        {
            const U64 movablePawns = pawns & ~Gen::ordinalPins & ~beforePromotionRank;

            // calculate cardinal pinned pawns
            U64 pinnedPush1 = movablePawns & Gen::cardinalPins;
            // push the pinned pawns one square
            pinnedPush1 = isWhite ? north(pinnedPush1) : south(pinnedPush1);
            // we can only push onto empty squares
            pinnedPush1 &= Position::emptySquares;
            // don't break the pin
            pinnedPush1 &= Gen::cardinalPins;

            // calculate unpinned pawns
            U64 unpinnedPush1 = movablePawns & ~Gen::cardinalPins;
            // push the unpinned pawns one square
            unpinnedPush1 = isWhite ? north(unpinnedPush1) : south(unpinnedPush1);
            // we can only push onto empty squares
            unpinnedPush1 &= Position::emptySquares;

            // calculate all one square pawn pushes
            U64 pushed1 = unpinnedPush1 | pinnedPush1;
            // we must resolve a check
            pushed1 &= Gen::resolverSquares;

            // add one square pawn pushes
            while (pushed1)
            {
                const Square to = popFirstPiece(pushed1);
                const Square from = isWhite ? south(to) : north(to);
                Gen::moveList.emplace_back(
                        Moves::createMove(from, to, pieceMoving, NULL_PIECE));
            }

            static constexpr U64 pushRank = RANK_MASKS[isWhite ? THIRD_RANK : SIXTH_RANK];
            // calculate two square cardinal pinned pawn pushes
            U64 pinnedPush2 = pinnedPush1 & pushRank;
            // push the pawns
            pinnedPush2 = isWhite ? north(pinnedPush2) : south(pinnedPush2);
            // don't break the pin
            pinnedPush2 &= Gen::cardinalPins;

            // calculate two square pawn pushes
            U64 unpinnedPush2 = unpinnedPush1 & pushRank;
            // push the pawns
            unpinnedPush2 = isWhite ? north(unpinnedPush2) : south(unpinnedPush2);

            U64 pushed2 = unpinnedPush2 | pinnedPush2;
            // we can only push 2 squares if the second square is empty
            pushed2 &= Position::emptySquares;
            // we must resolve a check
            pushed2 &= Gen::resolverSquares;

            // add initial two square pawn pushes
            while (pushed2)
            {
                Square to = popFirstPiece(pushed2);
                Square from = isWhite ? south<2>(to) : north<2>(to);
                Gen::moveList.emplace_back(
                        Moves::DOUBLE_PAWN_PUSH | Moves::createMove(from, to, pieceMoving, NULL_PIECE));
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
                Gen::moveList.emplace_back(
                        Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
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
                Gen::moveList.emplace_back(
                        Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
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
                Gen::moveList.emplace_back(
                        Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
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
                Gen::moveList.push_back(
                        Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
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
            Gen::moveList.push_back(
                    Moves::createMove(from, to, pieceMoving, Position::pieces[to]));
        }

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
                Gen::moveList.push_back(
                        Moves::SHORT_CASTLE | Moves::createMove(from, to, pieceMoving, NULL_PIECE));
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
                Gen::moveList.push_back(
                        Moves::LONG_CASTLE | Moves::createMove(from, to, pieceMoving, NULL_PIECE));
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
        Gen::moveList.clear();

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
    // an estimate for the maximum number of moves in a position
    moveList.reserve(256);
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

bool Gen::isInCheck()
{
    const Piece king = Position::rights.isWhiteToMove ? WHITE_KING : BLACK_KING;
    return ~safeSquares & Position::bitboards[king];
}