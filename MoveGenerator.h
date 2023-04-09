//
// Created by Joe Chrisman on 4/6/23.
//

#ifndef KARL_MOVEGENERATOR_H
#define KARL_MOVEGENERATOR_H

#include "Position.h"
#include "MagicSliders.h"

class MoveGenerator
{
public:
    explicit MoveGenerator(Position& position);

    template<bool quiets>
    void generateMoves()
    {
        moveList.clear();
        if (position.isWhiteToMove)
        {
            generateMoves<true, quiets>();
        }
        else
        {
            generateMoves<false, quiets>();
        }
    }

    std::vector<Move> moveList;
private:
    Position& position;

    void initKnightMoves();
    std::vector<U64> knightMoves = std::vector<U64>(64, EMPTY_BOARD);
    void initKingMoves();
    std::vector<U64> kingMoves = std::vector<U64>(64, EMPTY_BOARD);

    // squares that resolve a check
    U64 resolverSquares;
    // squares along pins
    U64 cardinalPins;
    U64 ordinalPins;

    template<bool isWhite, bool quiets>
    void generateMoves()
    {
        //updateSafeSquares<isWhite>();
        //updateResolverSquares<isWhite>();
        //updatePins<isWhite, true>();
        //updatePins<isWhite, false>();
        genPawnMoves<isWhite, quiets>();
        genKnightMoves<isWhite, quiets>();
        genKingMoves<isWhite, quiets>();
        genRookMoves<isWhite, quiets>();
        genBishopMoves<isWhite, quiets>();
        //genQueenMoves<isWhite, quiets>();
    }

    template<bool isWhite, bool quiets>
    void genPawnMoves()
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

    template<bool isWhite>
    void genPromotions(const Square from, const Square to, const Piece captured)
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


    template<bool isWhite, bool quiets>
    void genKnightMoves()
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
    void genBishopMoves()
    {
        constexpr Piece pieceMoving = isWhite ? WHITE_BISHOP : BLACK_BISHOP;
        U64 bishops = position.bitboards[pieceMoving];
        while (bishops)
        {
            Square from = popFirstPiece(bishops);
            U64 moves = getSlidingMoves<false>(from);
            moves &= isWhite ? position.blackOrEmpty : position.whiteOrEmpty;
            if constexpr (!quiets)
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
    void genRookMoves()
    {
        constexpr Piece pieceMoving = isWhite ? WHITE_ROOK : BLACK_ROOK;
        U64 rooks = position.bitboards[pieceMoving];
        while (rooks)
        {
            Square from = popFirstPiece(rooks);
            U64 moves = getSlidingMoves<true>(from);
            moves &= isWhite ? position.blackOrEmpty : position.whiteOrEmpty;
            if constexpr (!quiets)
            {
                rooks &= isWhite ? position.blackPieces : position.whitePieces;
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
    void genQueenMoves();

    template<bool isWhite, bool quiets>
    void genKingMoves()
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

    template<bool isCardinal>
    U64 getSlidingMoves(Square from)
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
    void updateSafeSquares();

    template<bool isWhite>
    void updateResolverSquares();

    template<bool isWhite, bool isCardinal>
    void updatePins();


};


#endif //KARL_MOVEGENERATOR_H
