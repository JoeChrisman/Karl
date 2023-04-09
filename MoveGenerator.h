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

    void generateMoves();
    void generateCaptures();

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
    void generate();

    template<bool isWhite, bool quiets>
    void genPawnMoves();

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
    void genPromotions(const Square from, const Square to, const Piece captured);

    template<bool isCardinal>
    U64 getSlidingMoves(Square from);

    template<bool isWhite>
    void updateSafeSquares();

    template<bool isWhite>
    void updateResolverSquares();

    template<bool isWhite, bool isCardinal>
    void updatePins();


};


#endif //KARL_MOVEGENERATOR_H
