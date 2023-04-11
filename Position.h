//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_POSITION_H
#define KARL_POSITION_H

#include "Defs.h"

namespace Position
{
    void init(const std::string& fen);

    extern std::vector<U64> bitboards;
    extern std::vector<Piece> pieces;

    inline void updateBitboards();
    extern U64 emptySquares;
    extern U64 occupiedSquares;
    extern U64 whitePieces;
    extern U64 blackPieces;
    extern U64 whiteOrEmpty;
    extern U64 blackOrEmpty;

    extern struct Rights
    {
        U64 enPassant;
        bool whiteLongCastle;
        bool whiteShortCastle;
        bool blackLongCastle;
        bool blackShortCastle;
        int currentPly;
        int lastIrreversiblePly;
        bool isWhiteToMove;
    } rights;

    void makeMove(const Move move);
    void unMakeMove(const Move move);

    void print(bool isWhiteOnBottom);
};


#endif //KARL_POSITION_H
