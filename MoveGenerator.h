//
// Created by Joe Chrisman on 4/6/23.
//

#ifndef KARL_MOVEGENERATOR_H
#define KARL_MOVEGENERATOR_H

#include "Position.h"

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

    // squares we can move a piece to without leaving the king in check
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
        //genKingMoves<isWhite, quiets>();
        //genRookMoves<isWhite, quiets>();
        //genBishopMoves<isWhite, quiets>();
        //genQueenMoves<isWhite, quiets>();
    }

    template<bool isWhite, bool quiets>
    void genPawnMoves()
    {
        constexpr U64 leftCaptureMask = ~FILE_MASKS[isWhite ? A_FILE : H_FILE];
        constexpr U64 rightCaptureMask = ~FILE_MASKS[isWhite ? H_FILE : F_FILE];
        constexpr U64 promotionMask = RANK_MASKS[isWhite ? EIGHTH_RANK : FIRST_RANK];
        constexpr U64 notPromotionMask = ~promotionMask;

        U64 pawns = position.bitboards[isWhite ? WHITE_PAWN : BLACK_PAWN];

        // calculate one square pawn pushes
        U64 pushed1 = (isWhite ? north(pawns) : south(pawns));

        // calculate left captures
        U64 leftCaptures = (isWhite ? west(pushed1 & leftCaptureMask)
                                   : east(pushed1 & leftCaptureMask));
        // calculate right captures
        U64 rightCaptures = (isWhite ? east(pushed1 & rightCaptureMask)
                                    : west(pushed1 & rightCaptureMask));

        // we can only capture if there is a piece to capture
        leftCaptures &= (isWhite ? position.blackPieces : position.whitePieces);
        rightCaptures &= (isWhite ? position.blackPieces : position.whitePieces);

        // calculate promotions by capturing
        uint64_t rightCapturePromotions = rightCaptures & promotionMask;
        uint64_t leftCapturePromotions = leftCaptures & promotionMask;

        // exclude promotions by capturing from normal captures
        rightCaptures &= notPromotionMask;
        leftCaptures &= notPromotionMask;

        while (leftCaptures)
        {
            Square to = popFirstPiece(leftCaptures);
            Square from = isWhite ? southEast(to) : northWest(to);
            moveList.emplace_back(createMove(
                    NORMAL,
                    isWhite ? WHITE_PAWN : BLACK_PAWN,
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
                    isWhite ? WHITE_PAWN : BLACK_PAWN,
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

        // we can only push onto empty squares
        pushed1 &= position.emptySquares;
        // calculate promotions by pawn push
        uint64_t pushPromotions = pushed1 & promotionMask;
        // exclude promotions from pawn pushes
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
                         isWhite ? WHITE_PAWN : BLACK_PAWN,
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
                        isWhite ? WHITE_PAWN : BLACK_PAWN,
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
        U64 knights = position.bitboards[isWhite ? WHITE_KNIGHT : BLACK_KNIGHT];
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
                        isWhite ? WHITE_KNIGHT : BLACK_KNIGHT,
                        position.pieces[to],
                        from,
                        to
                ));
            }
        }
    }


    template<bool isWhite, bool quiets>
    void genBishopMoves();

    template<bool isWhite, bool quiets>
    void genRookMoves();

    template<bool isWhite, bool quiets>
    void genQueenMoves();

    template<bool isWhite, bool quiets>
    void genKingMoves();

    template<bool isWhite>
    void updateSafeSquares();

    template<bool isWhite>
    void updateResolverSquares();

    template<bool isWhite, bool isCardinal>
    void updatePins();


};


#endif //KARL_MOVEGENERATOR_H
