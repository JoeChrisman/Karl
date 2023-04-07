//
// Created by Joe Chrisman on 4/6/23.
//

#ifndef KARL_MOVEGENERATOR_H
#define KARL_MOVEGENERATOR_H

#include "Position.h"

class MoveGenerator
{
public:

    MoveGenerator() = default;
    explicit MoveGenerator(Position& position);

    template<const bool quiets>
    void generateMoves();

    std::vector<Move> moveList;
private:
    Position position;

    void initKnightMoves();
    std::vector<U64> knightMoves = std::vector<U64>(64, EMPTY_BOARD);
    void initKingMoves();
    std::vector<U64> kingMoves = std::vector<U64>(64, EMPTY_BOARD);

    // squares we can move a piece to without leaving the king in check
    U64 resolverSquares;
    // squares along pins
    U64 cardinalPins;
    U64 ordinalPins;

    template<const bool isWhite, const bool quiets>
    void generateMoves();

    template<const bool isWhite, const bool quiets>
    void genPawnMoves();

    template<const bool isWhite, const bool quiets>
    void genKnightMoves();

    template<const bool isWhite, const bool quiets>
    void genBishopMoves();

    template<const bool isWhite, const bool quiets>
    void genRookMoves();

    template<const bool isWhite, const bool quiets>
    void genQueenMoves();

    template<const bool isWhite, const bool quiets>
    void genKingMoves();

    template<const bool isWhite>
    void genPromotions(const Square from, const Square to, const Piece captured);

    template<const bool isWhite>
    void updateSafeSquares();

    template<const bool isWhite>
    void updateResolverSquares();

    template<const bool isWhite, const bool isCardinal>
    void updatePins();


};


#endif //KARL_MOVEGENERATOR_H
