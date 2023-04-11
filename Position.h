//
// Created by Joe Chrisman on 4/5/23.
//

#ifndef KARL_POSITION_H
#define KARL_POSITION_H

#include "Definitions.h"

class Position
{

public:
    Position() = default;
    explicit Position(const std::string& fen);

    std::vector<U64> bitboards = std::vector<U64>(12, EMPTY_BOARD);
    std::vector<Piece> pieces = std::vector<Piece>(64, NULL_PIECE);

    U64 emptySquares;
    U64 occupiedSquares;
    U64 whitePieces;
    U64 blackPieces;
    U64 whiteOrEmpty;
    U64 blackOrEmpty;

    inline void updateBitboards();

    bool isWhiteToMove = true;

    void printPosition(bool isWhiteOnBottom) const;
    void makeMove(const Move move);
    void unMakeMove(const Move move);

    struct Rights
    {
        U64 enPassant;
        bool whiteLongCastle;
        bool whiteShortCastle;
        bool blackLongCastle;
        bool blackShortCastle;
        int currentPly;
        int lastIrreversiblePly;
    };

private:

    Piece getPieceByChar(const char piece) const;
    std::string getUnicodePiece(const Piece piece) const;

};


#endif //KARL_POSITION_H
