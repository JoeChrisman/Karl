//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_POSITION_H
#define KARL_POSITION_H

#include "Eval.h"
#include "Moves.h"
#include "Zobrist.h"

namespace Position
{
    bool init(const std::string& fen);
    void clear();

    extern Hash hash;

    extern U64 bitboards[13];
    extern Piece pieces[64];

    inline void updateBitboards();
    extern U64 emptySquares;
    extern U64 occupiedSquares;
    extern U64 whitePieces;
    extern U64 blackPieces;
    extern U64 whiteOrEmpty;
    extern U64 blackOrEmpty;

    extern Score materialScore;
    extern Score midgamePlacementScore;
    extern Score endgamePlacementScore;

    extern bool isWhiteToMove;
    extern int totalPlies;

    extern struct Irreversibles
    {
        int castlingFlags;
        int enPassantFile;
        int reversiblePlies;
    } irreversibles;

    extern Hash history[MAX_MOVES];

    void makeMove(const Move move);
    void unMakeMove(const Move move, const Irreversibles& state);

    template<bool isWhite>
    void makeMove(const Move move);

    template<bool isWhite>
    void unMakeMove(const Move move, const Irreversibles& state);

    inline constexpr int WHITE_CASTLE_SHORT = 0x1;
    inline constexpr int WHITE_CASTLE_LONG = 0x2;
    inline constexpr int BLACK_CASTLE_SHORT = 0x4;
    inline constexpr int BLACK_CASTLE_LONG = 0x8;

    void print(bool isWhiteOnBottom);
};


#endif //KARL_POSITION_H
