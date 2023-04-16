//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_POSITION_H
#define KARL_POSITION_H

#include "Moves.h"

namespace Position
{
    bool init(const std::string& fen);
    void clear();

    extern U64 bitboards[13];
    extern Piece pieces[64];

    inline void updateBitboards();
    extern U64 emptySquares;
    extern U64 occupiedSquares;
    extern U64 whitePieces;
    extern U64 blackPieces;
    extern U64 whiteOrEmpty;
    extern U64 blackOrEmpty;

    inline constexpr int WHITE_CASTLE_SHORT = 0x1;
    inline constexpr int WHITE_CASTLE_LONG = 0x2;
    inline constexpr int BLACK_CASTLE_SHORT = 0x4;
    inline constexpr int BLACK_CASTLE_LONG = 0x8;

    extern struct Rights
    {
        bool isWhiteToMove;

        int castlingFlags;
        int enPassantFile;

        int currentPly;
        int lastIrreversiblePly;
    } rights;

    void makeMove(const Move move);
    void unMakeMove(const Move move, const Rights& previousRights);

    template<bool isWhite>
    void makeMove(const Move move);

    template<bool isWhite>
    void unMakeMove(const Move move, const Rights& previousRights);

    void print(bool isWhiteOnBottom);
};


#endif //KARL_POSITION_H
