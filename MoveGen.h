//
// Created by Joe Chrisman on 4/6/23.
//

#ifndef KARL_MOVEGEN_H
#define KARL_MOVEGEN_H

#include "Position.h"
#include "Magics.h"
#include "Moves.h"

class MoveGen
{
public:
    MoveGen(Position& position, const Magics& magics);

    void genMoves();
    void genCaptures();

    bool isInCheck(const int color);

    int numMoves;
    Move moveList[256];

private:
    Position& position;
    const Magics& magics;

    template<bool isWhite, bool quiets>
    void genLegalMoves();

    U64 resolverSquares;
    U64 safeSquares;
    U64 cardinalPins;
    U64 ordinalPins;

    template<bool isWhite, bool isCardinal>
    void updatePins();

    template<bool isWhite>
    void updateSafeSquares();

    template<bool isWhite>
    void updateResolverSquares();

    template<bool isWhite>
    bool isEnPassantHorizontallyPinned(const Square from, const Square to);

    template<bool isWhite, bool isEast>
    U64 getPawnCaptures(const U64 pawns);

    template<bool isWhite>
    void genPromotions(const Square from, const Square to, const Piece captured);

    template<bool isWhite, bool quiets>
    void genPawnMoves();

    template<bool isCardinal>
    U64 getSlidingMoves(Square from);

    template<bool isWhite, bool quiets>
    void genKnightMoves();

    template<bool isWhite, bool quiets>
    void genBishopMoves();

    template<bool isWhite, bool quiets>
    void genRookMoves();

    template<bool isWhite, bool quiets>
    void genQueenMoves();

    template<bool isWhite, bool quiets>
    void genKingMoves();

    template<bool isWhite>
    bool isInCheck();


    void initKnightMoves();
    void initKingMoves();
    U64 knightMoves[64];
    U64 kingMoves[64];

};


#endif //KARL_MOVEGEN_H
